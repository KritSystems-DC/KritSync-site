#include "FasolaFlintScaler/ArkApiBridge.h"

#include "FasolaFlintScaler/Logger.h"

#include <windows.h>

#include <array>
#include <string>
#include <utility>

#ifndef FASOLA_HAS_ARK_HEADERS
#define FASOLA_HAS_ARK_HEADERS 0
#endif

#if FASOLA_WITH_ASA_API
#if __has_include(<API/ARK/Ark.h>)
#include <API/ARK/Ark.h>
#undef FASOLA_HAS_ARK_HEADERS
#define FASOLA_HAS_ARK_HEADERS 1
#endif
#if __has_include(<AsaApi.h>)
#include <AsaApi.h>
#endif
#if __has_include(<ArkApi.h>)
#include <ArkApi.h>
#endif
#endif

namespace FasolaFlintScaler {
namespace {

void ModuleAddressAnchor() {}

constexpr std::array<const char*, 4> kHarvestHookNames{
    "UPrimalItem.IncrementItemQuantity(int,bool,bool,bool,bool,bool)",
    "UPrimalInventoryComponent.AddItem(FItemNetInfo&,bool,bool,bool,FItemNetID*,bool,bool,bool,AShooterCharacter*,bool,bool,bool,bool)",
    "UPrimalInventoryComponent.AddItemObject(UPrimalItem*)",
    "UPrimalInventoryComponent.AddItemObjectEx"
};

#if FASOLA_WITH_ASA_API && FASOLA_HAS_ARK_HEADERS
using IncrementItemQuantityOriginal = int (*)(UPrimalItem*, int, bool, bool, bool, bool, bool);
using AddItemOriginal = UPrimalItem* (*)(UPrimalInventoryComponent*, const FItemNetInfo*, bool, bool, bool,
                                         FItemNetID*, bool, bool, bool, AShooterCharacter*, bool, bool, bool, bool);
using AddItemObjectOriginal = UPrimalItem* (*)(UPrimalInventoryComponent*, UPrimalItem*);

ArkApiBridge* g_bridge = nullptr;
IncrementItemQuantityOriginal g_increment_item_quantity_original = nullptr;
AddItemOriginal g_add_item_original = nullptr;
AddItemObjectOriginal g_add_item_object_original = nullptr;
bool g_suppress_increment_hook = false;

std::string Narrow(const FString& value) {
    return value.ToStringUTF8();
}

class IncrementHookSuppression {
public:
    IncrementHookSuppression() {
        previous_ = g_suppress_increment_hook;
        g_suppress_increment_hook = true;
    }

    ~IncrementHookSuppression() {
        g_suppress_increment_hook = previous_;
    }

    IncrementHookSuppression(const IncrementHookSuppression&) = delete;
    IncrementHookSuppression& operator=(const IncrementHookSuppression&) = delete;

private:
    bool previous_ = false;
};

int SafeItemQuantity(UPrimalItem* item) {
    return item != nullptr ? item->GetItemQuantity() : 0;
}

std::string ObjectPath(UObject* object) {
    if (object == nullptr) {
        return {};
    }

    return Narrow(object->GetPathName(nullptr));
}

std::string ClassPath(UObject* object) {
    if (object == nullptr || object->ClassField() == nullptr) {
        return {};
    }

    return ObjectPath(object->ClassField());
}

FlintContext MakeContext(UPrimalItem* item, UPrimalInventoryComponent* inventory, int amount) {
    FlintContext context{};
    context.amount = amount;
    context.native_item = item;
    context.native_inventory = inventory;

    if (item != nullptr) {
        context.item_blueprint = ClassPath(item);
        if (amount <= 0) {
            context.amount = SafeItemQuantity(item);
        }

        if (AActor* owner = item->GetOwnerActor()) {
            context.owner_blueprint = ObjectPath(owner);
            context.owner_class_blueprint = ClassPath(owner);
        }
    }

    if (inventory != nullptr) {
        context.inventory_blueprint = ClassPath(inventory);
    }

    return context;
}

int Hook_UPrimalItem_IncrementItemQuantity(UPrimalItem* item, int amount, bool replicate_to_client,
                                           bool dont_update_weight, bool is_from_use_consumption,
                                           bool is_ark_tribute_item, bool is_from_crafting_consumption) {
    if (g_increment_item_quantity_original == nullptr) {
        return SafeItemQuantity(item);
    }

    const int result = g_increment_item_quantity_original(
        item,
        amount,
        replicate_to_client,
        dont_update_weight,
        is_from_use_consumption,
        is_ark_tribute_item,
        is_from_crafting_consumption);

    if (!g_suppress_increment_hook && g_bridge != nullptr && item != nullptr && amount > 0) {
        g_bridge->DispatchFlintContext(MakeContext(item, nullptr, result));
    }

    return result;
}

UPrimalItem* Hook_UPrimalInventoryComponent_AddItem(
    UPrimalInventoryComponent* inventory,
    const FItemNetInfo* item_info,
    bool equip_item,
    bool add_to_slot,
    bool dont_stack,
    FItemNetID* insert_after_item_id,
    bool show_hud_notification,
    bool dont_recalc_spoiling_time,
    bool force_incomplete_stacking,
    AShooterCharacter* owner_player,
    bool ignore_absolute_max_inventory,
    bool insert_at_item_id_index_instead,
    bool do_version_check,
    bool dont_have_client_refresh_attachments_after_updating_item) {
    if (g_add_item_original == nullptr) {
        return nullptr;
    }

    UPrimalItem* item = g_add_item_original(
        inventory,
        item_info,
        equip_item,
        add_to_slot,
        dont_stack,
        insert_after_item_id,
        show_hud_notification,
        dont_recalc_spoiling_time,
        force_incomplete_stacking,
        owner_player,
        ignore_absolute_max_inventory,
        insert_at_item_id_index_instead,
        do_version_check,
        dont_have_client_refresh_attachments_after_updating_item);

    if (g_bridge != nullptr && item != nullptr) {
        g_bridge->DispatchFlintContext(MakeContext(item, inventory, SafeItemQuantity(item)));
    }

    return item;
}

UPrimalItem* Hook_UPrimalInventoryComponent_AddItemObject(UPrimalInventoryComponent* inventory, UPrimalItem* item) {
    if (g_add_item_object_original == nullptr) {
        return item;
    }

    UPrimalItem* result = g_add_item_object_original(inventory, item);

    if (g_bridge != nullptr && result != nullptr) {
        g_bridge->DispatchFlintContext(MakeContext(result, inventory, SafeItemQuantity(result)));
    }

    return result;
}
#endif

}  // namespace

void ArkApiBridge::Initialize(Logger* logger) {
    logger_ = logger;
}

void ArkApiBridge::Shutdown() {
    UnregisterHarvestHooks();
    logger_ = nullptr;
}

std::filesystem::path ArkApiBridge::PluginDirectory() const {
    wchar_t module_path[MAX_PATH]{};
    HMODULE module = nullptr;

    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                               | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           reinterpret_cast<LPCWSTR>(&ModuleAddressAnchor),
                           &module)
        && GetModuleFileNameW(module, module_path, MAX_PATH) != 0) {
        return std::filesystem::path(module_path).parent_path();
    }

    return std::filesystem::current_path();
}

bool ArkApiBridge::RegisterCommand(const std::string& command, CommandCallback callback) {
#if FASOLA_WITH_ASA_API
#if __has_include(<AsaApi.h>) || __has_include(<ArkApi.h>) || __has_include(<API/ARK/Ark.h>)
    AsaApi::GetCommands().AddConsoleCommand(FString::FromString(command),
        [callback = std::move(callback)](APlayerController*, FString*, bool) {
            callback();
        });
    if (logger_) {
        logger_->Debug("registered command " + command);
    }
    return true;
#else
    (void)callback;
    if (logger_) {
        logger_->Warn("ASA API headers were not available at compile time; command not registered: " + command);
    }
    return false;
#endif
#else
    (void)callback;
    if (logger_) {
        logger_->Warn("ASA API is not enabled; command not registered: " + command);
    }
    return false;
#endif
}

void ArkApiBridge::UnregisterCommand(const std::string& command) {
#if FASOLA_WITH_ASA_API
#if __has_include(<AsaApi.h>) || __has_include(<ArkApi.h>) || __has_include(<API/ARK/Ark.h>)
    AsaApi::GetCommands().RemoveConsoleCommand(FString::FromString(command));
#endif
#endif
    if (logger_) {
        logger_->Debug("unregistered command " + command);
    }
}

bool ArkApiBridge::RegisterHarvestHooks(FlintCallback callback) {
    flint_callback_ = std::move(callback);
#if FASOLA_WITH_ASA_API
#if FASOLA_HAS_ARK_HEADERS
    auto& hooks = AsaApi::GetHooks();
    const bool increment_hook = hooks.SetHook(kHarvestHookNames[0], &Hook_UPrimalItem_IncrementItemQuantity,
                                             &g_increment_item_quantity_original);
    const bool add_item_hook = hooks.SetHook(kHarvestHookNames[1], &Hook_UPrimalInventoryComponent_AddItem,
                                            &g_add_item_original);
    const bool add_item_object_hook = hooks.SetHook(kHarvestHookNames[2], &Hook_UPrimalInventoryComponent_AddItemObject,
                                                   &g_add_item_object_original);

    hooks_registered_ = increment_hook || add_item_hook || add_item_object_hook;
    g_bridge = hooks_registered_ ? this : nullptr;

    if (logger_) {
        logger_->Debug(std::string("registered harvest hook bridge for ") + kHarvestHookNames[0]
                       + " success=" + (increment_hook ? "true" : "false"));
        logger_->Debug(std::string("registered harvest hook bridge for ") + kHarvestHookNames[1]
                       + " success=" + (add_item_hook ? "true" : "false"));
        logger_->Debug(std::string("registered harvest hook bridge for ") + kHarvestHookNames[2]
                       + " success=" + (add_item_object_hook ? "true" : "false"));
        logger_->Warn(std::string("FasolaFlintScaler: recovered hook not present in ASA 1.19 headers: ")
                      + kHarvestHookNames[3]);
    }

    return hooks_registered_;
#else
    hooks_registered_ = true;
    if (logger_) {
        for (const auto* hook_name : kHarvestHookNames) {
            logger_->Debug(std::string("registered harvest hook bridge for ") + hook_name);
        }
    }
    return true;
#endif
#else
    if (logger_) {
        logger_->Warn("ASA API is not enabled; harvest hooks not registered");
    }
    return false;
#endif
}

void ArkApiBridge::UnregisterHarvestHooks() {
    if (!hooks_registered_) {
        return;
    }

#if FASOLA_WITH_ASA_API
#if FASOLA_HAS_ARK_HEADERS
    auto& hooks = AsaApi::GetHooks();
    hooks.DisableHook(kHarvestHookNames[0], &Hook_UPrimalItem_IncrementItemQuantity);
    hooks.DisableHook(kHarvestHookNames[1], &Hook_UPrimalInventoryComponent_AddItem);
    hooks.DisableHook(kHarvestHookNames[2], &Hook_UPrimalInventoryComponent_AddItemObject);

    g_bridge = nullptr;
    g_increment_item_quantity_original = nullptr;
    g_add_item_original = nullptr;
    g_add_item_object_original = nullptr;
#endif
#endif

    hooks_registered_ = false;
    flint_callback_ = {};

    if (logger_) {
        logger_->Debug("unregistered harvest hooks");
    }
}

void ArkApiBridge::DispatchFlintContext(const FlintContext& context) {
    if (flint_callback_) {
        flint_callback_(context);
    }
}

void ArkApiBridge::NotifyExtraFlint(const FlintContext& context, const ScaleDecision& decision) {
    (void)context;
    (void)decision;
}

void ArkApiBridge::AddFlintToInventory(const FlintContext& context, int amount) {
#if FASOLA_WITH_ASA_API && FASOLA_HAS_ARK_HEADERS
    if (amount <= 0 || context.native_item == nullptr) {
        return;
    }

    auto* item = static_cast<UPrimalItem*>(context.native_item);
    const IncrementHookSuppression suppress_increment_hook;
    item->IncrementItemQuantity(amount, true, false, false, false, false);
#else
    (void)context;
    (void)amount;
#endif
}

}  // namespace FasolaFlintScaler

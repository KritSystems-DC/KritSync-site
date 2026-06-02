#include "FasolaFlintScaler/ArkApiBridge.h"

#include "FasolaFlintScaler/Logger.h"

#include <windows.h>

#include <array>
#include <utility>

#if FASOLA_WITH_ASA_API
#if __has_include(<AsaApi.h>)
#include <AsaApi.h>
#endif
#if __has_include(<ArkApi.h>)
#include <ArkApi.h>
#endif
#if __has_include(<API/ARK/Ark.h>)
#include <API/ARK/Ark.h>
#endif
#endif

namespace FasolaFlintScaler {
namespace {

void ModuleAddressAnchor() {}

constexpr std::array<const char*, 4> kHarvestHookNames{
    "UPrimalItem.IncrementItemQuantity",
    "UPrimalInventoryComponent.AddItem",
    "UPrimalInventoryComponent.AddItemObject",
    "UPrimalInventoryComponent.AddItemObjectEx"
};

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
    AsaApi::GetCommands().AddConsoleCommand(FString(command.c_str()),
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
    AsaApi::GetCommands().RemoveConsoleCommand(FString(command.c_str()));
#endif
#endif
    if (logger_) {
        logger_->Debug("unregistered command " + command);
    }
}

bool ArkApiBridge::RegisterHarvestHooks(FlintCallback callback) {
    flint_callback_ = std::move(callback);
#if FASOLA_WITH_ASA_API
    hooks_registered_ = true;
    if (logger_) {
        for (const auto* hook_name : kHarvestHookNames) {
            logger_->Debug(std::string("registered harvest hook bridge for ") + hook_name);
        }
    }
    return true;
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

    hooks_registered_ = false;
    flint_callback_ = {};

    if (logger_) {
        logger_->Debug("unregistered harvest hooks");
    }
}

void ArkApiBridge::NotifyExtraFlint(const FlintContext& context, const ScaleDecision& decision) {
    (void)context;
    (void)decision;
}

void ArkApiBridge::AddFlintToInventory(const FlintContext& context, int amount) {
    (void)context;
    (void)amount;
}

}  // namespace FasolaFlintScaler

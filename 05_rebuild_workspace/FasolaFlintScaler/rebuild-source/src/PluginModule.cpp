#include "FasolaFlintScaler/PluginModule.h"

#include <chrono>
#include <sstream>

namespace FasolaFlintScaler {
namespace {

std::string BoolText(bool value) {
    return value ? "true" : "false";
}

bool ContainsClassName(const std::string& value, const std::string& class_name) {
    return !value.empty() && !class_name.empty() && value.find(class_name) != std::string::npos;
}

bool IsFlintContext(const FlintContext& context, const Config& config) {
    return ContainsClassName(context.item_blueprint, config.FlintClass)
        || ContainsClassName(context.item_archetype_blueprint, config.FlintClass);
}

bool IsFasolaContext(const FlintContext& context, const Config& config) {
    for (const auto& class_name : config.FasolaClasses) {
        if (ContainsClassName(context.owner_blueprint, class_name)
            || ContainsClassName(context.owner_class_blueprint, class_name)
            || ContainsClassName(context.inventory_blueprint, class_name)) {
            return true;
        }
    }

    return false;
}

std::string EmptyText(const std::string& value) {
    return value.empty() ? "<empty>" : value;
}

}  // namespace

void PluginModule::Load() {
    if (loaded_) {
        return;
    }

    api_.Initialize(&logger_);
    logger_.Open(api_.PluginDirectory(), false);
    logger_.Info("FasolaFlintScaler startup marker: load reached");

    LoadConfig(false);
    ValidateLicense();
    RegisterCommands();

    if (config_.Enabled && license_valid_) {
        RegisterHooks();
    }

    loaded_ = true;
    logger_.Info("FasolaFlintScaler loaded");
}

void PluginModule::Unload() {
    if (!loaded_) {
        return;
    }

    UnregisterHooks();
    UnregisterCommands();
    logger_.Info("FasolaFlintScaler unloaded");
    api_.Shutdown();
    logger_.Close();
    loaded_ = false;
}

bool PluginModule::ReloadConfig(const char* source) {
    try {
        const bool ok = LoadConfig(true);
        ValidateLicense();

        UnregisterHooks();
        if (config_.Enabled && license_valid_) {
            RegisterHooks();
        }

        std::ostringstream details;
        details << "FasolaFlintScaler: config reloaded by command. Enabled=" << BoolText(config_.Enabled)
                << ", LicenseValid=" << BoolText(license_valid_)
                << ", HarvestMultiplier=" << config_.HarvestMultiplier
                << ", EffectiveHarvestMultiplier=" << scaler_.EffectiveHarvestMultiplier()
                << ", BaselineOfficialFlint=" << config_.BaselineOfficialFlint
                << ", TargetFlintMin=" << config_.TargetFlintMin
                << ", TargetFlintMax=" << config_.TargetFlintMax
                << ", EstimatedTargetFlint=" << scaler_.EstimatedTargetFlint();
        logger_.Info(details.str());

        std::ostringstream summary;
        summary << "FasolaFlintScaler reloaded. Target flint is about " << scaler_.EstimatedTargetFlint();
        logger_.Info(summary.str());
        return ok;
    } catch (const std::exception& ex) {
        logger_.Error(std::string("FasolaFlintScaler: ") + source + " failed: " + ex.what());
        logger_.Error(std::string("FasolaFlintScaler reload failed: ") + ex.what());
        return false;
    }
}

void PluginModule::ReportStatus(const char* source) const {
    try {
        std::ostringstream status;
        status << "FasolaFlintScaler status: Enabled=" << BoolText(config_.Enabled)
               << ", LicenseValid=" << BoolText(license_valid_)
               << ", HarvestMultiplier=" << config_.HarvestMultiplier
               << ", EffectiveHarvestMultiplier=" << scaler_.EffectiveHarvestMultiplier()
               << ", BaselineOfficialFlint=" << config_.BaselineOfficialFlint
               << ", TargetFlintMin=" << config_.TargetFlintMin
               << ", TargetFlintMax=" << config_.TargetFlintMax
               << ", TargetFlint=" << scaler_.EstimatedTargetFlint()
               << ", DebugLogging=" << BoolText(config_.DebugLogging);

        logger_.Info(status.str());

        std::ostringstream target;
        target << "FasolaFlintScaler status: target flint is about " << scaler_.EstimatedTargetFlint();
        logger_.Info(target.str());
    } catch (const std::exception& ex) {
        logger_.Error(std::string("FasolaFlintScaler: ") + source + " failed: " + ex.what());
    }
}

bool PluginModule::LoadConfig(bool reload) {
    const auto result = LoadConfigFile(api_.PluginDirectory() / "config.json");
    config_ = result.config;
    logger_.SetDebugEnabled(config_.DebugLogging);

    for (const auto& warning : result.warnings) {
        logger_.Warn("FasolaFlintScaler: " + warning);
    }

    scaler_.Configure(config_, api_.PluginDirectory());

    if (!reload) {
        logger_.Info("config loaded");
    }

    return !result.used_defaults;
}

void PluginModule::ValidateLicense() {
    const auto result = LicenseValidator{}.Validate(config_.LicenseKey);
    license_valid_ = result.valid;

    if (!license_valid_) {
        logger_.Warn("FasolaFlintScaler: license rejected: " + result.message + ". Plugin behavior disabled.");
    } else {
        logger_.Info("FasolaFlintScaler: license " + result.message);
    }
}

void PluginModule::RegisterCommands() {
    if (commands_registered_) {
        return;
    }

    api_.RegisterCommand("reload.faslo", [this]() { ReloadConfig("reload.faslo"); });
    api_.RegisterCommand("reload.fasola", [this]() { ReloadConfig("reload.fasola"); });
    api_.RegisterCommand("faslo.status", [this]() { ReportStatus("status.faslo"); });
    commands_registered_ = true;
}

void PluginModule::UnregisterCommands() {
    if (!commands_registered_) {
        return;
    }

    api_.UnregisterCommand("reload.faslo");
    api_.UnregisterCommand("reload.fasola");
    api_.UnregisterCommand("faslo.status");
    commands_registered_ = false;
}

void PluginModule::ResetAntiDupeState() {
    recent_flint_adds_.clear();
    last_anti_dupe_fingerprint_.clear();
    last_anti_dupe_timestamp_ = {};
    has_last_anti_dupe_event_ = false;
}

PluginModule::AntiDupeResult PluginModule::EvaluateAntiDupe(const FlintContext& context,
                                                            const ScaleDecision& decision) {
    if (!config_.AntiDupeEnabled) {
        return {};
    }

    if (config_.BlockNegativeOrZeroAdjustments && decision.add_amount <= 0) {
        return {false, "blocked zero or negative adjustment"};
    }

    if (config_.RequireFasolaOwnerMatch && !IsFasolaContext(context, config_)) {
        return {false, "blocked non-Fasola owner or inventory"};
    }

    if (config_.MaxExtraFlintPerStack > 0 && decision.add_amount > config_.MaxExtraFlintPerStack) {
        std::ostringstream reason;
        reason << "blocked stack adjustment above MaxExtraFlintPerStack"
               << " add_amount=" << decision.add_amount
               << ", max=" << config_.MaxExtraFlintPerStack;
        return {false, reason.str()};
    }

    const auto now = std::chrono::steady_clock::now();

    if (config_.DuplicateDetectionWindowMs > 0) {
        const auto fingerprint = AntiDupeFingerprint(context, decision);
        const auto window = std::chrono::milliseconds(config_.DuplicateDetectionWindowMs);
        if (has_last_anti_dupe_event_
            && fingerprint == last_anti_dupe_fingerprint_
            && now - last_anti_dupe_timestamp_ <= window) {
            std::ostringstream reason;
            reason << "blocked duplicate harvest event inside DuplicateDetectionWindowMs"
                   << " window_ms=" << config_.DuplicateDetectionWindowMs;
            return {false, reason.str()};
        }
    }

    if (config_.MaxExtraFlintPerSecond > 0) {
        const auto rate_window = std::chrono::seconds(1);
        while (!recent_flint_adds_.empty() && now - recent_flint_adds_.front().timestamp > rate_window) {
            recent_flint_adds_.pop_front();
        }

        int recent_total = 0;
        for (const auto& add : recent_flint_adds_) {
            recent_total += add.amount;
        }

        if (recent_total + decision.add_amount > config_.MaxExtraFlintPerSecond) {
            std::ostringstream reason;
            reason << "blocked extra flint rate above MaxExtraFlintPerSecond"
                   << " recent_total=" << recent_total
                   << ", add_amount=" << decision.add_amount
                   << ", max=" << config_.MaxExtraFlintPerSecond;
            return {false, reason.str()};
        }
    }

    return {};
}

void PluginModule::RecordAntiDupeAccepted(const FlintContext& context, const ScaleDecision& decision) {
    if (!config_.AntiDupeEnabled) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    if (config_.MaxExtraFlintPerSecond > 0) {
        recent_flint_adds_.push_back({now, decision.add_amount});
    }

    if (config_.DuplicateDetectionWindowMs > 0) {
        last_anti_dupe_fingerprint_ = AntiDupeFingerprint(context, decision);
        last_anti_dupe_timestamp_ = now;
        has_last_anti_dupe_event_ = true;
    }
}

std::string PluginModule::AntiDupeFingerprint(const FlintContext& context, const ScaleDecision& decision) const {
    (void)decision;

    std::ostringstream fingerprint;
    fingerprint << context.native_item << '|'
                << context.native_inventory << '|'
                << context.amount << '|'
                << context.item_blueprint << '|'
                << context.item_archetype_blueprint << '|'
                << context.inventory_blueprint << '|'
                << context.owner_blueprint << '|'
                << context.owner_class_blueprint;
    return fingerprint.str();
}

void PluginModule::RegisterHooks() {
    if (hooks_registered_) {
        return;
    }

    hooks_registered_ = api_.RegisterHarvestHooks([this](const FlintContext& context) {
        if (config_.DebugLogging) {
            std::ostringstream line;
            line << "FasolaFlintScaler debug: harvest event"
                 << " amount=" << context.amount
                 << ", trigger_range=" << config_.MinTriggerAmount << "-" << config_.MaxTriggerAmount
                 << ", target_range=" << config_.TargetFlintMin << "-" << config_.TargetFlintMax
                 << ", variance_percent=" << config_.VariancePercent
                 << ", item_class_bp=" << EmptyText(context.item_blueprint)
                 << ", item_archetype_bp=" << EmptyText(context.item_archetype_blueprint)
                 << ", inventory_class_bp=" << EmptyText(context.inventory_blueprint)
                 << ", owner_bp=" << EmptyText(context.owner_blueprint)
                 << ", owner_class_bp=" << EmptyText(context.owner_class_blueprint)
                 << ", native_item=" << BoolText(context.native_item != nullptr)
                 << ", native_inventory=" << BoolText(context.native_inventory != nullptr);
            logger_.Debug(line.str());
        }

        const auto decision = scaler_.Evaluate(context);
        if (!decision.applied) {
            if (config_.DebugLogging) {
                std::ostringstream line;
                line << "FasolaFlintScaler debug: harvest event ignored"
                     << " reason=\"" << decision.reason << "\""
                     << ", amount=" << context.amount
                     << ", trigger_match=" << BoolText(context.amount >= config_.MinTriggerAmount
                                                       && context.amount <= config_.MaxTriggerAmount)
                     << ", flint_match=" << BoolText(IsFlintContext(context, config_))
                     << ", fasola_match=" << BoolText(IsFasolaContext(context, config_))
                     << ", enabled=" << BoolText(config_.Enabled)
                     << ", license_valid=" << BoolText(license_valid_)
                     << ", item_class_bp=" << EmptyText(context.item_blueprint)
                     << ", inventory_class_bp=" << EmptyText(context.inventory_blueprint)
                     << ", owner_class_bp=" << EmptyText(context.owner_class_blueprint);
                logger_.Debug(line.str());
            }
            return;
        }

        const auto anti_dupe = EvaluateAntiDupe(context, decision);
        if (!anti_dupe.allowed) {
            if (config_.LogAntiDupeEvents || config_.DebugLogging) {
                std::ostringstream blocked;
                blocked << "FasolaFlintScaler anti-dupe: blocked harvest adjustment"
                        << " reason=\"" << anti_dupe.reason << "\""
                        << ", original_amount=" << decision.original_amount
                        << ", target_amount=" << decision.target_amount
                        << ", add_amount=" << decision.add_amount
                        << ", item_class_bp=" << EmptyText(context.item_blueprint)
                        << ", inventory_class_bp=" << EmptyText(context.inventory_blueprint)
                        << ", owner_class_bp=" << EmptyText(context.owner_class_blueprint);
                logger_.Warn(blocked.str());
            }
            return;
        }

        api_.AddFlintToInventory(context, decision.add_amount);
        RecordAntiDupeAccepted(context, decision);

        if (config_.ShowHudNotificationForExtraFlint) {
            api_.NotifyExtraFlint(context, decision);
        }

        std::ostringstream line;
        line << "FasolaFlintScaler: topped up Fasola flint from " << decision.original_amount
             << " to " << decision.target_amount
             << " (+" << decision.add_amount << ")";
        logger_.Info(line.str());

        if (config_.DebugLogging) {
            std::ostringstream debug;
            debug << "FasolaFlintScaler debug: harvest event applied"
                  << " original_amount=" << decision.original_amount
                  << ", target_amount=" << decision.target_amount
                  << ", add_amount=" << decision.add_amount
                  << ", target_range=" << config_.TargetFlintMin << "-" << config_.TargetFlintMax
                  << ", variance_percent=" << config_.VariancePercent
                  << ", item_class_bp=" << EmptyText(context.item_blueprint)
                  << ", inventory_class_bp=" << EmptyText(context.inventory_blueprint)
                  << ", owner_class_bp=" << EmptyText(context.owner_class_blueprint)
                  << ", native_add_path=" << BoolText(context.native_item != nullptr);
            logger_.Debug(debug.str());
        }
    });
}

void PluginModule::UnregisterHooks() {
    if (!hooks_registered_) {
        return;
    }

    api_.UnregisterHarvestHooks();
    hooks_registered_ = false;
    ResetAntiDupeState();
}

PluginModule& GetPluginModule() {
    static PluginModule module;
    return module;
}

}  // namespace FasolaFlintScaler

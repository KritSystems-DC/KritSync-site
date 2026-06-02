#include "FasolaFlintScaler/PluginModule.h"

#include <sstream>

namespace FasolaFlintScaler {
namespace {

std::string BoolText(bool value) {
    return value ? "true" : "false";
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

void PluginModule::RegisterHooks() {
    if (hooks_registered_) {
        return;
    }

    hooks_registered_ = api_.RegisterHarvestHooks([this](const FlintContext& context) {
        if (config_.DebugLogging) {
            std::ostringstream line;
            line << "FasolaFlintScaler debug: flint detected amount=" << context.amount
                 << ", inventory_class_bp=" << context.inventory_blueprint
                 << ", owner_bp=" << context.owner_blueprint
                 << ", owner_class_bp=" << context.owner_class_blueprint;
            logger_.Debug(line.str());
        }

        const auto decision = scaler_.Evaluate(context);
        if (!decision.applied) {
            if (config_.DebugLogging) {
                logger_.Debug("FasolaFlintScaler debug: flint inventory rejected. amount="
                              + std::to_string(context.amount)
                              + ", inventory_class_bp=" + context.inventory_blueprint
                              + ", owner_bp=" + context.owner_blueprint
                              + ", owner_class_bp=" + context.owner_class_blueprint);
            }
            return;
        }

        api_.AddFlintToInventory(context, decision.add_amount);
        if (config_.ShowHudNotificationForExtraFlint) {
            api_.NotifyExtraFlint(context, decision);
        }

        std::ostringstream line;
        line << "FasolaFlintScaler: topped up Fasola flint from " << decision.original_amount
             << " to " << decision.target_amount
             << " (+" << decision.add_amount << ")";
        logger_.Info(line.str());
    });
}

void PluginModule::UnregisterHooks() {
    if (!hooks_registered_) {
        return;
    }

    api_.UnregisterHarvestHooks();
    hooks_registered_ = false;
}

PluginModule& GetPluginModule() {
    static PluginModule module;
    return module;
}

}  // namespace FasolaFlintScaler

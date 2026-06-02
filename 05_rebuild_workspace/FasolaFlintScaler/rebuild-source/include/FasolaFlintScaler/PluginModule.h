#pragma once

#include "FasolaFlintScaler/ArkApiBridge.h"
#include "FasolaFlintScaler/Config.h"
#include "FasolaFlintScaler/HarvestScaler.h"
#include "FasolaFlintScaler/LicenseValidator.h"
#include "FasolaFlintScaler/Logger.h"

namespace FasolaFlintScaler {

class PluginModule {
public:
    void Load();
    void Unload();
    bool ReloadConfig(const char* source);
    void ReportStatus(const char* source) const;

private:
    bool LoadConfig(bool reload);
    void ValidateLicense();
    void RegisterCommands();
    void UnregisterCommands();
    void RegisterHooks();
    void UnregisterHooks();

    Logger logger_{};
    ArkApiBridge api_{};
    Config config_{};
    HarvestScaler scaler_{};
    bool license_valid_ = false;
    bool loaded_ = false;
    bool commands_registered_ = false;
    bool hooks_registered_ = false;
};

PluginModule& GetPluginModule();

}  // namespace FasolaFlintScaler

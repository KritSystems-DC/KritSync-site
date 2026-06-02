#pragma once

#include "FasolaFlintScaler/ArkApiBridge.h"
#include "FasolaFlintScaler/Config.h"
#include "FasolaFlintScaler/HarvestScaler.h"
#include "FasolaFlintScaler/LicenseValidator.h"
#include "FasolaFlintScaler/Logger.h"

#include <chrono>
#include <deque>
#include <string>

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
    void ResetAntiDupeState();

    struct AntiDupeResult {
        bool allowed = true;
        std::string reason;
    };

    struct RecentFlintAdd {
        std::chrono::steady_clock::time_point timestamp{};
        int amount = 0;
    };

    AntiDupeResult EvaluateAntiDupe(const FlintContext& context, const ScaleDecision& decision);
    void RecordAntiDupeAccepted(const FlintContext& context, const ScaleDecision& decision);
    std::string AntiDupeFingerprint(const FlintContext& context, const ScaleDecision& decision) const;

    Logger logger_{};
    ArkApiBridge api_{};
    Config config_{};
    HarvestScaler scaler_{};
    std::deque<RecentFlintAdd> recent_flint_adds_{};
    std::string last_anti_dupe_fingerprint_{};
    std::chrono::steady_clock::time_point last_anti_dupe_timestamp_{};
    bool license_valid_ = false;
    bool loaded_ = false;
    bool commands_registered_ = false;
    bool hooks_registered_ = false;
    bool has_last_anti_dupe_event_ = false;
};

PluginModule& GetPluginModule();

}  // namespace FasolaFlintScaler

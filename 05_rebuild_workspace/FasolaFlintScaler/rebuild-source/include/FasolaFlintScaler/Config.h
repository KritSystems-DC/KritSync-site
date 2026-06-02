#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace FasolaFlintScaler {

struct Config {
    bool Enabled = true;
    std::string LicenseKey;
    double HarvestMultiplier = 10.0;
    bool ReadHarvestAmountMultiplierFromIni = true;
    std::string GameUserSettingsPath;
    int BaselineOfficialFlint = 80;
    int TargetFlintMin = 350;
    int TargetFlintMax = 550;
    int MinTriggerAmount = 1;
    int MaxTriggerAmount = 120;
    int VariancePercent = 10;
    bool AntiDupeEnabled = true;
    bool LogAntiDupeEvents = true;
    int MaxExtraFlintPerStack = 700;
    int MaxExtraFlintPerSecond = 2500;
    int DuplicateDetectionWindowMs = 750;
    bool BlockNegativeOrZeroAdjustments = true;
    bool RequireFasolaOwnerMatch = true;
    bool ShowHudNotificationForExtraFlint = false;
    bool DebugLogging = false;
    std::vector<std::string> FasolaClasses{
        "Fasola_Character_BP_C",
        "Fasola_Character_BP_Aberrant_C"
    };
    std::string FlintClass = "PrimalItemResource_Flint_C";
};

struct ConfigLoadResult {
    Config config;
    bool used_defaults = false;
    std::vector<std::string> warnings;
};

ConfigLoadResult LoadConfigFile(const std::filesystem::path& path);
void NormalizeConfig(Config& config, std::vector<std::string>& warnings);
std::filesystem::path DefaultConfigPath();

}  // namespace FasolaFlintScaler

#include "FasolaFlintScaler/Config.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>
#include <utility>

namespace FasolaFlintScaler {
namespace {

template <class T>
void AssignIfPresent(const nlohmann::json& json, const char* key, T& value, std::vector<std::string>& warnings) {
    auto item = json.find(key);
    if (item == json.end()) {
        return;
    }

    try {
        value = item->get<T>();
    } catch (const std::exception& ex) {
        warnings.emplace_back(std::string("invalid config value for ") + key + ": " + ex.what());
    }
}

}  // namespace

std::filesystem::path DefaultConfigPath() {
    return std::filesystem::current_path() / "config.json";
}

ConfigLoadResult LoadConfigFile(const std::filesystem::path& path) {
    ConfigLoadResult result{};

    std::ifstream input(path);
    if (!input) {
        result.used_defaults = true;
        result.warnings.emplace_back("config.json not found, using defaults");
        NormalizeConfig(result.config, result.warnings);
        return result;
    }

    try {
        nlohmann::json json;
        input >> json;

        AssignIfPresent(json, "Enabled", result.config.Enabled, result.warnings);
        AssignIfPresent(json, "LicenseKey", result.config.LicenseKey, result.warnings);
        AssignIfPresent(json, "HarvestMultiplier", result.config.HarvestMultiplier, result.warnings);
        AssignIfPresent(json, "ReadHarvestAmountMultiplierFromIni", result.config.ReadHarvestAmountMultiplierFromIni, result.warnings);
        AssignIfPresent(json, "GameUserSettingsPath", result.config.GameUserSettingsPath, result.warnings);
        AssignIfPresent(json, "BaselineOfficialFlint", result.config.BaselineOfficialFlint, result.warnings);
        AssignIfPresent(json, "TargetFlintMin", result.config.TargetFlintMin, result.warnings);
        AssignIfPresent(json, "TargetFlintMax", result.config.TargetFlintMax, result.warnings);
        AssignIfPresent(json, "MinTriggerAmount", result.config.MinTriggerAmount, result.warnings);
        AssignIfPresent(json, "MaxTriggerAmount", result.config.MaxTriggerAmount, result.warnings);
        AssignIfPresent(json, "VariancePercent", result.config.VariancePercent, result.warnings);
        AssignIfPresent(json, "AntiDupeEnabled", result.config.AntiDupeEnabled, result.warnings);
        AssignIfPresent(json, "LogAntiDupeEvents", result.config.LogAntiDupeEvents, result.warnings);
        AssignIfPresent(json, "MaxExtraFlintPerStack", result.config.MaxExtraFlintPerStack, result.warnings);
        AssignIfPresent(json, "MaxExtraFlintPerSecond", result.config.MaxExtraFlintPerSecond, result.warnings);
        AssignIfPresent(json, "DuplicateDetectionWindowMs", result.config.DuplicateDetectionWindowMs, result.warnings);
        AssignIfPresent(json, "BlockNegativeOrZeroAdjustments", result.config.BlockNegativeOrZeroAdjustments, result.warnings);
        AssignIfPresent(json, "RequireFasolaOwnerMatch", result.config.RequireFasolaOwnerMatch, result.warnings);
        AssignIfPresent(json, "ShowHudNotificationForExtraFlint", result.config.ShowHudNotificationForExtraFlint, result.warnings);
        AssignIfPresent(json, "DebugLogging", result.config.DebugLogging, result.warnings);
        AssignIfPresent(json, "FasolaClasses", result.config.FasolaClasses, result.warnings);
        AssignIfPresent(json, "FlintClass", result.config.FlintClass, result.warnings);
    } catch (const std::exception& ex) {
        result.used_defaults = true;
        result.config = Config{};
        result.warnings.emplace_back(std::string("could not parse config.json, using defaults: ") + ex.what());
    }

    NormalizeConfig(result.config, result.warnings);
    return result;
}

void NormalizeConfig(Config& config, std::vector<std::string>& warnings) {
    if (config.HarvestMultiplier <= 0.0) {
        warnings.emplace_back("HarvestMultiplier must be greater than zero; using 10.0");
        config.HarvestMultiplier = 10.0;
    }

    if (config.BaselineOfficialFlint < 1) {
        warnings.emplace_back("BaselineOfficialFlint must be at least 1; using 80");
        config.BaselineOfficialFlint = 80;
    }

    if (config.TargetFlintMin < 1) {
        warnings.emplace_back("TargetFlintMin must be at least 1; using 350");
        config.TargetFlintMin = 350;
    }

    if (config.TargetFlintMax < config.TargetFlintMin) {
        warnings.emplace_back("TargetFlintMax was lower than TargetFlintMin; swapping values");
        std::swap(config.TargetFlintMin, config.TargetFlintMax);
    }

    if (config.MinTriggerAmount < 0) {
        config.MinTriggerAmount = 0;
    }

    if (config.MaxTriggerAmount < config.MinTriggerAmount) {
        warnings.emplace_back("MaxTriggerAmount was lower than MinTriggerAmount; swapping values");
        std::swap(config.MinTriggerAmount, config.MaxTriggerAmount);
    }

    if (config.VariancePercent < 0) {
        config.VariancePercent = 0;
    }

    if (config.MaxExtraFlintPerStack < 0) {
        warnings.emplace_back("MaxExtraFlintPerStack must not be negative; using 0");
        config.MaxExtraFlintPerStack = 0;
    }

    if (config.MaxExtraFlintPerSecond < 0) {
        warnings.emplace_back("MaxExtraFlintPerSecond must not be negative; using 0");
        config.MaxExtraFlintPerSecond = 0;
    }

    if (config.DuplicateDetectionWindowMs < 0) {
        warnings.emplace_back("DuplicateDetectionWindowMs must not be negative; using 0");
        config.DuplicateDetectionWindowMs = 0;
    }

    if (config.FasolaClasses.empty()) {
        warnings.emplace_back("FasolaClasses was empty; restoring defaults");
        config.FasolaClasses = {"Fasola_Character_BP_C", "Fasola_Character_BP_Aberrant_C"};
    }

    if (config.FlintClass.empty()) {
        warnings.emplace_back("FlintClass was empty; restoring default");
        config.FlintClass = "PrimalItemResource_Flint_C";
    }
}

}  // namespace FasolaFlintScaler

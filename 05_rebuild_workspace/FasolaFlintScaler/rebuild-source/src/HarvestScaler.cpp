#include "FasolaFlintScaler/HarvestScaler.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

namespace FasolaFlintScaler {
namespace {

bool ContainsClassName(const std::string& value, const std::string& class_name) {
    return !value.empty() && value.find(class_name) != std::string::npos;
}

std::string Trim(std::string value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return {};
    }

    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

}  // namespace

void HarvestScaler::Configure(const Config& config, const std::filesystem::path& plugin_directory) {
    config_ = config;
    plugin_directory_ = plugin_directory;
    effective_harvest_multiplier_ = config_.HarvestMultiplier;

    if (config_.ReadHarvestAmountMultiplierFromIni) {
        if (const auto value = ReadHarvestAmountMultiplier(ResolveGameUserSettingsPath(plugin_directory_))) {
            effective_harvest_multiplier_ = *value;
        }
    }
}

double HarvestScaler::ConfiguredHarvestMultiplier() const {
    return config_.HarvestMultiplier;
}

double HarvestScaler::EffectiveHarvestMultiplier() const {
    return effective_harvest_multiplier_;
}

int HarvestScaler::EstimatedTargetFlint() const {
    return static_cast<int>(std::lround((config_.TargetFlintMin + config_.TargetFlintMax) / 2.0));
}

ScaleDecision HarvestScaler::Evaluate(const FlintContext& context) {
    ScaleDecision decision{};
    decision.original_amount = context.amount;

    if (!config_.Enabled) {
        decision.reason = "plugin disabled";
        return decision;
    }

    if (!IsTriggerAmount(context.amount)) {
        decision.reason = "amount outside trigger range";
        return decision;
    }

    if (!IsFlint(context)) {
        decision.reason = "item is not flint";
        return decision;
    }

    if (!IsFasolaTarget(context)) {
        decision.reason = "owner is not a Fasola target";
        return decision;
    }

    const int target = PickTarget();
    const int minimum = static_cast<int>(std::floor(target * (100 - config_.VariancePercent) / 100.0));
    if (context.amount >= minimum) {
        decision.reason = "amount already within target variance";
        return decision;
    }

    decision.applied = true;
    decision.target_amount = target;
    decision.add_amount = std::max(0, target - context.amount);
    return decision;
}

std::optional<double> HarvestScaler::ReadHarvestAmountMultiplier(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        return std::nullopt;
    }

    std::string line;
    while (std::getline(input, line)) {
        const auto equals = line.find('=');
        if (equals == std::string::npos) {
            continue;
        }

        const auto key = Trim(line.substr(0, equals));
        if (key != "HarvestAmountMultiplier") {
            continue;
        }

        const auto value = Trim(line.substr(equals + 1));
        try {
            return std::stod(value);
        } catch (...) {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

std::filesystem::path HarvestScaler::ResolveGameUserSettingsPath(const std::filesystem::path& plugin_directory) const {
    if (!config_.GameUserSettingsPath.empty()) {
        return config_.GameUserSettingsPath;
    }

    return plugin_directory / ".." / ".." / ".." / "Saved" / "Config" / "WindowsServer" / "GameUserSettings.ini";
}

bool HarvestScaler::IsFlint(const FlintContext& context) const {
    return ContainsClassName(context.item_blueprint, config_.FlintClass)
        || ContainsClassName(context.item_archetype_blueprint, config_.FlintClass);
}

bool HarvestScaler::IsFasolaTarget(const FlintContext& context) const {
    for (const auto& class_name : config_.FasolaClasses) {
        if (ContainsClassName(context.owner_blueprint, class_name)
            || ContainsClassName(context.owner_class_blueprint, class_name)
            || ContainsClassName(context.inventory_blueprint, class_name)) {
            return true;
        }
    }

    return false;
}

bool HarvestScaler::IsTriggerAmount(int amount) const {
    return amount >= config_.MinTriggerAmount && amount <= config_.MaxTriggerAmount;
}

int HarvestScaler::PickTarget() {
    std::uniform_int_distribution<int> distribution(config_.TargetFlintMin, config_.TargetFlintMax);
    return distribution(rng_);
}

}  // namespace FasolaFlintScaler

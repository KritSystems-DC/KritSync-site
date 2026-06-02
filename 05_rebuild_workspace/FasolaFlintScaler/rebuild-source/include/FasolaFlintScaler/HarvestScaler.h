#pragma once

#include "FasolaFlintScaler/Config.h"

#include <filesystem>
#include <optional>
#include <random>
#include <string>

namespace FasolaFlintScaler {

struct FlintContext {
    int amount = 0;
    void* native_item = nullptr;
    void* native_inventory = nullptr;
    std::string item_blueprint;
    std::string item_archetype_blueprint;
    std::string inventory_blueprint;
    std::string owner_blueprint;
    std::string owner_class_blueprint;
};

struct ScaleDecision {
    bool applied = false;
    int original_amount = 0;
    int target_amount = 0;
    int add_amount = 0;
    std::string reason;
};

class HarvestScaler {
public:
    void Configure(const Config& config, const std::filesystem::path& plugin_directory);

    double ConfiguredHarvestMultiplier() const;
    double EffectiveHarvestMultiplier() const;
    int EstimatedTargetFlint() const;

    ScaleDecision Evaluate(const FlintContext& context);

private:
    static std::optional<double> ReadHarvestAmountMultiplier(const std::filesystem::path& path);
    std::filesystem::path ResolveGameUserSettingsPath(const std::filesystem::path& plugin_directory) const;
    bool IsFlint(const FlintContext& context) const;
    bool IsFasolaTarget(const FlintContext& context) const;
    bool IsTriggerAmount(int amount) const;
    int PickTarget();

    Config config_{};
    std::filesystem::path plugin_directory_{};
    double effective_harvest_multiplier_ = 10.0;
    std::mt19937 rng_{std::random_device{}()};
};

}  // namespace FasolaFlintScaler

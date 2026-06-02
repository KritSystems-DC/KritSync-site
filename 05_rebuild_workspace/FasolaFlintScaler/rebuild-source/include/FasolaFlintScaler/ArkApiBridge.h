#pragma once

#include "FasolaFlintScaler/HarvestScaler.h"

#include <filesystem>
#include <functional>
#include <string>

namespace FasolaFlintScaler {

class Logger;

class ArkApiBridge {
public:
    using CommandCallback = std::function<void()>;
    using FlintCallback = std::function<void(const FlintContext&)>;

    void Initialize(Logger* logger);
    void Shutdown();

    std::filesystem::path PluginDirectory() const;

    bool RegisterCommand(const std::string& command, CommandCallback callback);
    void UnregisterCommand(const std::string& command);

    bool RegisterHarvestHooks(FlintCallback callback);
    void UnregisterHarvestHooks();

    void DispatchFlintContext(const FlintContext& context);
    void NotifyExtraFlint(const FlintContext& context, const ScaleDecision& decision);
    void AddFlintToInventory(const FlintContext& context, int amount);

private:
    Logger* logger_ = nullptr;
    FlintCallback flint_callback_{};
    bool hooks_registered_ = false;
};

}  // namespace FasolaFlintScaler

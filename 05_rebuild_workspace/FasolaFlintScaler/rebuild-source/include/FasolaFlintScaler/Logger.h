#pragma once

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>

namespace FasolaFlintScaler {

class Logger {
public:
    void Open(const std::filesystem::path& plugin_directory, bool debug_enabled);
    void SetDebugEnabled(bool enabled);
    void Close();

    void Info(const std::string& message);
    void Warn(const std::string& message);
    void Error(const std::string& message);
    void Debug(const std::string& message);

private:
    void Write(const char* level, const std::string& message);

    std::mutex mutex_;
    std::ofstream debug_file_;
    bool debug_enabled_ = false;
};

}  // namespace FasolaFlintScaler

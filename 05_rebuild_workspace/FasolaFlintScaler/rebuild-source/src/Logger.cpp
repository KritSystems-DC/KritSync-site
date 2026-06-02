#include "FasolaFlintScaler/Logger.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace FasolaFlintScaler {

void Logger::Open(const std::filesystem::path& plugin_directory, bool debug_enabled) {
    std::lock_guard lock(mutex_);
    debug_enabled_ = debug_enabled;

    if (debug_enabled_) {
        debug_file_.open(plugin_directory / "FasolaFlintScaler.debug.log", std::ios::app);
    }
}

void Logger::SetDebugEnabled(bool enabled) {
    std::lock_guard lock(mutex_);
    debug_enabled_ = enabled;
}

void Logger::Close() {
    std::lock_guard lock(mutex_);
    if (debug_file_.is_open()) {
        debug_file_.flush();
        debug_file_.close();
    }
}

void Logger::Info(const std::string& message) {
    Write("info", message);
}

void Logger::Warn(const std::string& message) {
    Write("warn", message);
}

void Logger::Error(const std::string& message) {
    Write("error", message);
}

void Logger::Debug(const std::string& message) {
    if (debug_enabled_) {
        Write("debug", message);
    }
}

void Logger::Write(const char* level, const std::string& message) {
    std::lock_guard lock(mutex_);

    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_s(&tm, &time);

    std::ostringstream line;
    line << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " [" << level << "] " << message;

    std::cout << line.str() << '\n';

    if (debug_file_.is_open()) {
        debug_file_ << line.str() << '\n';
        debug_file_.flush();
    }
}

}  // namespace FasolaFlintScaler

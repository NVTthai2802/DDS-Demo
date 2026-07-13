#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>

class Logger {
public:
    static void info(const std::string& msg) {
        log("INFO", msg);
    }

    static void warn(const std::string& msg) {
        log("WARN", msg);
    }

    static void error(const std::string& msg) {
        log("ERROR", msg);
    }

private:
    static void log(const std::string& level, const std::string& msg) {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::cerr << "[" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << "." 
                  << std::setfill('0') << std::setw(3) << ms.count() << "] "
                  << "[" << level << "] " << msg << std::endl;
    }
};

#define LOG_INFO(msg) Logger::info(msg)
#define LOG_WARN(msg) Logger::warn(msg)
#define LOG_ERROR(msg) Logger::error(msg)

#endif // LOGGER_HPP

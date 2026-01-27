/*
 * Logger utility for timestamped output
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

/**
 * Logger class providing info(), warn(), and error() methods with timestamps
 * Timestamps are formatted according to LOG_TIMESTAMP environment variable.
 * If LOG_TIMESTAMP is not set, uses default format "%Y-%m-%d %T"
 */
class Logger {
private:
    /**
     * Get current timestamp formatted according to LOG_TIMESTAMP environment variable.
     * @return Formatted timestamp string in brackets
     */
    static std::string getTimestamp() {
        const char* format_env = std::getenv("LOG_TIMESTAMP");
        std::string format = format_env ? std::string(format_env) : "%Y-%m-%d %T";

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << "[" << std::put_time(std::localtime(&time), format.c_str()) << "]";
        return ss.str();
    }

public:
    /**
     * Log info message to stdout
     * Format: [timestamp] INFO: message
     */
    static void info(const std::string& message) {
        std::cout << getTimestamp() << " INFO: " << message << std::endl;
    }

    /**
     * Log warning message to stderr
     * Format: [timestamp] WARNING: message
     */
    static void warn(const std::string& message) {
        std::cerr << getTimestamp() << " WARNING: " << message << std::endl;
    }

    /**
     * Log error message to stderr
     * Format: [timestamp] ERROR: message
     */
    static void error(const std::string& message) {
        std::cerr << getTimestamp() << " ERROR: " << message << std::endl;
    }
};

#endif // LOGGER_H

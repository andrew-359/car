#pragma once

#include <Arduino.h>
#include "Config.h"

//TODO в утилс
class Logger {
public:
    static void begin() {
        Serial.begin(Config::SERIAL_BAUD_RATE);
    }

    template<typename... Args>
    static void debug(const char* format, Args... args) {
        print(Config::Logger::LogLevel::DEBUG, "DEBUG", format, args...);
    }

    template<typename... Args>
    static void info(const char* format, Args... args) {
        print(Config::Logger::LogLevel::INFO, "INFO", format, args...);
    }

    template<typename... Args>
    static void warn(const char* format, Args... args) {
        print(Config::Logger::LogLevel::WARN, "WARN", format, args...);
    }

    template<typename... Args>
    static void error(const char* format, Args... args) {
        print(Config::Logger::LogLevel::ERROR, "ERROR", format, args...);
    }

private:
    template<typename... Args>
    static void print(Config::Logger::LogLevel level, const char* levelStr, const char* format, Args... args) {
        if (level <= Config::Logger::MIN_LOG_LEVEL) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "[%s] (%lums) ", levelStr, millis());
            Serial.print(buffer);
            
            // Clear buffer for the main message
            memset(buffer, 0, sizeof(buffer));
            snprintf(buffer, sizeof(buffer), format, args...);
            Serial.println(buffer);
        }
    }
};
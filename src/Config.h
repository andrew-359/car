#pragma once

#include <cstddef> // для size_t
#include <cstdint> // для uint16_t

namespace Config {
    // Serial (Последовательный порт)
    constexpr long SERIAL_BAUD_RATE = 115200;

    // Логгер
    namespace Logger {
        enum class LogLevel { NONE, ERROR, WARN, INFO, DEBUG };
        // Устанавливаем минимальный уровень логов для вывода
        constexpr LogLevel MIN_LOG_LEVEL = LogLevel::DEBUG;
    }

    // Защита от сбоев
    constexpr unsigned long FAILSAFE_TIMEOUT_MS = 500;

    // Сеть
    namespace Network {
        constexpr uint16_t HTTP_PORT = 80;
        constexpr const char* WIFI_AP_SSID = "ESP32-Car-Setup";
        constexpr const char* PREFERENCES_NAMESPACE = "car-wifi";
        constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 10000;
    }

    // Веб-сервер
    namespace Web {
        constexpr const char* WS_ENDPOINT = "/ws";
        constexpr const char* ROUTE_ROOT = "/";
        constexpr const char* ROUTE_WIFI_SETUP = "/wifi";
        constexpr const char* ROUTE_WIFI_SAVE = "/save-wifi";
    }

    // WebSocket
    namespace WebSocket {
        constexpr size_t JSON_DOC_SIZE = 128;
        constexpr const char* KEY_THROTTLE = "t";
        constexpr const char* KEY_STEER = "s";
    }

    // Контроллер моторов
    namespace Motor {
        // Общий пин Standby для драйвера
        constexpr int STBY_PIN = 13;

        // ШИМ (PWM)
        constexpr int PWM_FREQ = 5000;
        constexpr int PWM_RESOLUTION = 8;
        constexpr int MAX_SPEED = 255;

        // Структура для описания одного мотора
        struct MotorPins {
            const int pwm; const int in1; const int in2; const int channel;
        };

        // Конфигурация левого и правого моторов
        constexpr MotorPins LEFT_MOTOR  = { .pwm = 26, .in1 = 25, .in2 = 33, .channel = 0 };
        constexpr MotorPins RIGHT_MOTOR = { .pwm = 12, .in1 = 14, .in2 = 27, .channel = 1 };
    }
}
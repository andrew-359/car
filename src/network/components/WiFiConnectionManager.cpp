#include "network/components/WiFiConnectionManager.h"
#include <WiFi.h>
#include "Config.h"
#include "tech/Logger.h"

WiFiConnectionManager::WiFiConnectionManager(Preferences& preferences) 
    : _preferences(preferences), _state(State::IDLE), _lastAttemptTime(0) 
{
    // Заполняем таблицу поиска для конечного автомата
    _stateLogic[static_cast<size_t>(State::CONNECTING_STA)] = {
        /* onEnter */ [this]() {
            _preferences.begin(Config::Network::PREFERENCES_NAMESPACE, true);
            String ssid = _preferences.getString("ssid", "");
            String pass = _preferences.getString("password", "");
            _preferences.end();

            if (ssid.length() > 0) {
                Logger::info("Подключение к: %s", ssid.c_str());
                WiFi.mode(WIFI_STA);
                WiFi.begin(ssid.c_str(), pass.c_str());
                _lastAttemptTime = millis();
            } else {
                Logger::info("Нет сохраненных сетей. Переключение в режим точки доступа.");
                _changeState(State::STARTING_AP);
            }
        },
        /* onLoop */ [this]() {
            if (WiFi.status() == WL_CONNECTED) {
                _changeState(State::CONNECTED_STA);
            } else if (millis() - _lastAttemptTime > Config::Network::WIFI_CONNECT_TIMEOUT_MS) {
                Logger::warn("Не удалось подключиться к сохраненной сети Wi-Fi. Переключение в режим точки доступа.");
                _changeState(State::STARTING_AP);
            }
        }
    };

    _stateLogic[static_cast<size_t>(State::CONNECTED_STA)] = {
        /* onEnter */ [this]() {
            Logger::info("Подключено! IP адрес: %s", WiFi.localIP().toString().c_str());
            if (onStaConnected) onStaConnected();
        },
        /* onLoop */ [this]() {
            if (WiFi.status() != WL_CONNECTED) {
                Logger::warn("Потеряно соединение Wi-Fi. Попытка переподключения...");
                _changeState(State::CONNECTING_STA);
            }
        }
    };

    _stateLogic[static_cast<size_t>(State::STARTING_AP)] = {
        /* onEnter */ [this]() {
            Logger::info("Запуск в режиме точки доступа. Подключитесь к '%s' и перейдите на 192.168.4.1", Config::Network::WIFI_AP_SSID);
            WiFi.mode(WIFI_AP);
            WiFi.softAP(Config::Network::WIFI_AP_SSID);
            Logger::info("IP адрес точки доступа: %s", WiFi.softAPIP().toString().c_str());
            if (onApModeStarted) onApModeStarted();
            _changeState(State::AP_MODE);
        },
        /* onLoop */ nullptr
    };

    // Добавляем обработчики для оставшихся состояний, чтобы массив был полным
    _stateLogic[static_cast<size_t>(State::IDLE)] = {
        /* onEnter */ nullptr,
        /* onLoop */ nullptr
    };

    _stateLogic[static_cast<size_t>(State::AP_MODE)] = {
        /* onEnter */ nullptr,
        /* onLoop */ nullptr
    };
}

void WiFiConnectionManager::begin() {
    _changeState(State::CONNECTING_STA);
}

void WiFiConnectionManager::loop() {
    if (auto& handler = _stateLogic[static_cast<size_t>(_state)]; handler.onLoop) {
        handler.onLoop();
    }
}

void WiFiConnectionManager::_changeState(State newState) {
    if (_state == newState) return;
    _state = newState;
    if (auto& handler = _stateLogic[static_cast<size_t>(_state)]; handler.onEnter) {
        handler.onEnter();
    }
}
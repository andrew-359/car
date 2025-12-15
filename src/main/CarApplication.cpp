#include "main/CarApplication.h"
#include "Config.h"
#include <LittleFS.h>
#include "tech/Logger.h"

CarApplication::CarApplication(MotorController& motorController, NetworkManager& networkManager, sensors::ProximitySensor& proximitySensor)
    : _motorController(motorController), 
      _networkManager(networkManager),
      _proximitySensor(proximitySensor)
{
    // Заполняем таблицу поиска для конечного автомата
    _stateHandlers[static_cast<size_t>(State::IDLE)] = StateHandlers {
        /* label */ "IDLE",
        /* onEnter */ [this]() {
            Logger::info("Вход в состояние IDLE.");
            _motorController.stop();
        },
        /* onExit */ nullptr,
        /* onLoop */ nullptr
    };

    _stateHandlers[static_cast<size_t>(State::DRIVING)] = StateHandlers {
        /* label */ "DRIVING",
        /* onEnter */ nullptr,
        /* onExit */ [this]() {
            Logger::info("Выход из состояния DRIVING.");
        },
        /* onLoop */ [this]() {
            // Проверяем таймаут потери связи
            if (_lastCmdTime != 0 && millis() - _lastCmdTime > Config::FAILSAFE_TIMEOUT_MS) {
                _changeState(State::FAILSAFE);
            }
        }
    };

    _stateHandlers[static_cast<size_t>(State::FAILSAFE)] = StateHandlers {
        /* label */ "FAILSAFE",
        /* onEnter */ [this]() {
            Logger::warn("Вход в состояние FAILSAFE. Остановка моторов.");
            _motorController.stop();
            _lastCmdTime = 0; // Сбрасываем таймер потери связи
            _activeClientId = 0; // Сбрасываем активного клиента
        },
        /* onExit */ nullptr,
        /* onLoop */ nullptr
    };
}

void CarApplication::setup() {
    Logger::begin();
    Logger::info("Запуск CarApplication...");

    _motorController.begin();
    _proximitySensor.setup(); // Инициализируем новый модуль

    if (!LittleFS.begin(true)) {
        Logger::error("Произошла ошибка при монтировании LittleFS");
        return;
    }

    // --- Связываем компоненты ---
    _networkManager.onControlCommand = [this](AsyncWebSocketClient* client, int throttle, int steer) {
        //TODO сделать нормально
        // --- Логика автотормоза ---
        float distance = _proximitySensor.getDistanceCm();
        if (throttle > 0 && distance < Config::Proximity::MIN_BRAKE_DISTANCE_CM) {
            Logger::warn("Препятствие на расстоянии %.1f см! Автотормоз.", distance);
            _motorController.setSpeed(0, steer); // Блокируем движение вперед, но разрешаем поворот на месте
        } else {
            _motorController.setSpeed(throttle, steer);
        }

        // Последний, кто отдал команду, становится активным водителем
        _activeClientId = client->id();
        _changeState(State::DRIVING);
        _lastCmdTime = millis(); // Сбрасываем таймер failsafe
    };

    _networkManager.onClientDisconnect = [this](AsyncWebSocketClient* client) {
        Logger::info("Клиент WebSocket #%u отключился.", client->id());
        // Переходим в FAILSAFE, только если отключился активный клиент
        if (client->id() == _activeClientId) {
            _changeState(State::FAILSAFE);
        }
    };

    _networkManager.onClientConnect = [this](AsyncWebSocketClient* client) {
        Logger::info("Клиент WebSocket #%u подключен с IP: %s", client->id(), client->remoteIP().toString().c_str());
        // Если мы были в аварийном состоянии, новое подключение переводит нас в режим ожидания
        if (_currentState == State::FAILSAFE) {
            _changeState(State::IDLE);
        }
    };

    // Запускаем сетевой менеджер, который поднимет Wi-Fi и веб-сервер
    _networkManager.begin();

    _changeState(State::IDLE);
}

void CarApplication::loop() {
    _networkManager.loop(); // Даем сетевому менеджеру выполнять свои задачи
    
    // Выполняем действие onLoop для текущего состояния, если оно существует
    if (auto& handler = _stateHandlers[static_cast<size_t>(_currentState)]; handler.onLoop) {
        handler.onLoop();
    }
}

//TODO не нравится этот код
void CarApplication::_changeState(State newState) {
    if (_currentState == newState) return;

    // 1. Выполняем действие onExit для СТАРОГО состояния
    if (auto& handler = _stateHandlers[static_cast<size_t>(_currentState)]; handler.onExit) {
        handler.onExit();
    }

    _currentState = newState;

    // 2. Выполняем действие onEnter для НОВОГО состояния
    if (auto& handler = _stateHandlers[static_cast<size_t>(_currentState)]; handler.onEnter) {
        handler.onEnter();
    }

    // 3. Оповещаем всех клиентов о смене состояния
    _networkManager.broadcastStatus(_stateHandlers[static_cast<size_t>(_currentState)].label);
}
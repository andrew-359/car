#pragma once

#include <Arduino.h>
#include <functional>
#include <map>
#include "tech/MotorController.h"
#include "Config.h"
#include "network/NetworkManager.h"
#include "sensors/ProximitySensor.h" // Заменяем на новый модуль

class CarApplication {
public:
    // Внедрение зависимостей: получаем компоненты через конструктор
    CarApplication(MotorController& motorController, NetworkManager& networkManager, sensors::ProximitySensor& proximitySensor);

    void setup();
    void loop();

private:
    // --- Конечный автомат ---
    enum class State { IDLE, DRIVING, FAILSAFE };

    struct StateHandlers {
        const char* label;             // Текстовое представление состояния
        std::function<void()> onEnter; // Действие при входе в состояние
        std::function<void()> onExit;  // Действие при выходе из состояния
        std::function<void()> onLoop;  // Действие на каждом тике цикла
    };

    void _changeState(State newState);

    // --- Компоненты приложения ---
    MotorController& _motorController;
    NetworkManager& _networkManager;
    sensors::ProximitySensor& _proximitySensor;

    // --- Состояние приложения ---
    State _currentState = State::IDLE;
    unsigned long _lastCmdTime = 0;
    uint32_t _activeClientId = 0; // ID клиента, который управляет машинкой
    std::map<State, StateHandlers> _stateHandlers;
};
#pragma once

#include <Preferences.h>
#include <functional>
#include <array>

class WiFiConnectionManager {
public:
    enum class State {
        IDLE,
        CONNECTING_STA,
        CONNECTED_STA,
        STARTING_AP,
        AP_MODE
    };

    // Определяем количество состояний для использования в std::array
    static constexpr size_t STATE_COUNT = 5;

    // Структура для хранения логики состояний
    struct StateLogic {
        std::function<void()> onEnter;
        std::function<void()> onLoop;
    };

    WiFiConnectionManager(Preferences& preferences);
    void begin();
    void loop();

    // Коллбэки для уведомления о готовности сети
    std::function<void()> onStaConnected;
    std::function<void()> onApModeStarted;

private:
    Preferences& _preferences;
    State _state = State::IDLE;
    std::array<StateLogic, STATE_COUNT> _stateLogic;
    unsigned long _lastAttemptTime = 0;

    void _changeState(State newState);
};
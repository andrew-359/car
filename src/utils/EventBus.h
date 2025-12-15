#pragma once

#include <functional>
//типа шина событий =))
class EventBus {
public:
    // --- События ---
    std::function<void()> onRestartRequired;

    // --- Методы для публикации ---
    void publishRestartRequired();
};

// Глобальный экземпляр шины событий, доступный из любого места программы
extern EventBus GlobalEventBus;

#include "utils/EventBus.h"

// Определение глобального экземпляра
EventBus GlobalEventBus;

void EventBus::publishRestartRequired() {
    if (onRestartRequired) onRestartRequired();
}

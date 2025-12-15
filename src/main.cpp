#include "main/CarApplication.h"
#include "tech/Logger.h"
#include <Ticker.h>
#include "utils/EventBus.h"

// Создаем экземпляры компонентов
MotorController motorController;
NetworkManager networkManager;

// Глобальный Ticker для отложенных действий, таких как перезагрузка
Ticker globalTicker;

// Создаем единственный экземпляр нашего приложения,
// внедряя зависимости через конструктор.
CarApplication app(motorController, networkManager);

void setup() {
  // Подписываемся на событие перезагрузки из глобальной шины событий.
  GlobalEventBus.onRestartRequired = []() {
    Logger::info("Перезагрузка системы запланирована через 1 секунду...");
    globalTicker.once_ms(1000, []() { ESP.restart(); });
  };

  app.setup();
}

void loop() {
  app.loop();
}
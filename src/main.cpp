#include "main/CarApplication.h"
#include "tech/Logger.h"
#include <Ticker.h>
#include "utils/EventBus.h"
#include "sensors/ProximitySensor.h" // Заменяем на новый модуль
 
// --- Компоненты приложения ---
// Создаем экземпляры в этом файле. Ключевое слово 'static' ограничивает их
// видимость только этим файлом (main.cpp), предотвращая случайное
// использование в других частях программы.
static MotorController motorController;
static NetworkManager networkManager;
static sensors::ProximitySensor proximitySensor; // Создаем экземпляр сенсора приближения
static Ticker globalTicker;
 
// Создаем единственный экземпляр нашего приложения,
// внедряя зависимости через конструктор.
static CarApplication app(motorController, networkManager, proximitySensor);

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
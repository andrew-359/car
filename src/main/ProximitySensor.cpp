#include "sensors/ProximitySensor.h"
#include "tech/Logger.h"
#include "Config.h"
#include <Arduino.h>

//TODO refactoring
namespace sensors {

ProximitySensor::ProximitySensor() {
    _dataMutex = xSemaphoreCreateMutex();
}

ProximitySensor::~ProximitySensor() {
    if (_taskHandle != NULL) vTaskDelete(_taskHandle);
    if (_dataMutex != NULL) vSemaphoreDelete(_dataMutex);
}

void ProximitySensor::setup() {
    pinMode(Config::Proximity::TRIG_PIN, OUTPUT);
    pinMode(Config::Proximity::ECHO_PIN, INPUT);

    xTaskCreatePinnedToCore(
        sensorReadTask, "Proximity Task", 2048, this, 1, &_taskHandle, 0);
    
    logger::info("Сенсор приближения запущен.");
}

float ProximitySensor::getDistanceCm() {
    float localData;
    if (xSemaphoreTake(_dataMutex, (TickType_t)10) == pdTRUE) {
        localData = _distanceCm;
        xSemaphoreGive(_dataMutex);
    }
    return localData;
}

void ProximitySensor::sensorReadTask(void* pvParameters) {
    ProximitySensor* self = static_cast<ProximitySensor*>(pvParameters);
    for (;;) {
        self->readAndProcess();
        vTaskDelay(pdMS_TO_TICKS(Config::Proximity::SAMPLE_INTERVAL_MS));
    }
}

void ProximitySensor::readAndProcess() {
    // Алгоритм для HC-SR04
    digitalWrite(Config::Proximity::TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(Config::Proximity::TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(Config::Proximity::TRIG_PIN, LOW);

    // Читаем длительность эхо-сигнала
    long duration = pulseIn(Config::Proximity::ECHO_PIN, HIGH, 25000); // 25ms таймаут

    // Рассчитываем дистанцию
    float distance = duration * 0.034 / 2;

    if (xSemaphoreTake(_dataMutex, (TickType_t)10) == pdTRUE) {
        _distanceCm = (distance > 0) ? distance : 999.0f; // Если таймаут, ставим большое значение
        xSemaphoreGive(_dataMutex);
    }
}

} // namespace sensors
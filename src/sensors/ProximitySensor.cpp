#include "sensors/ProximitySensor.h"
#include "tech/Logger.h"
#include "Config.h"
#include <Arduino.h>

namespace sensors {

// Инициализация статического указателя
ProximitySensor* ProximitySensor::_instance = nullptr;

ProximitySensor::ProximitySensor() {
    _dataMutex = xSemaphoreCreateMutex();
    _measurementReadySemaphore = xSemaphoreCreateBinary();
    _instance = this; // Сохраняем указатель на текущий экземпляр
}

ProximitySensor::~ProximitySensor() {
    if (_taskHandle != NULL) vTaskDelete(_taskHandle);
    if (_dataMutex != NULL) vSemaphoreDelete(_dataMutex);
    if (_measurementReadySemaphore != NULL) vSemaphoreDelete(_measurementReadySemaphore);
}

void ProximitySensor::setup() {
    pinMode(Config::Proximity::TRIG_PIN, OUTPUT);
    pinMode(Config::Proximity::ECHO_PIN, INPUT);

    // Привязываем прерывание к пину ECHO.
    // Функция _echo_isr будет вызываться при любом изменении уровня сигнала (CHANGE).
    attachInterrupt(digitalPinToInterrupt(Config::Proximity::ECHO_PIN), _echo_isr, CHANGE);

    xTaskCreatePinnedToCore(
        sensorReadTask, "Proximity Task", 2048, this, 2, &_taskHandle, 0);
    
    Logger::info("Сенсор приближения запущен.");
}

float ProximitySensor::getDistanceCm() {
    float localData;
    if (xSemaphoreTake(_dataMutex, (TickType_t)10) == pdTRUE) {
        localData = _distanceCm;
        xSemaphoreGive(_dataMutex);
    }
    return localData;
}

// Обработчик прерывания. Должен быть максимально быстрым.
// IRAM_ATTR говорит компилятору поместить эту функцию в быструю память IRAM.
void IRAM_ATTR ProximitySensor::_echo_isr() {
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    if (digitalRead(Config::Proximity::ECHO_PIN) == HIGH) {
        _instance->_echoStartTime = micros();
    } else {
        _instance->_echoStopTime = micros();
        // Измерение завершено, отдаем семафор задаче
        xSemaphoreGiveFromISR(_instance->_measurementReadySemaphore, &higherPriorityTaskWoken);
    }

    // Если отдача семафора разбудила задачу с более высоким приоритетом,
    // немедленно переключаем контекст.
    if (higherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

void ProximitySensor::sensorReadTask(void* pvParameters) {
    ProximitySensor* self = static_cast<ProximitySensor*>(pvParameters);
    for (;;) {
        // 1. Отправляем триггерный импульс
        digitalWrite(Config::Proximity::TRIG_PIN, LOW);
        delayMicroseconds(2);
        digitalWrite(Config::Proximity::TRIG_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(Config::Proximity::TRIG_PIN, LOW);

        // 2. Ждем, пока ISR не сообщит о завершении измерения, с таймаутом в 50 мс
        if (xSemaphoreTake(self->_measurementReadySemaphore, pdMS_TO_TICKS(50)) == pdTRUE) {
            // 3. Атомарно считываем время, зафиксированное в прерывании
            noInterrupts();
            unsigned long startTime = self->_echoStartTime;
            unsigned long stopTime = self->_echoStopTime;
            interrupts();

            // 4. Рассчитываем дистанцию
            unsigned long duration = (stopTime > startTime) ? (stopTime - startTime) : 0;
            float distance = duration * 0.0343 / 2;

            if (xSemaphoreTake(self->_dataMutex, (TickType_t)10) == pdTRUE) {
                self->_distanceCm = (distance > 0 && distance < 400) ? distance : 999.0f; // Отфильтровываем невалидные значения
                xSemaphoreGive(self->_dataMutex);
            }
        } else {
            // Таймаут: сенсор не ответил. Записываем невалидное значение.
            if (xSemaphoreTake(self->_dataMutex, (TickType_t)10) == pdTRUE) {
                self->_distanceCm = 999.0f;
                xSemaphoreGive(self->_dataMutex);
            }
        }

        // 5. Ждем до следующего цикла опроса
        vTaskDelay(pdMS_TO_TICKS(Config::Proximity::SAMPLE_INTERVAL_MS));
    }
}

} // namespace sensors
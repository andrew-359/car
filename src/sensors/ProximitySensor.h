#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h> // Для мьютекса

namespace sensors {

class ProximitySensor {
public:
    ProximitySensor();
    ~ProximitySensor();

    // Инициализирует сенсор и запускает фоновую задачу
    void setup();

    // Потокобезопасный метод для получения расстояния в см
    float getDistanceCm();

private:
    // Статический метод для обработчика прерываний
    static void IRAM_ATTR _echo_isr();

    static void sensorReadTask(void* pvParameters);

    static ProximitySensor* _instance; // Указатель на экземпляр для доступа из ISR

    TaskHandle_t _taskHandle = NULL;
    SemaphoreHandle_t _dataMutex = NULL;
    SemaphoreHandle_t _measurementReadySemaphore = NULL; // Семафор для сигнала о готовности измерения
    float _distanceCm = 999.0f; // Общие данные, инициализируем большим значением
    volatile unsigned long _echoStartTime = 0;
    volatile unsigned long _echoStopTime = 0;
};

} // namespace sensors
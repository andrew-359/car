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
    static void sensorReadTask(void* pvParameters);
    void readAndProcess();

    TaskHandle_t _taskHandle = NULL;
    SemaphoreHandle_t _dataMutex = NULL;
    float _distanceCm = 999.0f; // Общие данные, инициализируем большим значением
};

} // namespace sensors
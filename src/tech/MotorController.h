#pragma once // Защита от двойного включения

#include <Arduino.h>
#include "Config.h"

class MotorController {
public:
    // Конструктор: здесь можно передать пины, если не использовать #define
    MotorController();

    // Инициализация пинов и PWM
    void begin();

    // Установка скорости и поворота
    void setSpeed(int throttle, int steer);

    // Полная остановка моторов
    void stop();

private:
    void driveMotor(const Config::Motor::MotorPins& motor, int speed);
};
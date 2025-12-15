#include "tech/MotorController.h"
#include "Config.h"

// --- Детали реализации для таблицы поиска ---
namespace {
    // Структура для хранения состояния пинов направления
    struct MotorPinState {
        int in1;
        int in2;
    };

    // Таблица поиска состояний мотора.
    // Индекс соответствует знаку скорости + 1.
    // Index 0: Назад (для знака -1)
    // Index 1: Стоп (для знака 0)
    // Index 2: Вперед (для знака 1)
    constexpr MotorPinState stateTable[] = {
        {LOW, HIGH},  // Назад
        {LOW, LOW},   // Стоп
        {HIGH, LOW}   // Вперед
    };
}

MotorController::MotorController() {
    // Конструктор пока пуст
}

void MotorController::begin() {
    pinMode(Config::Motor::STBY_PIN, OUTPUT);
    digitalWrite(Config::Motor::STBY_PIN, HIGH); // Активируем драйвер

    // Настройка пинов и ШИМ для левого мотора
    pinMode(Config::Motor::LEFT_MOTOR.in1, OUTPUT);
    pinMode(Config::Motor::LEFT_MOTOR.in2, OUTPUT);
    ledcSetup(Config::Motor::LEFT_MOTOR.channel, Config::Motor::PWM_FREQ, Config::Motor::PWM_RESOLUTION);
    ledcAttachPin(Config::Motor::LEFT_MOTOR.pwm, Config::Motor::LEFT_MOTOR.channel);

    // Настройка пинов и ШИМ для правого мотора
    pinMode(Config::Motor::RIGHT_MOTOR.in1, OUTPUT);
    pinMode(Config::Motor::RIGHT_MOTOR.in2, OUTPUT);
    ledcSetup(Config::Motor::RIGHT_MOTOR.channel, Config::Motor::PWM_FREQ, Config::Motor::PWM_RESOLUTION);
    ledcAttachPin(Config::Motor::RIGHT_MOTOR.pwm, Config::Motor::RIGHT_MOTOR.channel);

    stop(); // Убедимся, что при старте моторы стоят
}

void MotorController::setSpeed(int throttle, int steer) {
    // Логика танкового управления с ограничением скорости
    long rawLeft = (long)throttle + steer;
    long rawRight = (long)throttle - steer;

    int leftSpeed = constrain(rawLeft, -Config::Motor::MAX_SPEED, Config::Motor::MAX_SPEED);
    int rightSpeed = constrain(rawRight, -Config::Motor::MAX_SPEED, Config::Motor::MAX_SPEED);

    driveMotor(Config::Motor::LEFT_MOTOR, leftSpeed);
    driveMotor(Config::Motor::RIGHT_MOTOR, rightSpeed);
}

void MotorController::stop() {
    setSpeed(0, 0);
}

void MotorController::driveMotor(const Config::Motor::MotorPins& motor, int speed) {
    // Вычисляем знак скорости: -1 для отрицательной, 0 для нуля, 1 для положительной.
    int sign = (speed > 0) - (speed < 0);

    // Используем знак для прямого доступа к состоянию в таблице (со смещением +1)
    const auto& state = stateTable[sign + 1];

    digitalWrite(motor.in1, state.in1);
    digitalWrite(motor.in2, state.in2);
    ledcWrite(motor.channel, abs(speed));
}
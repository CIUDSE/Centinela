// ControlMotor.h
#ifndef CONTROLMOTOR_H
#define CONTROLMOTOR_H

#include <Arduino.h>

// Pines motores
#define FRONT_R_PWM 27
#define DIR_R       32

#define FRONT_L_PWM 26
#define DIR_L       25

// I2C (ESP32)
// #define I2C_SLAVE_SCL_IO 22
// #define I2C_SLAVE_SDA_IO 21
#define I2C_SLAVE_ADDR   0x08

#define MAX_PWM 155

void setupMotorPins();
void setMotor(int dirPin, int pwmPin, int speed);
void stopMotors();
void controlDrive(int x, int y, bool brakes);

#endif // CONTROLMOTOR_H

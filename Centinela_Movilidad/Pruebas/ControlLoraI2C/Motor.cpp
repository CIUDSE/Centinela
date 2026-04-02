#include "ControlMotor.h"

void setupMotorPins() {
  pinMode(DIR_R, OUTPUT);
  pinMode(FRONT_R_PWM, OUTPUT);

  pinMode(DIR_L, OUTPUT);
  pinMode(FRONT_L_PWM, OUTPUT);

  analogWrite(FRONT_R_PWM, 0);
  analogWrite(FRONT_L_PWM, 0);
}

int clampValue(int value, int minVal, int maxVal) {
  if (value < minVal) return minVal;
  if (value > maxVal) return maxVal;
  return value;
}

void setMotor(int dirPin, int pwmPin, int speed) {
  speed = clampValue(speed, -MAX_PWM, MAX_PWM);

  if (speed > 0) {
    digitalWrite(dirPin, HIGH);
    analogWrite(pwmPin, speed);
  } 
  else if (speed < 0) {
    digitalWrite(dirPin, LOW);
    analogWrite(pwmPin, -speed);
  } 
  else {
    analogWrite(pwmPin, 0);
  }
}

void stopMotors() {
  analogWrite(FRONT_R_PWM, 0);
  analogWrite(FRONT_L_PWM, 0);
}

void controlDrive(int x, int y, bool brakes) {
  if (brakes) {
    stopMotors();
    return;
  }

  x = clampValue(x, -155, 155);
  y = clampValue(y, -155, 155);

  // zona muerta
  if (x > -10 && x < 10) x = 0;
  if (y > -10 && y < 10) y = 0;

  // mezclar joystick
  int leftMix  = y + x;
  int rightMix = y - x;

  leftMix = clampValue(leftMix, -310, 310);
  rightMix = clampValue(rightMix, -310, 310);

  // convertir de rango aprox -310..310 a -MAX_PWM..MAX_PWM
  int leftSpeed  = map(leftMix,  -310, 310, -MAX_PWM, MAX_PWM);
  int rightSpeed = map(rightMix, -310, 310, -MAX_PWM, MAX_PWM);

  leftSpeed  = clampValue(leftSpeed,  -MAX_PWM, MAX_PWM);
  rightSpeed = clampValue(rightSpeed, -MAX_PWM, MAX_PWM);

  setMotor(DIR_L, FRONT_L_PWM, leftSpeed);
  setMotor(DIR_R, FRONT_R_PWM, rightSpeed);
}

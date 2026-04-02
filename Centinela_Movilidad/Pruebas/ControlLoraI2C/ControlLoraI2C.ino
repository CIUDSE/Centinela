#include <Wire.h>
#include <stdint.h>
#include "ControlLora.h"

#include "ControlMotor.h"


void setup() {
  Serial.begin(115200);

  setupMotorPins();

  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onReceive(onReceive);

  Serial.println("ESP32 esclavo listo");
}

void loop() {
  if (newData) {
  ControlData data;

  noInterrupts();
  data.x = rxData.x;
  data.y = rxData.y;
  data.brakes = rxData.brakes;
  newData = false;
  interrupts();

  Serial.print("x: ");
  Serial.print(data.x);
  Serial.print("  y: ");
  Serial.print(data.y);
  Serial.print("  brakes: ");
  Serial.println(data.brakes);

  controlDrive(data.x, data.y, data.brakes);
}
}
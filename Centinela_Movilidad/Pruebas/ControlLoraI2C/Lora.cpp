#include <Wire.h>
#include "ControlLora.h"

volatile ControlData rxData = {0, 0, 0};
volatile bool newData = false;

void onReceive(int numBytes) {
  if (numBytes == sizeof(ControlData)) {
    uint8_t *ptr = (uint8_t*)&rxData;
    for (int i = 0; i < sizeof(ControlData); i++) {
      if (Wire.available()) {
        ptr[i] = Wire.read();
      }
    }
    newData = true;
  } else {
    while (Wire.available()) Wire.read(); // limpiar buffer
  }
}

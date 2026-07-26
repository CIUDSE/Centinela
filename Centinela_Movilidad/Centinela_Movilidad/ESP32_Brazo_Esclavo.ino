#include <Wire.h>
#include "pines_brazo.h"

#define I2C_SLAVE_ADDR_BRAZO 0x08

struct BrazoCmd {
  uint16_t ch1;
  uint16_t ch2;
  uint16_t ch4;
  uint8_t  velocidad;
};

volatile BrazoCmd ultimoComando;
volatile bool nuevoComando = false;

void onReceiveI2C(int numBytes) {
  if (numBytes == sizeof(BrazoCmd)) {
    Wire.readBytes((uint8_t*)&ultimoComando, sizeof(BrazoCmd));
    nuevoComando = true;
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SLAVE_ADDR_BRAZO); // modo esclavo
  Wire.onReceive(onReceiveI2C);

  // Inicializar pines PWM/DIR de los 3 MDD10A aqui


}

void loop() {
  if (nuevoComando) {
    nuevoComando = false;
    float factor = ultimoComando.velocidad ? 1.0 : 0.3;

    // Mapear ch1, ch2, ch4 (rango tipico 1000-2000us) a velocidad de cada driver
    // Ejemplo: mapear a -255..255 y aplicar el factor
    int velMotor1 = (map(ultimoComando.ch1, 1000, 2000, -255, 255)) * factor;
    int velMotor2 = (map(ultimoComando.ch2, 1000, 2000, -255, 255)) * factor;
    int velMotor3 = (map(ultimoComando.ch4, 1000, 2000, -255, 255)) * factor;

    // controlarMDD10A_1(velMotor1);
    // controlarMDD10A_2(velMotor2);
    // controlarMDD10A_3(velMotor3);
  }
}
/*
*****************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
******************************
*/

#define Pines_Telemetria
#include "Telemetria_Control.h"

void compartirDatosI2C()
{
  Wire.beginTransmission(I2C_SLAVE_ADDR); // ESP32 rover
  Wire.write((uint8_t*)&telemetryControl, sizeof(telemetryControl));
  Wire.endTransmission();
}

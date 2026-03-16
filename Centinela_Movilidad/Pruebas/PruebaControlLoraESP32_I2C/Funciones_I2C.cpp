/****************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers

Archivo cpp con la declaración de funciones de I2C para el control de la PPM implementando ESP32.
****************************************************************************************************************************************/

#define Pines_Control
#include "Telemetria_Control.h"

void recibirDatosI2C(int numBytes)
{
  if(numBytes == sizeof(telemetryControl))
  {
    Wire.readBytes((uint8_t*)&telemetryControl, sizeof(telemetryControl));
  }
}
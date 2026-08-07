/***************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo cpp con la declaración de funciones del GPS para el módulo de telemetría.

Este código es para el GPS2 (GPS externo Neo6m).
****************************************************************************************************************************************/
#include "Telemetria_Emisor.h"

TinyGPSPlus gps2;
HardwareSerial SerialGPS2(2);

void inicializarGPS2()
{
  SerialGPS2.begin(GPS2_BAUD, SERIAL_8N1, GPS2_RX_PIN, GPS2_TX_PIN);
}

void leerGPS2()
{
  while (SerialGPS2.available()) 
  {
    gps2.encode(SerialGPS2.read());
  }

  //GPS2 datos
  if (gps2.location.isUpdated()) 
  {
    sensorData.lat2 = gps2.location.lat();
    sensorData.lon2 = gps2.location.lng();
  }

  //Valores convertidos a entero para enviar en la telemetría
  telemetryData.lat2_32 = (int32_t)(sensorData.lat2 * 1e6);
  telemetryData.lon2_32 = (int32_t)(sensorData.lon2 * 1e6); 
}

void asegurarGPS2()
{
  while (!gps2.location.isValid())
  {
    while (SerialGPS2.available())
    {
      gps2.encode(SerialGPS2.read());
    }
  }
}
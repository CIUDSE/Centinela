/***************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo cpp con la declaración de funciones del GPS para el módulo de telemetría TRANSMISOR/EMISOR implementando LilyGo TTGO T-Beam V1.2

Este código es para el GPS2 (GPS externo Neo6m).
****************************************************************************************************************************************/

#define Pines_Telemetria
#include "Telemetria_Emisor.h"

TinyGPSPlus gps2;
HardwareSerial SerialGPS2(2);

void inicializarGPS2()
{
  SerialGPS2.begin(GPS2_BAUD, SERIAL_8N1, GPS2_RX_PIN, GPS2_TX_PIN);
}

void leerGPS2()
{
  while (SerialGPS2.available()) gps2.encode(SerialGPS2.read());

  if (gps2.location.isUpdated()) 
  {
    telemetryData.lat2 = gps2.location.lat();
    telemetryData.lon2 = gps2.location.lng();
  }
}
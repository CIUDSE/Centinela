/***************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo cpp con la declaración de funciones del GPS para el módulo de telemetría TRANSMISOR/EMISOR implementando LilyGo TTGO T-Beam V1.2.

Este código es para el GPS1 (GPS integrado en placa T-Beam).
****************************************************************************************************************************************/

#define Pines_Telemetria
#include "Telemetria_Emisor.h"

TinyGPSPlus gps1;
HardwareSerial SerialGPS1(1);

XPowersAXP2101 *PMU = nullptr;

void setupPMU()
{
  PMU = new XPowersAXP2101(Wire);

  if (!PMU->init()) {
    
    delete PMU;
    PMU = nullptr;
    return;
  }

  // Alimentar GPS1 desde ALDO3
  PMU->setALDO3Voltage(3300);
  PMU->enableALDO3();
}

void inicializarGPS1()
{
  SerialGPS1.begin(GPS1_BAUD, SERIAL_8N1, GPS1_RX_PIN, GPS1_TX_PIN);
}

void leerGPS1()
{
  while (SerialGPS1.available()) gps1.encode(SerialGPS1.read());

  if (gps1.location.isUpdated()) 
  {
    telemetryData.lat1 = gps1.location.lat();
    telemetryData.lon1 = gps1.location.lng();
  }
}
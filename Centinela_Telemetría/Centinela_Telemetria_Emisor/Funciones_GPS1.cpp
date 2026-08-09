/***************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo cpp con la declaración de funciones del GPS para el módulo de telemetría.

Este código es para el GPS1 (GPS integrado en placa T-Beam).
****************************************************************************************************************************************/
#include "Telemetria_Emisor.h"

TinyGPSPlus gps1;
HardwareSerial SerialGPS1(1);

XPowersAXP2101 *PMU = nullptr;

void setupPMU()
{
  PMU = new XPowersAXP2101(Wire);

  if (!PMU->init()) {
    
    Serial.println(" No se encontró PMU AXP2101");

    delete PMU;
    PMU = nullptr;
    return;
  }

  // Alimentar GPS1 desde ALDO3
  PMU->setALDO3Voltage(3300);
  PMU->enableALDO3();

  Serial.print("ALDO3 State: ");
  Serial.println(PMU->isEnableALDO3() ? "ON" : "OFF");

  Serial.print("ALDO3 Voltage: ");
  Serial.println(PMU->getALDO3Voltage()); 

  Serial.println(" GPS alimentado desde ALDO3 (3.3V)");
}

void inicializarGPS1()
{
  setupPMU();
  SerialGPS1.begin(GPS1_BAUD, SERIAL_8N1, GPS1_RX_PIN, GPS1_TX_PIN);
}

void leerGPS1()
{    
  while (SerialGPS1.available()) 
  {
    gps1.encode(SerialGPS1.read());
  }

  //GPS1 datos
  if (gps1.location.isUpdated()) 
  {
    sensorData.lat1 = gps1.location.lat();
    sensorData.lon1 = gps1.location.lng();
  }

  //Valores convertidos a entero para enviar en la telemetría
  telemetryData.lat1_32 = (int32_t)(sensorData.lat1 * 1e6);
  telemetryData.lon1_32 = (int32_t)(sensorData.lon1 * 1e6); 
}

void asegurarGPS1()
{
  while (!gps1.location.isValid())
  {
    Serial.println("Location valid");
    while (SerialGPS1.available())
    {
      Serial.println("gps available");
      gps1.encode(SerialGPS1.read());
    }
  }
}
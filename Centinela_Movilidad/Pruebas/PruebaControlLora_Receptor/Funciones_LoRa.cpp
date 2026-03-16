/***************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo cpp con la declaración de funciones de LORA para el control implementando LilyGo TTGO T-Beam V1.2.
****************************************************************************************************************************************/

#define Pines_Telemetria
#include "Telemetria_Control.h"

//Función para inicilizar el módulo LORA esp32
void inicializarLora()
{
   // Iniciar LoRa
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  // Chequeo rapido
  if (!LoRa.begin(BAND)) 
  {
    if(Serial)
      Serial.println("Fallo al iniciar LoRa");
    while (1);
  }

  // Asignar palabra de sincronización (debe ser la misma en TX y RX)
  LoRa.setSyncWord(0xA0); 

  // ESTAS LÍNEAS SON IMPORTANTES PARA LA LATENCIA 
  LoRa.setSpreadingFactor(7);     // Mínimo SF para máx velocidad
  LoRa.setSignalBandwidth(500E3); // Máximo BW para máx velocidad
}

bool recibirDatos()
{
  int packetSize = LoRa.parsePacket();

  if (packetSize == sizeof(telemetryControl)) 
  {
    uint8_t buffer[sizeof(telemetryControl)];

    LoRa.readBytes(buffer, sizeof(buffer));

    memcpy(&telemetryControl,buffer,sizeof(buffer));     //Copiar los datos guardados al struct telemetryControl de los datos.

    return true;
  }
  
  return false;
}
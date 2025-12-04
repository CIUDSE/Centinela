/*
*****************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo cpp con la declaración de funciones de LORA para el módulo de telemetría TRANSMISOR/EMISOR implementando LilyGo TTGO T-Beam V1.2
******************************
*/

#define Pines_Telemetria
#include "Telemetria_Emisor.h"

//Función para inicilizar el módulo LORA esp32
void inicializarLora()
{
   // Iniciar LoRa
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  // Chequeo rapido
  if (!LoRa.begin(BAND)) 
  {
    #ifdef BUZZER
      tonoBuzzerError();
    #endif
    
    if(Serial)
      Serial.println("Fallo al iniciar LoRa");
    while (1);
  }


  // Asignar palabra de sincronización (debe ser la misma en TX y RX)
  LoRa.setSyncWord(0xA0); 

  // ESTAS LÍNEAS SON IMPORTANTES PARA LA LATENCIA 
  LoRa.setSpreadingFactor(7);     // Mínimo SF para máx velocidad
  LoRa.setSignalBandwidth(500E3); // Máximo BW para máx velocidad
  
  sendMessage("LoRa inicializado correctamente"); delay(time_delay); 
  #ifdef BUZZER
    tonoBuzzerCorrecto();
  #endif
}

void sendMessage(String message)
{
  LoRa.beginPacket();     // 1. Iniciar el paquete de LoRa
  LoRa.print(message);    // 2. Escribir el contenido del mensaje
  LoRa.endPacket();       // 3. Finalizar el paquete y enviarlo
}
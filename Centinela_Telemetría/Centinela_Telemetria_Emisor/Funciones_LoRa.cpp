/***************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo cpp con la declaración de funciones de LORA para el módulo de telemetría.
****************************************************************************************************************************************/
#include "Telemetria_Emisor.h"

unsigned long tiempoSegundo = 0;            // Variable usada en archivo .ino
unsigned long tiempoRespuesta = 0;          // Variable usada en archivo .ino 
int contadorPaquetes = 0;                   // Variable usada en archivo .ino
volatile bool loraEnviando = false;
unsigned long tiempoInicioEnvio = 0;


// ─── Callback: se dispara por DIO0/G0 cuando termina la transmisión ──────────
void onLoraTxDone()
{
  loraEnviando = false;
}


//Función para inicilizar el módulo LORA esp32
void inicializarLora()
{
  // Iniciar LoRa
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(BAND)) 
  {
    if(Serial)
      Serial.println("Fallo al iniciar LoRa");
    while (1);
  }

  LoRa.setSyncWord(SYNC_WORD);
  LoRa.setSpreadingFactor(SPREADING_FACTOR);      //SF modificar cuando se use la antena posiblmentente a 10
  LoRa.setSignalBandwidth(BANDWIDTH);  // SB modificar cuando la antena posiblemente a 125
  LoRa.setCodingRate4(CODING_RATE);

  //Registrar callback AL FINAL
  LoRa.onTxDone(onLoraTxDone);

  if (Serial)
    Serial.println("LoRa iniciado correctamente");
}

void enviarDatos()
{
  /*
  //LoRa normal
  uint8_t buffer[sizeof(telemetryData)];
  memcpy(buffer, &telemetryData, sizeof(telemetryData)); 

  LoRa.beginPacket();                    // 1. Iniciar el paquete de LoRa
  LoRa.write(buffer, sizeof(buffer));    // 2. Escribir el contenido del mensaje
  LoRa.endPacket();                      // 3. Finalizar el paquete y enviarlo
  */

  //LoRa asíncrono
  // Si aún está transmitiendo, no interrumpir
  if (loraEnviando) return;

  loraEnviando = true;

  LoRa.beginPacket();
  LoRa.write((uint8_t*)&telemetryData, sizeof(telemetryData_t));
  LoRa.endPacket(true);   // true = asíncrono, NO bloqueante

  tiempoInicioEnvio   = millis();
}


void timeoutLora()
{
  // Timeout de seguridad — libera loraEnviando si el callback no se disparó
  if (loraEnviando && millis() - tiempoInicioEnvio > 1300)
  {
    loraEnviando = false;  // Forzar liberación
  }
}
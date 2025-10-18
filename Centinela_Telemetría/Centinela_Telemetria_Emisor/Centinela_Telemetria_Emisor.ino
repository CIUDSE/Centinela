/*
*****************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
//Código EMISOR para telemetría general del rover Centinela implementando una LilyGo TTGO T-Beam V1.2, Neo6m integrado, Gy-87, DS18B20.
******************************
*/

//Protocolos de comunicación
#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>

#define TBEAM_GY87
#include "Declaracion_Pines_Telemetria.h"

#define BAUD_RATE 115200

void setup() 
{
  Serial.begin(115200);
  delay(3000);
  inicializarLora();
}

unsigned int counter = 0;
void loop() 
{
  Serial.println(String(counter));

  //enviar paquete
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;
  delay(1000);
}

void inicializarLora()
{
   // Iniciar LoRa
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  // Chequeo rapido
  if (!LoRa.begin(BAND)) 
  {
    Serial.println("Fallo al iniciar LoRa");
    while (1);
  }
  Serial.println("LoRa iniciado");
  delay(1500);  
}
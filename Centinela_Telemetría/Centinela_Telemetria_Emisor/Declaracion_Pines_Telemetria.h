/*
*****************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo header con la declaración de pines y configuración inicial para el módulo de telemetría TRANSMISOR/EMISOR implementando LilyGo TTGO T-Beam V1.2
******************************
*/

//Protocolos de comunicación
#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>

#ifndef TELEMETRIA_H
#define TELEMETRIA_H

//Declaracion de pines y configuración inicial para la telemetría usando la placa LilyGo TTGO T-Beam V1.2.
#ifdef TBEAM_GY87
  //Baud rate de esp32
  #define BAUD_RATE 115200

  //Banda LORA
  #define BAND 915E6

  //Pines para LORA de la placa T-Beam
  #define LORA_SCK 5      //Reloj SPI
  #define LORA_MISO 19    //MISO
  #define LORA_MOSI 27    //MOSI
  #define LORA_SS 18      //Slave Select
  #define LORA_RST 23     //Reset
  #define LORA_DIO0 26    //IRQ (Interrupt Request)
#endif

void inicializarLora();
void sendMessage(String message);

#endif
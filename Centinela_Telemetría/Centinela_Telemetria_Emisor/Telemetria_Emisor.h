/*
*****************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo header con la declaración de pines y configuración inicial para el módulo de telemetría TRANSMISOR/EMISOR implementando 
LilyGo TTGO T-Beam V1.2, Módulo GY87, GPS Neo6m integrado y externo, sensores de temperatura DS18B20.
******************************
*/

//Protocolos de comunicación
#include <cstdint>
#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>

#ifndef TELEMETRIA_H
#define TELEMETRIA_H

//------------------------------------------------------------------------------//
//Declaracion de pines y configuración inicial para la telemetría usando la placa LilyGo TTGO T-Beam V1.2.
#ifdef Pines_Telemetria
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

//Variables globales
extern int time_delay;

extern int16_t RAW_accelX, RAW_accelY, RAW_accelZ;
extern float accelX, accelY, accelZ;

extern int16_t gyroX, gyroY, gyroZ;
extern float rotX, rotY, rotZ;

//------------------------------------------------------------------------------//

//Funciones Globales
String crearMensaje();

//Funciones Lora
void inicializarLora();
void sendMessage(String message);

//Funciones GY87
void inicializarGY87();
void leerAceleracion();
void leerGiroscopio();
void processGyroData();
void processAccelData();

//------------------------------------------------------------------------------//

#endif
/******************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo header con la declaración de pines y configuración inicial para el módulo de telemetría TRANSMISOR/EMISOR implementando 
LilyGo TTGO T-Beam V1.2, Módulo GY87, GPS Neo6m integrado y externo, sensores de temperatura DS18B20.
*******************************************************************************************************************************/

//Librerias
#include <cstdint>
#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>
#include <OneWire.h>                
#include <DallasTemperature.h>

#ifndef TELEMETRIA_H
#define TELEMETRIA_H

//------------------------------------------------------------------------------//
//Declaracion de pines y configuración inicial para la telemetría usando la placa LilyGo TTGO T-Beam V1.2.
#ifdef Pines_Telemetria
  //Baud rate de esp32
  #define BAUD_RATE 115200

  //Pin de buzzer
  #define PIN_BUZZER 32
  #ifndef BUZZER
  #define BUZZER
  #endif

  //Pin DS18B20 (Sensores de temperatura)
  extern OneWire ourWire;

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

//Constantes
#define DS18B20_RESOLUTION 9

//Variables globales
extern int time_delay;

extern int16_t RAW_accelX, RAW_accelY, RAW_accelZ;
extern float accelX, accelY, accelZ;

extern int16_t RAW_gyroX, RAW_gyroY, RAW_gyroZ;
extern float rotX, rotY, rotZ;

extern float temp1;
extern float temp2;
extern float temp3;
extern float temp4;
extern float temp5;
extern float temp6;
extern float temp7;
extern float temp8;
extern float temp9;
extern float temp10;

//Objetos
extern DallasTemperature sensors; //Declaración de sensores de temperatura

//------------------------------------------------------------------------------//

//Funciones Globales
String crearMensaje();

void tonoBuzzerActivacion();
void tonoBuzzerCorrecto();
void tonoBuzzerError();

//Funciones Lora
void inicializarLora();
void sendMessage(String message);

//Funciones GY87
void inicializarGY87();
void leerAceleracion();
void leerGiroscopio();
void processGyroData();
void processAccelData();

//Funciones GPS integrado en la placa T-Beam
void inicializarGPS_TBEAM();
void leerGPS_TBEAM();

//Funciones GPS1
void inicializarGPS1();
void leerGPS1();

//Funciones DS18B20
void inicializarDS18B20();
void leerDS18B20();

//------------------------------------------------------------------------------//

#endif
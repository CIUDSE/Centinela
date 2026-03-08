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
#include <XPowersLib.h>
#include <OneWire.h>                
#include <DallasTemperature.h>

#ifndef TELEMETRIA_H
#define TELEMETRIA_H

//------------------------------------------------------------------------------//
//Declaracion de pines y configuración inicial para la telemetría usando la placa LilyGo TTGO T-Beam V1.2.
#ifdef Pines_Telemetria
  //Baud rate de esp32
  #define BAUD_RATE 115200

  //Pines para LORA de la placa T-Beam
  #define LORA_SCK 5      //Reloj SPI
  #define LORA_MISO 19    //MISO
  #define LORA_MOSI 27    //MOSI
  #define LORA_SS 18      //Slave Select
  #define LORA_RST 23     //Reset
  #define LORA_DIO0 26    //IRQ (Interrupt Request)
  
  //Banda LORA
  #define BAND 915E6

  //Pines I2C
  #define I2C_SDA 21
  #define I2C_SCL 22
  #define I2C_FREQ 400000

  //Dirección GY87
  #define GY87_ADDRESS 0x68

  //Pines GPS 1 (GPS integrado en placa T-Beam)
  #define GPS1_RX_PIN 34
  #define GPS1_TX_PIN 12
  #define GPS1_BAUD   9600

  //Pines GPS 2 (GPS externo Neo6m)
  #define GPS2_RX_PIN 35
  #define GPS2_TX_PIN 33
  #define GPS2_BAUD   9600

  //Pin de buzzer
  //#define PIN_BUZZER 32
  //#ifndef BUZZER
  //#define BUZZER
  //#endif

  //Pin DS18B20 (Sensores de temperatura)
  extern OneWire ourWire;

#endif

//Constantes
#define DS18B20_RESOLUTION 9
#define DS18B20_CANTIDAD 10

//Variables globales
typedef struct telemetryData 
{
  //Datos GY87
  float accelX = 0.0;       //Valores en m/s^2
  float accelY = 0.0;
  float accelZ = 0.0;
  float rotX = 0.0;         //Valores en °/s
  float rotY = 0.0;
  float rotZ = 0.0;
  
  //Datos GPS1 (GPS integrado en placa T-Beam)
  double lat1 = -1.0;
  double lon1 = -1.0;

  //Datos GPS2 (GPS externo Neo6m)
  double lat2 = -1.0;
  double lon2 = -1.0;

  //Datos DS18B20 (Sensores de temperatura)
  float temp[DS18B20_CANTIDAD];

}telemetryData_t;

extern telemetryData_t telemetryData;   //Crear una variable de la estructura

extern XPowersAXP2101 *PMU;   

//Objetos
extern DallasTemperature sensors; //Declaración de sensores de temperatura
extern TinyGPSPlus gps1;
extern TinyGPSPlus gps2;

//------------------------------------------------------------------------------//
//Funciones Globales

/*
void tonoBuzzerActivacion();
void tonoBuzzerCorrecto();
void tonoBuzzerError();
*/

//Funciones Lora
void inicializarLora();
void enviarDatos();

//Funciones GY87
void inicializarGY87();
void leerAcelerometro();
void leerGiroscopio();
void processGyroData();
void processAccelData();

//Funciones GPS1 (GPS integrado en placa T-Beam)
void inicializarGPS1();
void leerGPS1();
void setupPMU();

//Funciones GPS2 (GPS externo Neo6m)
void inicializarGPS2();
void leerGPS2();

//Funciones DS18B20
void inicializarDS18B20();
void leerDS18B20();

//------------------------------------------------------------------------------//

#endif
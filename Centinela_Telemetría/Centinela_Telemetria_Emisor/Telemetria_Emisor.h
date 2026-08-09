/******************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo header con la declaración de pines y configuración inicial para el módulo de telemetría TRANSMISOR/EMISOR implementando 
LilyGo TTGO T-Beam V1.2, Módulo GY87, GPS Neo6m integrado y externo, sensores de temperatura DS18B20.
*******************************************************************************************************************************/
#pragma once

//Librerias
#include <cstdint>
#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>
#include <XPowersLib.h>
#include <OneWire.h>                
#include <DallasTemperature.h>

//--------------------------- Pines del LilyGo TTGO T-Beam ---------------------------//
//Baud rate de esp32
#define BAUD_RATE 115200

//LORA de la placa T-Beam
#define LORA_SCK 5      //Reloj SPI
#define LORA_MISO 19    //MISO
#define LORA_MOSI 27    //MOSI
#define LORA_SS 18      //Slave Select
#define LORA_RST 23     //Reset
#define LORA_DIO0 26    //IRQ (Interrupt Request)

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
#define PIN_BUZZER 32

//--------------------------- Objetos ---------------------------//
//DS18B20
extern OneWire ourWire;
extern DallasTemperature sensors;

//GPS1
extern XPowersAXP2101 *PMU;   
extern TinyGPSPlus gps1;

//GPS2
extern TinyGPSPlus gps2;

//--------------------------- Variables y constantes en el código ---------------------------//
// NOTA: Estas son las declaraciones de que se usarán estas variables y constantes en el código, pero cada una esta definida en su respectivo archivo
// de función. Aquí solo se declaran, pero en cada archivo se le da su respectivo valor. Las variables aqui son tipo "extern" por ese motivo.

#define ID '1'    //ID

//LoRa
#define BAND 915E6
#define SYNC_WORD 0xA0
#define SPREADING_FACTOR 7
#define BANDWIDTH 500E3
#define CODING_RATE 5
extern unsigned long tiempoSegundo;
extern unsigned long tiempoRespuesta;
extern int contadorPaquetes;
extern volatile bool loraEnviando;
extern unsigned long tiempoInicioEnvio;

//DS18B20
#define DS18B20_RESOLUTION 9
#define DS18B20_CANTIDAD 10

//--------------------------- Estructura de datos de sensores ---------------------------//
typedef struct sensorData 
{
  //Datos GY87
  float accel_x = 0.0;       //Valores en m/s^2
  float accel_y = 0.0;
  float accel_z = 0.0;
  float vel_ang_x = 0.0;         //Valores en °/s
  float vel_ang_y = 0.0;
  float vel_ang_z = 0.0;

  //Datos GPS1 (GPS integrado en placa T-Beam)
  double lat1 = 0.0;
  double lon1 = 0.0;

  //GPS2 (GPS externo Neo6m)
  double lat2 = 0.0;
  double lon2 = 0.0;

  //DS18B20 (Sensores de temperatura)
  float temp[DS18B20_CANTIDAD];
} sensorData_t;

extern sensorData_t sensorData;

//--------------------------- Estructura de datos de telemetría ---------------------------//
typedef struct __attribute__((packed)) 
{
  char id;
  uint16_t tiempoRecibido;
  uint16_t numPaquete;
  
  //GY87
  int16_t accel_x_16 = 0;       //Valores en m/s^2
  int16_t accel_y_16 = 0;
  int16_t accel_z_16 = 0;
  int32_t vel_ang_x_32 = 0;         //Valores en °/s
  int32_t vel_ang_y_32 = 0;
  int32_t vel_ang_z_32 = 0;
  
  //GPS1 (GPS integrado en placa T-Beam)
  int32_t lat1_32 = 0;
  int32_t lon1_32 = 0;

  //GPS2 (GPS externo Neo6m)
  int32_t lat2_32 = 0;
  int32_t lon2_32 = 0;

  //DS18B20 (Sensores de temperatura)
  int16_t temp_16[DS18B20_CANTIDAD];
} telemetryData_t;

extern telemetryData_t telemetryData;

//--------------------------- Funciones ---------------------------//
//Funciones LoRa
void inicializarLora();
void enviarDatos();
void actualizarLora();
void timeoutLora();

//Funciones GY87
void inicializarGY87();
void calibrarGY87(); //Calibración de giroscopio y acelerómetro
void leerAcelerometro();
void leerGiroscopio();
void processGyroData();
void processAccelData();

//Funciones GPS1 (GPS integrado en placa T-Beam)
void inicializarGPS1();
void leerGPS1();
void setupPMU();
void asegurarGPS1();

//Funciones GPS2 (GPS externo Neo6m)
void inicializarGPS2();
void leerGPS2();
void asegurarGPS2();

//Funciones DS18B20
void inicializarDS18B20();
void leerDS18B20();


//Funciones Globales
void tonoBuzzerActivacion();
//void tonoBuzzerCorrecto();
//void tonoBuzzerError();



/******************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo header con la declaración de pines y configuración inicial para prueba del control del rover implementando LilyGo TTGO T-Beam V1.2.
*******************************************************************************************************************************/

//Librerias
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

  #define I2C_SLAVE_ADDR 0x08   //Dirección I2C para la esp32
#endif

//Variables globales
typedef struct telemetryControl 
{
  int16_t x;
  int16_t y;
  int8_t brake;
}telemetryControl_t;

extern telemetryControl_t telemetryControl;   //Crear una variable de la estructura

//------------------------------------------------------------------------------//
//Funciones Lora
void inicializarLora();
bool recibirDatos();

//Funciones I2C
void compartirDatosI2C();

//------------------------------------------------------------------------------//

#endif
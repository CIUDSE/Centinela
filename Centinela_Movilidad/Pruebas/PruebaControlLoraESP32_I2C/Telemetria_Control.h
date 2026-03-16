/******************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo header con la declaración de pines y configuración inicial para pruebas del control del rover implementando ESP32.
*******************************************************************************************************************************/

//Librerias
#include <cstdint>
#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <driver/ledc.h>  //Permite usar PWM en esp32

#ifndef TELEMETRIA_H
#define TELEMETRIA_H

//------------------------------------------------------------------------------//
//Declaracion de pines y configuración inicial para el control usando la placa ESP32
#ifdef Pines_Control
  //Baud rate de esp32
  #define BAUD_RATE 115200

  //Pines I2C
  #define I2C_SDA 21
  #define I2C_SCL 22
  #define I2C_FREQ 400000

  #define I2C_SLAVE_ADDR 0x08

  //Pines de control
  #define FRONT_RIGHT 27
  #define DIR_RIGHT 13

  #define FRONT_LEFT 19
  #define DIR_LEFT 23

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
//Funciones I2C
void recibirDatosI2C(int numBytes);

//------------------------------------------------------------------------------//

#endif
/***************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por 

Archivo de arduinoIDE para la esp32 para las pruebas del control de los motores de hoverboard con el driver riorand.

El objetivo de este código es comprobar el funcionamiento de la recepción de datos de control por I2C desde la placa LilyGo a la esp32
para el control de la Plataforma de Pruebas Móviles (PPM).

Board en ArduinoIDE es ESP32 Dev Module
****************************************************************************************************************************************/

//Declaramos configuración de pines 
#define Pines_Control
#include "Telemetria_Control.h"

const int frequency = 4000;

const int rightChannel = 0;
const int leftChannel = 1;

const int resolution = 8;

const int velocidad = 120;

bool initialized0 = false;
bool initialized1 = false;

telemetryControl_t telemetryControl;

void setup() 
{
  
  //Wire.onReceive(recibirDatosI2C);  //Habilita la recepción de datos en la función recibirDatosI2C

  if (ledcAttachChannel(FRONT_RIGHT, frequency, resolution, rightChannel) == false)   // Declaracion del pin PWM que se utilizará y comprobación que se inicialice correctamente
  {
    initialized0 = false;
  }

  initialized0 = true;

  if (ledcAttachChannel(FRONT_LEFT, frequency, resolution, leftChannel) == false)   // Declaracion del pin PWM que se utilizará y comprobación que se inicialice correctamente
  {
    initialized1 = false;
  }

  initialized1 = true;
}

void loop() 
{
  ledcWrite(FRONT_RIGHT, velocidad);
  ledcWrite(FRONT_LEFT, velocidad);

  delay(10);
}

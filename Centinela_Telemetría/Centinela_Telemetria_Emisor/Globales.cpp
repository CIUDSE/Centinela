/*
*****************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo cpp con la declaración de variables y funciones GLOBALES para el módulo de telemetría TRANSMISOR/EMISOR implementando LilyGo TTGO T-Beam V1.2.
En este archivo se encuentran las variables globales, además de las funciones generales que no aplican al resto de archivos de funciones.
******************************
*/

#define Pines_Telemetria
#include "Telemetria_Emisor.h"

//Variables Globales
int16_t RAW_accelX, RAW_accelY, RAW_accelZ;   //Valores RAW
float accelX, accelY, accelZ;                 //Valores en m/s^2

int16_t RAW_gyroX, RAW_gyroY, RAW_gyroZ;      //Valores RAW
float rotX, rotY, rotZ;                       //Valores en °/s

int time_delay = 1000;

String msg = "";
//------------------------------------------------------------------------------//
//Funciones Globales
String crearMensaje()
{
  msg = String(accelX);
  msg += ",";
  msg += String(accelY);
  msg += ",";
  msg += String(accelZ);
  msg += ",";
  msg += String(rotX);
  msg += ",";
  msg += String(rotY);
  msg += ",";
  msg += String(rotZ);
  msg += ",";

  return msg;
}
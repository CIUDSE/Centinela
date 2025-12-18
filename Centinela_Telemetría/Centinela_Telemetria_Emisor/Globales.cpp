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

float temp1;
float temp2;
float temp3;
float temp4;
float temp5;
float temp6;
float temp7;
float temp8;
float temp9;
float temp10;

float gps1_lat = -1.0;
float gps1_lon = -1.0;

float gps2_lat = -1.0;
float gps2_lon = -1.0;

extern float gps2_lat;
extern float gps2_lon;

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
  msg += String(temp1);
  msg += ",";
  msg += String(temp2);
  msg += ",";
  msg += String(gps1_lat);
  msg += ",";
  msg += String(gps1_lon);
  msg += ",";
  msg += String(gps2_lat);
  msg += ",";
  msg += String(gps2_lon);
  msg += ",";

  return msg;
}

void tonoBuzzerActivacion()
{
  // 3 Beeps.  Largo - Corto - Corto
  digitalWrite(PIN_BUZZER, HIGH);
  delay(800);
  digitalWrite(PIN_BUZZER, LOW);
  delay(100);
  digitalWrite(PIN_BUZZER, HIGH);
  delay(100);
  digitalWrite(PIN_BUZZER, LOW);
  delay(100);
  digitalWrite(PIN_BUZZER, HIGH);
  delay(100);
  digitalWrite(PIN_BUZZER, LOW);
  delay(100);
}

void tonoBuzzerCorrecto()
{
  // 3 Beeps.  Corto - Corto - Corto
  digitalWrite(PIN_BUZZER, HIGH);
  delay(100);
  digitalWrite(PIN_BUZZER, LOW);
  delay(100);
  digitalWrite(PIN_BUZZER, HIGH);
  delay(100);
  digitalWrite(PIN_BUZZER, LOW);
  delay(100);
  digitalWrite(PIN_BUZZER, HIGH);
  delay(100);
  digitalWrite(PIN_BUZZER, LOW);
  delay(100);
}

void tonoBuzzerError()
{
  digitalWrite(PIN_BUZZER, HIGH);
  delay(800);
  digitalWrite(PIN_BUZZER, LOW);
  delay(200);
  digitalWrite(PIN_BUZZER, HIGH);
  delay(800);
  digitalWrite(PIN_BUZZER, LOW);
  delay(100);

}
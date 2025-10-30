/*
*****************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Código TRANSMISOR/EMISOR para telemetría general del rover Centinela implementando una LilyGo TTGO T-Beam V1.2, Neo6m integrado, MPU 6050, DS18B20.

Para la prueba del envio de datos se utilizo este código en la placa lilygo ttgo t-beam como transmisor, mientras que se uso la lilygo con pantalla
como receptor utilizando el codigo "Receptor_2" en la carpeta de pruebas.

Librerias necesarias:
LoRa by Sandeep Mistry
******************************
*/

//#define SERIAL_MONITOR

//Protocolos de comunicación
#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>

//Declaramos configuración de pines 
#define Pines_Telemetria
#include "Telemetria_Emisor.h"
//Banda Lora actual 915E6 ----- Se puede modificar en archivo .h

String mensaje = "";

void setup() 
{
  inicializarLora();
  inicializarGY87();  //Inicializa I2C automaticamente
}

void loop() 
{
  leerAceleracion();
  leerGiroscopio();
  
  mensaje = crearMensaje();

  sendMessage(mensaje);   //Enviar mensaje
  
  delay(500); // Espera de 1 segundo antes del próximo envío (el tiempo es solo para prueba)
}
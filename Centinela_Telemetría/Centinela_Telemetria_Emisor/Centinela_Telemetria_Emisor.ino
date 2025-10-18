/*
*****************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Código TRANSMISOR/EMISOR para telemetría general del rover Centinela implementando una LilyGo TTGO T-Beam V1.2, Neo6m integrado, Gy-87, DS18B20.

Para la prueba del envio de datos se utilizo este codigo en la placa lilygo ttgo t-beam como transmisor, mientras que se uso la lilygo con pantalla
como receptor utilizando el codigo "Receptor_2" en la carpeta de pruebas.
******************************
*/

//Protocolos de comunicación
#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>

//Declaramos configuración de pines 
#define TBEAM_GY87
#include "Declaracion_Pines_Telemetria.h"

//Banda Lora actual 915E6 ----- Modificar en archivo .h

unsigned int contador = 0;  //Contador de prueba.
unsigned int mensaje = 0;

void setup() 
{
  //Inicializamos monitor serial
  Serial.begin(115200);
  while(!Serial);

  inicializarLora();
}

void loop() 
{
  // Texto en monitor serial
  Serial.print("Enviando paquete: ");
  Serial.println(contador);

  mensaje = contador;

  sendMessage(String(mensaje));   //Enviar mensaje

  contador++; // Incrementar el contador
  
  delay(1000); // Espera de 1 segundo antes del próximo envío (el tiempo es solo para prueba)
}
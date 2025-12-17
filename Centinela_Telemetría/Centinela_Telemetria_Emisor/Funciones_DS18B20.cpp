/*****************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo cpp con la declaración de funciones del sensores de temperatura DS18B20 para el módulo de telemetría TRANSMISOR/EMISOR 
implementando LilyGo TTGO T-Beam V1.2
*****************************************************************************************************************************/

#define Pines_Telemetria
#include "Telemetria_Emisor.h"

OneWire ourWire(4);                //Se establece el pin 4 como bus OneWire
DallasTemperature sensors(&ourWire); //Se declara una objeto de los sensores.

DeviceAddress address1 = {0x28, 0x61, 0x64, 0x35, 0xC6, 0x10, 0xBF, 0x30};//dirección del sensor 1
DeviceAddress address2 = {0x28, 0x61, 0x64, 0x35, 0xC1, 0x40, 0x7E, 0xA9};//dirección del sensor 2

void inicializarDS18B20()
{
  sensors.begin();   //Se inicia el sensor

  sensors.setResolution(address1, DS18B20_RESOLUTION);
  sensors.setResolution(address2, DS18B20_RESOLUTION);
}

void leerDS18B20()
{
  sensors.requestTemperatures();   //envía el comando para obtener las temperaturas
  temp1= sensors.getTempC(address1);//Se obtiene la temperatura en °C del sensor 1
  temp2= sensors.getTempC(address2);//Se obtiene la temperatura en °C del sensor 2
}
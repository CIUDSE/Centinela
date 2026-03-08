/***************************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Código TRANSMISOR/EMISOR para telemetría general del rover Centinela implementando una LilyGo TTGO T-Beam V1.2, Neo6m integrado, MPU 6050, DS18B20.

Para la prueba del envio de datos se utilizo este código en la placa lilygo ttgo t-beam como transmisor, mientras que se uso la lilygo con pantalla
como receptor utilizando el codigo "ReceptorStruct" en la carpeta de pruebas.

Librerias necesarias:
LoRa by Sandeep Mistry
OneWire by Jim Studt...
DallasTemperature
TinyGPS+

Estructura de envio de datos:
telemetryData -> {accelX, accelY, accelZ, rotX, rotY, rotZ, lat1, lon1, lat2, lon2, temp[10]}
**************************************************************************************************************************************************/

//Declaramos configuración de pines 
#define Pines_Telemetria
#include "Telemetria_Emisor.h"
//Banda Lora actual 915E6 ----- Se puede modificar en archivo .h

telemetryData_t telemetryData;

void setup() 
{
  //pinMode(PIN_BUZZER, OUTPUT); tonoBuzzerActivacion(); //Inicializa buzzer
  inicializarLora();  //Función para inicializar LoRa.

  Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQ);

  inicializarGY87();    //Inicializa I2C automaticamente.
  inicializarDS18B20(); //Inicializa los sensores de temperatura.
  inicializarGPS1();    //Inicializa GPS1 (GPS integrado en T-Beam)
  inicializarGPS2();     //Inicializa GPS2 (GPS Neo6m externo)
  
}

void loop() 
{
  leerAcelerometro();
  leerGiroscopio();
  leerDS18B20();
  leerGPS1();
  leerGPS2();
  enviarDatos();


  //recibir control
  //enviar control a esp32

  
  delay(500); // Espera de 1 segundo antes del próximo envío (el tiempo es solo para prueba)
}
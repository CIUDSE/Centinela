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

// Variable para control de tiempo no bloqueante (reemplaza al delay)
unsigned long ultimoEnvio = 0;
const unsigned long intervaloEnvio = 500; // Tiempo en ms (mismo tiempo de prueba original)

void setup() 
{
  //pinMode(PIN_BUZZER, OUTPUT); tonoBuzzerActivacion(); //Inicializa buzzer
  inicializarLora();  //Función para inicializar LoRa.

  Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQ);

  inicializarGY87();    //Inicializa I2C automaticamente.
  calibrarGY87();       //Calibración de la IMU al encender
  inicializarDS18B20(); //Inicializa los sensores de temperatura.
  inicializarGPS1();    //Inicializa GPS1 (GPS integrado en T-Beam)
  inicializarGPS2();     //Inicializa GPS2 (GPS Neo6m externo)
  
}

void loop() 
{
  // Lectura continua de la IMU para alimentar el filtro complementario sin pausas
  leerAcelerometro();
  leerGiroscopio();

  // Lectura continua de buffers GPS
  leerGPS1();
  leerGPS2();

  // Envío por LoRa y sensores lentos cada 500 ms (mismo intervalo original, sin bloquear el flujo)
  if (millis() - ultimoEnvio >= intervaloEnvio) 
  {
    ultimoEnvio = millis();

    leerDS18B20();
    enviarDatos();
  }

  //recibir control
  //enviar control a esp32
}
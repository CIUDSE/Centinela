/***************************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Código TRANSMISOR/EMISOR para telemetría general del rover Centinela implementando una LilyGo TTGO T-Beam V1.2.

Estructura de envio de datos:
telemetryData -> {ID, Tiempo recibido, Número de paquete, accelX, accelY, accelZ, rotX, rotY, rotZ, lat1, lon1, lat2, lon2, temp[10]}
**************************************************************************************************************************************************/
#include "Telemetria_Emisor.h"

sensorData_t sensorData;            //Struct utilizado para datos de sensores en sus tipo de datos originales.
telemetryData_t telemetryData;      //Struct utilizado para envío de datos de telemetria. Aqui todos los datos son de tipo int.

void setup() 
{
  Serial.begin(115200);
  Serial.println("Inicio codigo");

  //Definir datos
  telemetryData.id = ID;
  pinMode(PIN_BUZZER, OUTPUT); tonoBuzzerActivacion();

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQ);

  inicializarLora();  //Función para inicializar LoRa.

  inicializarGY87();                      //Inicializa I2C automaticamente.
  //inicializarDS18B20();                   //Inicializa los sensores de temperatura.
  inicializarGPS1(); asegurarGPS1();        //Inicializa GPS1 (GPS integrado en T-Beam)
  //inicializarGPS2();       //Inicializa GPS2 (GPS Neo6m externo)
}

void loop() 
{
  //Datos de telemetría
  leerAcelerometro();
  leerGiroscopio();
  //leerDS18B20();
  leerGPS1();
  //leerGPS2();

  Serial.println("Enviando paquete");
  
  //Secuencia de envio de datos por LoRa
  timeoutLora();  // Timeout de seguridad — libera loraEnviando si el callback no se disparó

  if (!loraEnviando)           //Enviar los datos por LoRa
  {
    tiempoSegundo = millis();
    contadorPaquetes++;
    telemetryData.numPaquete = contadorPaquetes;
    telemetryData.tiempoRecibido = millis() - tiempoRespuesta;
    tiempoRespuesta = millis();

    enviarDatos();
  }
}
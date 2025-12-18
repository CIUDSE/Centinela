/***************************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Código TRANSMISOR/EMISOR para telemetría general del rover Centinela implementando una LilyGo TTGO T-Beam V1.2, Neo6m integrado, MPU 6050, DS18B20.

Para la prueba del envio de datos se utilizo este código en la placa lilygo ttgo t-beam como transmisor, mientras que se uso la lilygo con pantalla
como receptor utilizando el codigo "Receptor_2" en la carpeta de pruebas.

Librerias necesarias:
LoRa by Sandeep Mistry
OneWire
DallasTemperature
TinyGPS+
**************************************************************************************************************************************************/

//#define SERIAL_MONITOR

//Declaramos configuración de pines 
#define Pines_Telemetria
#include "Telemetria_Emisor.h"
//Banda Lora actual 915E6 ----- Se puede modificar en archivo .h

/* =====================================================
   CONFIGURACIÓN GPS
   ===================================================== */
// GPS 1 (UART1)
#define GPS1_RX_PIN 34
#define GPS1_TX_PIN 12
#define GPS1_BAUD   9600

// GPS 2 (UART2)
#define GPS2_RX_PIN 35
#define GPS2_TX_PIN 33
#define GPS2_BAUD   9600

/* =====================================================
   CONFIGURACIÓN I2C
   ===================================================== */
#define I2C_SDA 21
#define I2C_SCL 22


/* =====================================================
   OBJETOS
   ===================================================== */
TinyGPSPlus gps1;
TinyGPSPlus gps2;

HardwareSerial SerialGPS1(1);
HardwareSerial SerialGPS2(2);

XPowersAXP2101 *PMU = nullptr;

String mensaje = "";




void setup() 
{
  pinMode(PIN_BUZZER, OUTPUT); tonoBuzzerActivacion(); //Inicializa buzzer
  inicializarLora();  //Función para inicializar LoRa.

  Wire.begin(I2C_SDA, I2C_SCL, 400000);
  setupPMU();

  inicializarGY87();  //Inicializa I2C automaticamente.
  inicializarDS18B20(); //Inicializa los sensores de temperatura.

  
  SerialGPS1.begin(GPS1_BAUD, SERIAL_8N1, GPS1_RX_PIN, GPS1_TX_PIN);
  SerialGPS2.begin(GPS2_BAUD, SERIAL_8N1, GPS2_RX_PIN, GPS2_TX_PIN);

}

void loop() 
{
   /* ---------- GPS ---------- */
  while (SerialGPS1.available()) gps1.encode(SerialGPS1.read());
  while (SerialGPS2.available()) gps2.encode(SerialGPS2.read());

  /* ---------- OUTPUT ---------- */
  if (gps1.location.isUpdated()) 
  {
    
    gps1_lat = gps1.location.lat();
    gps1_lon = gps1.location.lng();
  }

  if (gps2.location.isUpdated()) 
  {
    gps2_lat = gps2.location.lat();
    gps2_lon = gps2.location.lng();
  }

  leerAceleracion();
  leerGiroscopio();
  leerDS18B20();

  mensaje = crearMensaje();

  sendMessage(mensaje);   //Enviar mensaje
  
  delay(500); // Espera de 1 segundo antes del próximo envío (el tiempo es solo para prueba)
}


void setupPMU()
{
  PMU = new XPowersAXP2101(Wire);

  if (!PMU->init()) {
    
    delete PMU;
    PMU = nullptr;
    return;
  }

  // Alimentar GPS1 desde ALDO3
  PMU->setALDO3Voltage(3300);
  PMU->enableALDO3();
}
/***************************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Código de prueba de receptor para datos de control del rover Centinela implementando una LilyGo TTGO T-Beam V1.2 y esp32 por I2C.

Para la prueba del envio de datos se utilizó la lilygo con pantalla como transmisor y este código en la placa lilygo ttgo t-beam como receptor.

El objetivo de este código es comprobar el funcionamiento del LoRa como sistema de control.

Librerias necesarias:
LoRa by Sandeep Mistry

Board en ArduinoIDE es Lilygo T-Display
**************************************************************************************************************************************************/

//Declaramos configuración de pines 
#define Pines_Telemetria
#include "Telemetria_Control.h"
//Banda Lora actual 915E6 ----- Se puede modificar en archivo .h

telemetryControl_t telemetryControl;

void setup() 
{
  Serial.begin(115200);

  inicializarLora();  //Función para inicializar LoRa.
  Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQ); 
}

void loop() 
{
  if(recibirDatos() == true)
  {
    compartirDatosI2C();
    Serial.print("X: ");
  Serial.print(telemetryControl.x );
  Serial.print(" | Y: ");
  Serial.println(telemetryControl.y);
  }
  
  delay(10); 
}
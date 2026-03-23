/***************************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers

Código de prueba de emisor con joystick para datos de control del rover Centinela implementando una Esp32 con Lora (TTGO Lora32) y un joystick.
Para la prueba del envio de datos se utilizó este código en la lilygo con pantalla. Como receptor se uso la T-Beam que envía los datos por I2C a la esp32.

El objetivo de este código es comprobar el funcionamiento del LoRa como sistema de control.

Librerias necesarias:
LoRa by Sandeep Mistry

Board en ArduinoIDE es TTGO LoRa32-OLED
**************************************************************************************************************************************************/

//Declaramos configuración de pines 
#define Pines_Telemetria
#include "Telemetria_Control.h"
//Banda Lora actual 915E6 ----- Se puede modificar en archivo .h

telemetryControl_t telemetryControl;

void setup() 
{
  inicializarLora();  //Función para inicializar LoRa.
   Serial.begin(115200);

  // Configure joystick pins
  pinMode(JOYSTICK_X, INPUT);
  pinMode(JOYSTICK_Y, INPUT);
  pinMode(JOYSTICK_SW, INPUT_PULLUP); // Internal pull-up resistor for button
}

void loop() 
{
  //Leer datos de JoyStick
  telemetryControl.x = analogRead(JOYSTICK_X); // Range: 0-4095
  telemetryControl.y = analogRead(JOYSTICK_Y); // Range: 0-4095
  telemetryControl.brake = digitalRead(JOYSTICK_SW) == LOW;
  
  enviarDatos();
  Serial.print("X: ");
  Serial.print(telemetryControl.x );
  Serial.print(" | Y: ");
  Serial.println(telemetryControl.y);
  delay(10); 
}
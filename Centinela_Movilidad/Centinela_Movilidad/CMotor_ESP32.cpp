/*
*****************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código hecho por Jose Sebastian Mendez Peyro
Clase de motor desarrollada especificamente para esp32 y driver de motores MDD10A
******************************
*/

#include "CMotor_ESP32.h"

uint8_t Motor::channelCounter = 0; // Inicializamos el contador estático de canales

Motor::Motor(uint8_t dir, uint8_t pwm) // Constructor de clase motor en donde declaramos los respectivos valores para dir y el pwm.
{
  pinDIR = dir;
  pinPWM = pwm;

  initialized = false;
}

bool Motor::begin(uint32_t freq, uint8_t resolution) 
{
  //Serial.printf("Inicializando motor en pin DIR=%d PWM=%d\n", pinDIR, pinPWM);
  pinMode(pinDIR, OUTPUT);    // Declaramos el pin de DIR como salida
  channel = channelCounter; // Asignamos el canal de salida PWM automáticamente
  channelCounter++;         // Aumentamos el contador del número de canales PWM

  if (ledcAttachChannel(pinPWM, freq, resolution, channel) == false)   // Declaracion del pin PWM que se utilizará y comprobación que se inicialice correctamente
  {
    initialized = false;
    return false;
  }

  stop();                     // Le damos stop para que no se mueva el motor al inicializar
  initialized = true;
  return initialized;
}

void Motor::forward(uint8_t vel)
{
  if (!initialized) return;        // Verificación que el motor este inicializado para seguridad del código.
  digitalWrite(pinDIR, HIGH);      // Definimos el estado HIGH (1) como avance.
  ledcWriteChannel(channel, vel);  // Actualizamos el valor de velocidad que recibe el motor en el canal dedicado al motor. 
}                                  // Se utiliza ledcWrite porque esp32 no soporta analogWrite.

void Motor::backward(uint8_t vel)
{
  if (!initialized) return;      
  digitalWrite(pinDIR, LOW);       // Definimos el estado LOW (0) como retroceder.
  ledcWriteChannel(channel, vel);  // Actualizamos el valor de velocidad que recibe el motor en el canal dedicado al motor.
}

void Motor::stop()
{
  if (!initialized) return;      
  digitalWrite(pinDIR, LOW);      // Definimos el estado LOW (0) como retroceder.
  ledcWriteChannel(channel, 0);   // Actualizamos el valor de velocidad que recibe el motor en el canal dedicado al motor. 
}

uint8_t Motor::getChannel()
{
  return channel;
}

//Ya se porque no funcionaba mi codigo originalmente, porque estaba haciendo el serialprint de los motores front right y front left cuando yo estaba usando los back right y back left.
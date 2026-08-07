/***************************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Codigo de pruebas para control de motores de hoverboard usando driver brushless RioRand 350W.

Este código se encuentra en la placa ESP32 conectada por I2C al T-Beam. El T-Beam recibe los comandos por LoRa y los envia a la esp32 por I2C.

El objetivo de este código es comprobar la recepción de datos de control en la esp32.

Librerias necesarias:

Board en ArduinoIDE es ESP32 Dev Module
**************************************************************************************************************************************************/

#define MDD10A_ESP32 // Definimos la configuracion Driver-Motor que estaremos utilizando. Esta es nuestra declaración de pines.

#include "CMotor_ESP32.h"     // Definimos nuestra clase de motor que usaremos

uint8_t velocidad_ruedas = 255;

Motor front_left_motor(FL_DIR, FL_PWM);
Motor front_right_motor(FR_DIR, FR_PWM);
Motor middle_left_motor(ML_DIR, ML_PWM);
Motor middle_right_motor(MR_DIR, MR_PWM);
Motor back_left_motor(BL_DIR, BL_PWM);
Motor back_right_motor(BR_DIR, BR_PWM);

void setup() 
{
  Serial.begin(115200);
  delay(3000);      //Delay para que se estabilice el Serial
  inicializarMotores();
}

void loop() 
{
  //recibir datos I2C

  //
  
  
  //Secuencia de prueba simple
  avanzar(velocidad_ruedas);        //Prueba de funcion avanzar.
  delay(10000);
  back_left_motor.stop();           //Prueba de funcion stop
  back_right_motor.stop();
  delay(10000);
  back_left_motor.backward(velocidad_ruedas);     //Prueba de funcion para retroceder
  back_right_motor.backward(velocidad_ruedas);
  delay(10000);
}

void inicializarMotores()
{
  
  if (front_left_motor.begin()) 
    Serial.println("Front Left Motor se inicializó correctamente");
  else 
    Serial.println("Error inicializando Front Left Motor.");
    
  if (front_right_motor.begin())
    Serial.println("Front Right Motor inicializó correctamente");
  else
    Serial.println("Error inicializando Front Right Motor.");

  if (middle_left_motor.begin()) 
    Serial.println("Middle Left Motor se inicializó correctamente");
  else 
    Serial.println("Error inicializando Middle Left Motor.");

  if (middle_right_motor.begin()) 
    Serial.println("Middle Right Motor se inicializó correctamente");
  else 
    Serial.println("Error inicializando Middle Right Motor.");

  if (back_left_motor.begin()) 
    Serial.println("Back Left Motor se inicializó correctamente");
  else 
    Serial.println("Error inicializando Back Left Motor.");

  if (back_right_motor.begin()) 
    Serial.println("Back Right Motor se inicializó correctamente");
  else 
    Serial.println("Error inicializando Back Right Motor.");
}

void avanzar(int velocidad)
{
  front_left_motor.forward(velocidad);
  front_right_motor.forward(velocidad);
  middle_left_motor.forward(velocidad);
  middle_right_motor.forward(velocidad);
  back_left_motor.forward(velocidad);
  back_right_motor.forward(velocidad);
}
void retroceder(int velocidad)
{
  front_left_motor.backward(velocidad);
  front_right_motor.backward(velocidad);
  middle_left_motor.backward(velocidad);
  middle_right_motor.backward(velocidad);
  back_left_motor.backward(velocidad);
  back_right_motor.backward(velocidad);
}
/*
*****************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código hecho por Jose Sebastian Mendez Peyro
Clase de motor desarrollada especificamente para esp32 y driver de motores MDD10A
******************************
*/

#include <Arduino.h> // Al ser archivo (.h) necesitamos declarar que usaremos las librerias de arduino
#include <driver/ledc.h>
#include <cstdint>

#ifndef MOTOR_H // if para que el archivo de cabecera (.h) solo se incluya una vez, incluso si es llamado varias veces por diferentes archivos del proyecto.
#define MOTOR_H 

//Declaracion de pines a utilizar de la esp 32. Consideracion de Esp32 Wroom 32 de 38 pines.
#ifdef MDD10A_ESP32
  #define FL_DIR 4 // Front Left Motor DIR pin. DIR 1
  #define FL_PWM 13 // Front Left Motor PWM pin. PWM 1

  #define FR_DIR 16 // Front Right Motor DIR pin. DIR 2
  #define FR_PWM 27 // Front Right Motor PWM pin. PWM 2
  
  #define ML_DIR 17 // Middle Left Motor DIR pin. DIR 1 
  #define ML_PWM 26 // Middle Left Motor PWM pin. PWM 1
  
  #define MR_DIR 18 // Middle Right Motor DIR pin. DIR 2
  #define MR_PWM 25 // Middle Right Motor PWM pin. PWM 2
  
  #define BL_DIR 19 // Back Left Motor DIR pin. DIR 1
  #define BL_PWM 33 // Back Left Motor PWM pin. PWM 1
  
  #define BR_DIR 23 // Back Right Motor DIR pin. DIR 2
  #define BR_PWM 32 // Back Right Motor PWM pin. PWM 2
#endif

#define PWM_FREQUENCY 18000
#define PWM_RESOLUTION 8

class Motor
{
  private:
    uint8_t pinDIR;   // Define el numero de pin para la DIR del respectivo motor. // Se usa entero de 8 bits sin signo para tomar valores desde 0
    uint8_t pinPWM;   // Define el numero de pin para el PWM del respectivo motor.    hasta 255 por ser el tipo de dato mas pequeño. Ocupan 1 byte de memoria.
    uint8_t channel;  // Define el número de canal para el pin PWM del respectivo motor
    bool initialized; // Variable para saber si se inicializó el motor

    static uint8_t channelCounter;  //Contador del número de cananles creados

  public:
    Motor(uint8_t dir, uint8_t pwm);      // Constructor de la clase.

    bool begin(uint32_t freq = PWM_FREQUENCY, uint8_t resolution = PWM_RESOLUTION); // Declaracion de begin para inicializar los pines y asignar los valores fijos para la frecuencia y resolucion de las salidas PWM del esp32.
    void forward(uint8_t vel);            // Declaracion de funcion para avanzar.
    void backward(uint8_t vel);           // Declaracion de funcion para retroceder.
    void stop();                          // Declaracion para funcion de detener.
    uint8_t getChannel();
};

#endif
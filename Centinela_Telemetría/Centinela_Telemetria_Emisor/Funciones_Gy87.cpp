/****************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo cpp con la declaración de funciones del GY87 para el módulo de telemetría TRANSMISOR/EMISOR implementando LilyGo TTGO T-Beam V1.2

Link del video sobre como usar los registros: https://youtu.be/M9lZ5Qy5S2s?si=_7S1XLJ0XDP_fVhh
*****************************************************************************************************************************************/

#define Pines_Telemetria
#include "Telemetria_Emisor.h"

//Funcion para establecer comunicación con el MPU e incializar los registros
void inicializarGY87()
{

  // Verificar WHO_AM_I
  Wire.beginTransmission(0x68);
  Wire.write(0x75); // WHO_AM_I register
  Wire.endTransmission();
  Wire.requestFrom(0x68, (uint8_t)1);
  if (Wire.available()) 
  {
    uint8_t who = Wire.read();

    String msg = "WHO_AM_I = 0x";
    msg += String(who, HEX);
    sendMessage(msg); delay(time_delay);  //I2C Inicializado

    Wire.beginTransmission(0x68); //This is the I2C address of the MPU (b1101000/b1101001 or 0x68/0x69 for AC0 low/high datasheet sec. 9.2)
    Wire.write(0x6B); //Accessing the register 6B - Power Management (Sec. 4.28)
    Wire.write(0x00); //Setting SLEEP register to 0. (Required; see Note on p. 9)
    Wire.endTransmission();  

    Wire.beginTransmission(0x68); //I2C address of the MPU
    Wire.write(0x1B); //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4) 
    Wire.write(0x00); //Setting the gyro to full scale +/- 250deg./s 
    Wire.endTransmission(); 

    Wire.beginTransmission(0x68); //I2C address of the MPU
    Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5) 
    Wire.write(0x00); //Setting the accel to +/- 2g
    Wire.endTransmission(); 

    sendMessage("GY87 inicializado correctamente."); delay(time_delay);
    #ifdef BUZZER
      tonoBuzzerCorrecto();
    #endif
  } 
  else 
  {
    //Serial.println("No responde WHO_AM_I. Revisa I2C/dirección.");
    sendMessage("No responde WHO_AM_I. Revisa I2C/dirección.");
    #ifdef BUZZER
      tonoBuzzerError();
    #endif
  }
}

void leerAceleracion()
{
  Wire.beginTransmission(0x68); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();

  Wire.requestFrom(0x68,6); //Request Accel Registers (3B - 40)
  while(Wire.available() < 6);
  RAW_accelX = (int16_t)(Wire.read() << 8 | Wire.read());
  RAW_accelY = (int16_t)(Wire.read() << 8 | Wire.read());
  RAW_accelZ = (int16_t)(Wire.read() << 8 | Wire.read());
  processAccelData();
}

void leerGiroscopio()
{
  Wire.beginTransmission(0x68); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();

  Wire.requestFrom(0x68,6); //Request Gyro Registers (43 - 48)
  while(Wire.available() < 6);
  RAW_gyroX = (int16_t)(Wire.read()<<8|Wire.read()); //Store first two bytes into accelX
  RAW_gyroY = (int16_t)(Wire.read()<<8|Wire.read()); //Store middle two bytes into accelY
  RAW_gyroZ = (int16_t)(Wire.read()<<8|Wire.read()); //Store last two bytes into accelZ
  processGyroData();
}

void processGyroData() 
{
  rotX = RAW_gyroX / 131.0;
  rotY = RAW_gyroY / 131.0; 
  rotZ = RAW_gyroZ / 131.0;
}

void processAccelData()
{
  accelX = (RAW_accelX / 16384.0)*9.80665;
  accelY = (RAW_accelY / 16384.0)*9.80665; 
  accelZ = (RAW_accelZ / 16384.0)*9.80665;
}
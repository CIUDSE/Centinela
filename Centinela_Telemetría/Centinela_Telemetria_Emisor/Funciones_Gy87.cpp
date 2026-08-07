/****************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo cpp con la declaración de funciones del GY87 para el módulo de telemetría.

Link del video sobre como usar los registros: https://youtu.be/M9lZ5Qy5S2s?si=_7S1XLJ0XDP_fVhh
*****************************************************************************************************************************************/
#include "Telemetria_Emisor.h"
#include <math.h>

int16_t RAW_accelX, RAW_accelY, RAW_accelZ;   //Valores RAW
int16_t RAW_gyroX, RAW_gyroY, RAW_gyroZ;      //Valores RAW

// Variables físicas aceleración y giroscopio
float accelX_mps2, accelY_mps2, accelZ_mps2;
float gyroX_dps, gyroY_dps, gyroZ_dps;

// Offsets de calibración
float accelX_offset = 0;
float accelY_offset = 0;
float accelZ_offset = 0;
float gyroX_offset = 0;
float gyroY_offset = 0;
float gyroZ_offset = 0;

// Variables temporales para el ángulo estimado solo con Acelerómetro
float pitchAcc = 0.0;
float rollAcc  = 0.0;

// Constantes físicas del sensor y filtro complementario
const float ACC_SENS = 16384.0;   // ±2g
const float GYRO_SENS = 131.0;    // ±250°/s
const float G_TO_MPS2 = 9.80665;
const float dt = 0.01;            // 100 Hz (10 ms)
const float alpha = 0.98;         // Coeficiente del filtro

//Funcion para establecer comunicación con el MPU e incializar los registros
void inicializarGY87()
{
  // Verificar WHO_AM_I
  Wire.beginTransmission(GY87_ADDRESS);
  Wire.write(0x75); // WHO_AM_I register
  Wire.endTransmission();
  Wire.requestFrom(GY87_ADDRESS, (uint8_t)1);

  if (Wire.available()) 
  {
    Wire.beginTransmission(GY87_ADDRESS); //This is the I2C address of the MPU (b1101000/b1101001 or 0x68/0x69 for AC0 low/high datasheet sec. 9.2)
    Wire.write(0x6B); //Accessing the register 6B - Power Management (Sec. 4.28)
    Wire.write(0x00); //Setting SLEEP register to 0. (Required; see Note on p. 9)
    Wire.endTransmission();  

    Wire.beginTransmission(GY87_ADDRESS); //I2C address of the MPU
    Wire.write(0x1B); //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4) 
    Wire.write(0x00); //Setting the gyro to full scale +/- 250deg./s 
    Wire.endTransmission(); 

    Wire.beginTransmission(GY87_ADDRESS); //I2C address of the MPU
    Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5) 
    Wire.write(0x00); //Setting the accel to +/- 2g
    Wire.endTransmission(); 
  } 
  else 
  {
    Serial.println("No responde WHO_AM_I. Revisa I2C/dirección.");
    //sendMessage("No responde WHO_AM_I. Revisa I2C/dirección.");
  }
}

void leerAcelerometro()
{
  Wire.beginTransmission(GY87_ADDRESS); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();

  Wire.requestFrom(GY87_ADDRESS,6); //Request Accel Registers (3B - 40)
  while(Wire.available() < 6);
  RAW_accelX = (int16_t)(Wire.read() << 8 | Wire.read());
  RAW_accelY = (int16_t)(Wire.read() << 8 | Wire.read());
  RAW_accelZ = (int16_t)(Wire.read() << 8 | Wire.read());
  processAccelData();
}

void leerGiroscopio()
{
  Wire.beginTransmission(GY87_ADDRESS); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();

  Wire.requestFrom(GY87_ADDRESS,6); //Request Gyro Registers (43 - 48)
  while(Wire.available() < 6);
  RAW_gyroX = (int16_t)(Wire.read()<<8|Wire.read()); //Store first two bytes into accelX
  RAW_gyroY = (int16_t)(Wire.read()<<8|Wire.read()); //Store middle two bytes into accelY
  RAW_gyroZ = (int16_t)(Wire.read()<<8|Wire.read()); //Store last two bytes into accelZ
  processGyroData();
}

void processGyroData() 
{
  sensorData.vel_ang_x = RAW_gyroX / 131.0;
  sensorData.vel_ang_y = RAW_gyroY / 131.0; 
  sensorData.vel_ang_z = RAW_gyroZ / 131.0;

  //Valores convertidos a entero para enviar en la telemetría
  telemetryData.vel_ang_x_32 = (int32_t)(sensorData.vel_ang_x * 100);
  telemetryData.vel_ang_y_32 = (int32_t)(sensorData.vel_ang_y * 100);
  telemetryData.vel_ang_z_32 = (int32_t)(sensorData.vel_ang_z * 100);
}

void processAccelData()
{
  sensorData.accel_x = (RAW_accelX / 16384.0)*9.80665;
  sensorData.accel_y = (RAW_accelY / 16384.0)*9.80665; 
  sensorData.accel_z = (RAW_accelZ / 16384.0)*9.80665;

  telemetryData.accel_x_16 = (int16_t)(sensorData.accel_x * 100);
  telemetryData.accel_y_16 = (int16_t)(sensorData.accel_y * 100);
  telemetryData.accel_z_16 = (int16_t)(sensorData.accel_z * 100); 
}

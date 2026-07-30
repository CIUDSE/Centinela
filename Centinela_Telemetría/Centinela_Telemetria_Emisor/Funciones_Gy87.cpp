/****************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Archivo cpp con la declaración de funciones del GY87 para el módulo de telemetría TRANSMISOR/EMISOR implementando LilyGo TTGO T-Beam V1.2

Link del video sobre como usar los registros: https://youtu.be/M9lZ5Qy5S2s?si=_7S1XLJ0XDP_fVhh
*****************************************************************************************************************************************/

#define Pines_Telemetria
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
    //Serial.println("No responde WHO_AM_I. Revisa I2C/dirección.");
    //sendMessage("No responde WHO_AM_I. Revisa I2C/dirección.");
  }
}

// Función para calibrar los offsets en reposo (mantiene la lectura RAW original)
void calibrarGY87()
{
  long sumAx = 0, sumAy = 0, sumAz = 0;
  long sumGx = 0, sumGy = 0, sumGz = 0;
  const int N = 500;

  for(int i = 0; i < N; i++) {
    leerAcelerometro();
    leerGiroscopio();

    sumAx += RAW_accelX; 
    sumAy += RAW_accelY; 
    sumAz += RAW_accelZ;

    sumGx += RAW_gyroX; 
    sumGy += RAW_gyroY; 
    sumGz += RAW_gyroZ;

    delay(2);
  }

  accelX_offset = (sumAx / (float)N) / ACC_SENS * G_TO_MPS2;
  accelY_offset = (sumAy / (float)N) / ACC_SENS * G_TO_MPS2;
  accelZ_offset = ((sumAz / (float)N) / ACC_SENS * G_TO_MPS2) - G_TO_MPS2;

  gyroX_offset = (sumGx / (float)N) / GYRO_SENS;
  gyroY_offset = (sumGy / (float)N) / GYRO_SENS;
  gyroZ_offset = (sumGz / (float)N) / GYRO_SENS;
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

void processAccelData()
{
  // Mantiene la fórmula original restando únicamente el offset de calibración
  accelX_mps2 = (RAW_accelX / ACC_SENS) * G_TO_MPS2 - accelX_offset;
  accelY_mps2 = (RAW_accelY / ACC_SENS) * G_TO_MPS2 - accelY_offset;
  accelZ_mps2 = (RAW_accelZ / ACC_SENS) * G_TO_MPS2 - accelZ_offset;

  telemetryData.accelX = accelX_mps2;
  telemetryData.accelY = accelY_mps2;
  telemetryData.accelZ = accelZ_mps2;

  // Calculo de la inclinación del acelerómetro
  pitchAcc = atan2(accelY_mps2, sqrt(accelX_mps2 * accelX_mps2 + accelZ_mps2 * accelZ_mps2)) * 180.0 / M_PI;
  rollAcc  = atan2(-accelX_mps2, accelZ_mps2) * 180.0 / M_PI;

}

void processGyroData() 
{
  // Mantiene la fórmula original restando únicamente el offset de calibración
  gyroX_dps = (RAW_gyroX / GYRO_SENS) - gyroX_offset;
  gyroY_dps = (RAW_gyroY / GYRO_SENS) - gyroY_offset;
  gyroZ_dps = (RAW_gyroZ / GYRO_SENS) - gyroZ_offset;

  telemetryData.rotX = gyroX_dps; 
  telemetryData.rotY = gyroY_dps;
  telemetryData.rotZ = gyroZ_dps;

  telemetryData.pitch = alpha * (telemetryData.pitch + gyroX_dps * dt) + (1 - alpha) * pitchAcc;
  telemetryData.roll  = alpha * (telemetryData.roll  + gyroY_dps * dt) + (1 - alpha) * rollAcc;
  telemetryData.yaw = telemetryData.yaw + gyroZ_dps * dt; 

  if (telemetryData.yaw >= 180.0) telemetryData.yaw -= 360.0;
  else if (telemetryData.yaw < -180.0) telemetryData.yaw += 360.0;
}


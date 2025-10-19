/*
MPU 6050 Código de prueba

Este es el código de prueba base (Sin calibración) para el MPU6050 que se puede encontrar en placas como el Gy-521 o el Gy-87

Link del video sobre como usar los registros: https://youtu.be/M9lZ5Qy5S2s?si=_7S1XLJ0XDP_fVhh
*/

#include <Wire.h>

int16_t accelX, accelY, accelZ;
float gForceX, gForceY, gForceZ;

int16_t gyroX, gyroY, gyroZ;
float rotX, rotY, rotZ;

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  setupMPU();
}

void loop()
{
  recordAccelRegisters();
  recordGyroRegisters();
  printData();
  delay(1000);
}

//Funcion para establecer comunicación con el MPU e incializar los registros
void setupMPU()
{
  // Verificar WHO_AM_I
  Wire.beginTransmission(0x68);
  Wire.write(0x75); // WHO_AM_I register
  Wire.endTransmission();
  Wire.requestFrom(0x68, (uint8_t)1);
  if (Wire.available()) {
    uint8_t who = Wire.read();
    Serial.print("WHO_AM_I = 0x"); Serial.println(who, HEX);
  } else {
    Serial.println("No responde WHO_AM_I. Revisa I2C/dirección.");
  }

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
}

void recordAccelRegisters() {
  Wire.beginTransmission(0x68); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();

  Wire.requestFrom(0x68,6); //Request Accel Registers (3B - 40)
  while(Wire.available() < 6);
  accelX = (int16_t)(Wire.read() << 8 | Wire.read());
  accelY = (int16_t)(Wire.read() << 8 | Wire.read());
  accelZ = (int16_t)(Wire.read() << 8 | Wire.read());
  processAccelData();
}

void processAccelData(){
  gForceX = (accelX / 16384.0)*9.80665;
  gForceY = (accelY / 16384.0)*9.80665; 
  gForceZ = (accelZ / 16384.0)*9.80665;
}

void recordGyroRegisters() {
  Wire.beginTransmission(0x68); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();

  Wire.requestFrom(0x68,6); //Request Gyro Registers (43 - 48)
  while(Wire.available() < 6);
  gyroX = (int16_t)(Wire.read()<<8|Wire.read()); //Store first two bytes into accelX
  gyroY = (int16_t)(Wire.read()<<8|Wire.read()); //Store middle two bytes into accelY
  gyroZ = (int16_t)(Wire.read()<<8|Wire.read()); //Store last two bytes into accelZ
  processGyroData();
}

void processGyroData() {
  rotX = gyroX / 131.0;
  rotY = gyroY / 131.0; 
  rotZ = gyroZ / 131.0;
}

void printData() {
  Serial.print("Gyro (deg)");
  Serial.print(" X=");
  Serial.print(rotX);
  Serial.print(" Y=");
  Serial.print(rotY);
  Serial.print(" Z=");
  Serial.print(rotZ);
  Serial.print(" Accel (RAW)");
  Serial.print(" X=");
  Serial.print(accelX);
  Serial.print(" Y=");
  Serial.print(accelY);
  Serial.print(" Z=");
  Serial.print(accelZ);
  Serial.print(" Accel (m/s^2)");
  Serial.print(" X=");
  Serial.print(gForceX);
  Serial.print(" Y=");
  Serial.print(gForceY);
  Serial.print(" Z=");
  Serial.println(gForceZ);
}
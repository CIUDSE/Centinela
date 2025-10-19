/*
MPU 6050 Código de prueba

Este es el código de prueba para el MPU6050 con calibracion implementada que se puede encontrar en placas como el Gy-521 o el Gy-87

Link del video sobre como usar los registros: https://youtu.be/M9lZ5Qy5S2s?si=_7S1XLJ0XDP_fVhh
*/

#include <Wire.h>
#include <math.h>

// Lecturas crudas
int16_t accelX, accelY, accelZ;
int16_t gyroX, gyroY, gyroZ;

// Lecturas físicas
float accelX_mps2, accelY_mps2, accelZ_mps2;
float gyroX_dps, gyroY_dps, gyroZ_dps;

// Offsets
float accelX_offset = 0;
float accelY_offset = 0;
float accelZ_offset = 0;
float gyroX_offset = 0;
float gyroY_offset = 0;
float gyroZ_offset = 0;

// Constantes
const float ACC_SENS = 16384.0;   // ±2g
const float GYRO_SENS = 131.0;    // ±250°/s
const float G_TO_MPS2 = 9.80665;
const float dt = 0.01;            // 100 Hz
const float alpha = 0.98;         // Filtro complementario
const float accelThreshold = 0.05; // m/s² deadband
const float filterVelAlpha = 0.95; // Suavizado de velocidad

// Ángulos
float pitch = 0;
float roll = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  setupMPU();
  calibrateMPU();
  Serial.println("Inicio completo. Leyendo datos...");
}

void loop() {
  recordAccelRegisters();
  recordGyroRegisters();
  processAccelData();
  processGyroData();

  // ---------------- Filtro complementario ----------------
  float pitchAcc = atan2(accelY_mps2, sqrt(accelX_mps2*accelX_mps2 + accelZ_mps2*accelZ_mps2)) * 180.0 / PI;
  float rollAcc  = atan2(-accelX_mps2, accelZ_mps2) * 180.0 / PI;

  pitch = alpha * (pitch + gyroX_dps * dt) + (1 - alpha) * pitchAcc;
  roll  = alpha * (roll + gyroY_dps * dt)  + (1 - alpha) * rollAcc;

  // ---------------- Compensar gravedad ----------------
  float gX = G_TO_MPS2 * sin(pitch * PI/180);
  float gY = G_TO_MPS2 * sin(roll * PI/180);
  float gZ = G_TO_MPS2 * cos(pitch * PI/180) * cos(roll * PI/180);

  float ax_lin = accelX_mps2 - gX;
  float ay_lin = accelY_mps2 - gY;

  // ---------------- Deadband ----------------
  if(abs(ax_lin) < accelThreshold) ax_lin = 0;
  if(abs(ay_lin) < accelThreshold) ay_lin = 0;

  // ---------------- Imprimir datos ----------------
  Serial.print("Pitch=");
  Serial.print(pitch,2);
  Serial.print(" Roll=");
  Serial.print(roll,2);

  Serial.print(" | Accel (m/s^2) X=");
  Serial.print(accelX_mps2,2);
  Serial.print(" Y=");
  Serial.print(accelY_mps2,2);
  Serial.print(" Z=");
  Serial.print(accelZ_mps2,2);

  Serial.print(" | Gyro (deg/s) X=");
  Serial.print(gyroX_dps,2);
  Serial.print(" Y=");
  Serial.print(gyroY_dps,2);
  Serial.print(" Z=");
  Serial.println(gyroZ_dps,2);

  delay(10); // 100 Hz
}

// ---------------------- Configuración MPU ----------------------
void setupMPU() {
  Wire.beginTransmission(0x68);
  Wire.write(0x6B); Wire.write(0x00); Wire.endTransmission(); // Wake up
  Wire.beginTransmission(0x68);
  Wire.write(0x1B); Wire.write(0x00); Wire.endTransmission(); // Gyro ±250°/s
  Wire.beginTransmission(0x68);
  Wire.write(0x1C); Wire.write(0x00); Wire.endTransmission(); // Accel ±2g
}

// ---------------------- Calibración ----------------------
void calibrateMPU() {
  long sumAx=0, sumAy=0, sumAz=0;
  long sumGx=0, sumGy=0, sumGz=0;
  const int N = 500;

  Serial.println("Calibrando MPU6050, mantenga el sensor quieto...");
  for(int i=0; i<N; i++){
    recordAccelRegisters();
    recordGyroRegisters();
    sumAx += accelX; sumAy += accelY; sumAz += accelZ;
    sumGx += gyroX; sumGy += gyroY; sumGz += gyroZ;
    delay(5);
  }

  accelX_offset = (sumAx / N) / ACC_SENS * G_TO_MPS2;
  accelY_offset = (sumAy / N) / ACC_SENS * G_TO_MPS2;
  accelZ_offset = ((sumAz / N) / ACC_SENS * G_TO_MPS2) - G_TO_MPS2;

  gyroX_offset = (sumGx / N) / GYRO_SENS;
  gyroY_offset = (sumGy / N) / GYRO_SENS;
  gyroZ_offset = (sumGz / N) / GYRO_SENS;

  Serial.println("Calibración completa.");
}

// ---------------------- Lectura acelerómetro ----------------------
void recordAccelRegisters() {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission();
  Wire.requestFrom(0x68,6);
  while(Wire.available()<6);
  accelX = (int16_t)(Wire.read()<<8 | Wire.read());
  accelY = (int16_t)(Wire.read()<<8 | Wire.read());
  accelZ = (int16_t)(Wire.read()<<8 | Wire.read());
}

void processAccelData() {
  accelX_mps2 = (accelX / ACC_SENS) * G_TO_MPS2 - accelX_offset;
  accelY_mps2 = (accelY / ACC_SENS) * G_TO_MPS2 - accelY_offset;
  accelZ_mps2 = (accelZ / ACC_SENS) * G_TO_MPS2 - accelZ_offset;
}

// ---------------------- Lectura giroscopio ----------------------
void recordGyroRegisters() {
  Wire.beginTransmission(0x68);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0x68,6);
  while(Wire.available()<6);
  gyroX = (int16_t)(Wire.read()<<8 | Wire.read());
  gyroY = (int16_t)(Wire.read()<<8 | Wire.read());
  gyroZ = (int16_t)(Wire.read()<<8 | Wire.read());
}

void processGyroData() {
  gyroX_dps = (gyroX / GYRO_SENS) - gyroX_offset;
  gyroY_dps = (gyroY / GYRO_SENS) - gyroY_offset;
  gyroZ_dps = (gyroZ / GYRO_SENS) - gyroZ_offset;
}


/*
#include <Wire.h>
#include <math.h>

// Lecturas crudas
int16_t accelX, accelY, accelZ;
int16_t gyroX, gyroY, gyroZ;

// Lecturas físicas
float accelX_mps2, accelY_mps2, accelZ_mps2;
float gyroX_dps, gyroY_dps, gyroZ_dps;

// Offsets calculados en calibración
float accelX_offset = 0;
float accelY_offset = 0;
float accelZ_offset = 0;
float gyroX_offset = 0;
float gyroY_offset = 0;
float gyroZ_offset = 0;

// Constantes
const float ACC_SENS = 16384.0;   // ±2g
const float GYRO_SENS = 131.0;    // ±250°/s
const float G_TO_MPS2 = 9.80665;
const float dt = 0.01;            // 10 ms ~100Hz
const float alpha = 0.98;         // Filtro complementario

// Ángulos
float pitch = 0;
float roll = 0;

// Velocidad lineal
float vX = 0, vY = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  setupMPU();
  calibrateMPU();
  Serial.println("Inicio completo. Leyendo datos...");
}

void loop() {
  recordAccelRegisters();
  recordGyroRegisters();
  processAccelData();
  processGyroData();

  // ---------------- Filtro complementario ----------------
  float pitchAcc = atan2(accelY_mps2, sqrt(accelX_mps2*accelX_mps2 + accelZ_mps2*accelZ_mps2)) * 180.0 / PI;
  float rollAcc  = atan2(-accelX_mps2, accelZ_mps2) * 180.0 / PI;

  pitch = alpha * (pitch + gyroX_dps * dt) + (1 - alpha) * pitchAcc;
  roll  = alpha * (roll + gyroY_dps * dt)  + (1 - alpha) * rollAcc;

  // ---------------- Compensar gravedad ----------------
  float gX = G_TO_MPS2 * sin(pitch * PI/180);
  float gY = G_TO_MPS2 * sin(roll * PI/180);
  float gZ = G_TO_MPS2 * cos(pitch * PI/180) * cos(roll * PI/180);

  float ax_lin = accelX_mps2 - gX;
  float ay_lin = accelY_mps2 - gY;
  // Z se puede calcular si quieres velocidad vertical
  // float az_lin = accelZ_mps2 - gZ;

  // ---------------- Integración para velocidad ----------------
  vX += ax_lin * dt;
  vY += ay_lin * dt;

  // ---------------- Imprimir datos ----------------
  Serial.print("Pitch=");
  Serial.print(pitch,2);
  Serial.print(" Roll=");
  Serial.print(roll,2);

  Serial.print(" | Velocidad (m/s) X=");
  Serial.print(vX,2);
  Serial.print(" Y=");
  Serial.print(vY,2);

  Serial.print(" | Accel (m/s^2) X=");
  Serial.print(accelX_mps2,2);
  Serial.print(" Y=");
  Serial.print(accelY_mps2,2);
  Serial.print(" Z=");
  Serial.print(accelZ_mps2,2);

  Serial.print(" | Gyro (deg/s) X=");
  Serial.print(gyroX_dps,2);
  Serial.print(" Y=");
  Serial.print(gyroY_dps,2);
  Serial.print(" Z=");
  Serial.println(gyroZ_dps,2);

  delay(10); // 100 Hz
}

// ---------------------- Configuración MPU ----------------------
void setupMPU() {
  Wire.beginTransmission(0x68);
  Wire.write(0x6B); Wire.write(0x00); Wire.endTransmission(); // Wake up
  Wire.beginTransmission(0x68);
  Wire.write(0x1B); Wire.write(0x00); Wire.endTransmission(); // Gyro ±250°/s
  Wire.beginTransmission(0x68);
  Wire.write(0x1C); Wire.write(0x00); Wire.endTransmission(); // Accel ±2g
}

// ---------------------- Calibración ----------------------
void calibrateMPU() {
  long sumAx=0, sumAy=0, sumAz=0;
  long sumGx=0, sumGy=0, sumGz=0;
  const int N = 500;

  Serial.println("Calibrando MPU6050, mantenga el sensor quieto...");
  for(int i=0; i<N; i++){
    recordAccelRegisters();
    recordGyroRegisters();
    sumAx += accelX; sumAy += accelY; sumAz += accelZ;
    sumGx += gyroX; sumGy += gyroY; sumGz += gyroZ;
    delay(5);
  }

  accelX_offset = (sumAx / N) / ACC_SENS * G_TO_MPS2;
  accelY_offset = (sumAy / N) / ACC_SENS * G_TO_MPS2;
  accelZ_offset = ((sumAz / N) / ACC_SENS * G_TO_MPS2) - G_TO_MPS2;

  gyroX_offset = (sumGx / N) / GYRO_SENS;
  gyroY_offset = (sumGy / N) / GYRO_SENS;
  gyroZ_offset = (sumGz / N) / GYRO_SENS;

  Serial.println("Calibración completa.");
}

// ---------------------- Lectura acelerómetro ----------------------
void recordAccelRegisters() {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission();
  Wire.requestFrom(0x68,6);
  while(Wire.available()<6);
  accelX = (int16_t)(Wire.read()<<8 | Wire.read());
  accelY = (int16_t)(Wire.read()<<8 | Wire.read());
  accelZ = (int16_t)(Wire.read()<<8 | Wire.read());
}

void processAccelData() {
  accelX_mps2 = (accelX / ACC_SENS) * G_TO_MPS2 - accelX_offset;
  accelY_mps2 = (accelY / ACC_SENS) * G_TO_MPS2 - accelY_offset;
  accelZ_mps2 = (accelZ / ACC_SENS) * G_TO_MPS2 - accelZ_offset;
}

// ---------------------- Lectura giroscopio ----------------------
void recordGyroRegisters() {
  Wire.beginTransmission(0x68);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0x68,6);
  while(Wire.available()<6);
  gyroX = (int16_t)(Wire.read()<<8 | Wire.read());
  gyroY = (int16_t)(Wire.read()<<8 | Wire.read());
  gyroZ = (int16_t)(Wire.read()<<8 | Wire.read());
}

void processGyroData() {
  gyroX_dps = (gyroX / GYRO_SENS) - gyroX_offset;
  gyroY_dps = (gyroY / GYRO_SENS) - gyroY_offset;
  gyroZ_dps = (gyroZ / GYRO_SENS) - gyroZ_offset;
}



/*
#include <Wire.h>
#include <math.h>

// Lecturas crudas
int16_t accelX, accelY, accelZ;
int16_t gyroX, gyroY, gyroZ;

// Lecturas físicas
float accelX_mps2, accelY_mps2, accelZ_mps2;
float gyroX_dps, gyroY_dps, gyroZ_dps;

// Offsets calculados en calibración
float accelX_offset = 0;
float accelY_offset = 0;
float accelZ_offset = 0;
float gyroX_offset = 0;
float gyroY_offset = 0;
float gyroZ_offset = 0;

// Constantes
const float ACC_SENS = 16384.0;   // ±2g
const float GYRO_SENS = 131.0;    // ±250°/s
const float G_TO_MPS2 = 9.80665;
const float dt = 0.01;            // 10 ms ~100Hz
const float alpha = 0.98;         // Filtro complementario

// Ángulos
float pitch = 0;
float roll = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  setupMPU();
  calibrateMPU();
  Serial.println("Inicio completo. Leyendo datos...");
}

void loop() {
  recordAccelRegisters();
  recordGyroRegisters();
  processAccelData();
  processGyroData();

  // ---------------- Filtro complementario ----------------
  float pitchAcc = atan2(accelY_mps2, sqrt(accelX_mps2*accelX_mps2 + accelZ_mps2*accelZ_mps2)) * 180.0 / PI;
  float rollAcc  = atan2(-accelX_mps2, accelZ_mps2) * 180.0 / PI;

  pitch = alpha * (pitch + gyroX_dps * dt) + (1 - alpha) * pitchAcc;
  roll  = alpha * (roll + gyroY_dps * dt)  + (1 - alpha) * rollAcc;

  // ---------------- Imprimir ----------------
  Serial.print("Pitch=");
  Serial.print(pitch,2);
  Serial.print(" Roll=");
  Serial.print(roll,2);
  Serial.print(" | Accel (m/s^2) X=");
  Serial.print(accelX_mps2,2);
  Serial.print(" Y=");
  Serial.print(accelY_mps2,2);
  Serial.print(" Z=");
  Serial.print(accelZ_mps2,2);
  Serial.print(" | Gyro (deg/s) X=");
  Serial.print(gyroX_dps,2);
  Serial.print(" Y=");
  Serial.print(gyroY_dps,2);
  Serial.print(" Z=");
  Serial.println(gyroZ_dps,2);

  delay(10); // 100 Hz
}

// ---------------------- Configuración MPU ----------------------
void setupMPU() {
  Wire.beginTransmission(0x68);
  Wire.write(0x6B); Wire.write(0x00); Wire.endTransmission(); // Wake up
  Wire.beginTransmission(0x68);
  Wire.write(0x1B); Wire.write(0x00); Wire.endTransmission(); // Gyro ±250°/s
  Wire.beginTransmission(0x68);
  Wire.write(0x1C); Wire.write(0x00); Wire.endTransmission(); // Accel ±2g
}

// ---------------------- Calibración ----------------------
void calibrateMPU() {
  long sumAx=0, sumAy=0, sumAz=0;
  long sumGx=0, sumGy=0, sumGz=0;
  const int N = 500;

  Serial.println("Calibrando MPU6050, mantenga el sensor quieto...");
  for(int i=0; i<N; i++){
    recordAccelRegisters();
    recordGyroRegisters();
    sumAx += accelX; sumAy += accelY; sumAz += accelZ;
    sumGx += gyroX; sumGy += gyroY; sumGz += gyroZ;
    delay(5);
  }

  accelX_offset = (sumAx / N) / ACC_SENS * G_TO_MPS2;
  accelY_offset = (sumAy / N) / ACC_SENS * G_TO_MPS2;
  accelZ_offset = ((sumAz / N) / ACC_SENS * G_TO_MPS2) - G_TO_MPS2; // Z compensa gravedad

  gyroX_offset = (sumGx / N) / GYRO_SENS;
  gyroY_offset = (sumGy / N) / GYRO_SENS;
  gyroZ_offset = (sumGz / N) / GYRO_SENS;

  Serial.println("Calibración completa.");
}

// ---------------------- Lectura acelerómetro ----------------------
void recordAccelRegisters() {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission();
  Wire.requestFrom(0x68,6);
  while(Wire.available()<6);
  accelX = (int16_t)(Wire.read()<<8 | Wire.read());
  accelY = (int16_t)(Wire.read()<<8 | Wire.read());
  accelZ = (int16_t)(Wire.read()<<8 | Wire.read());
}

void processAccelData() {
  accelX_mps2 = (accelX / ACC_SENS) * G_TO_MPS2 - accelX_offset;
  accelY_mps2 = (accelY / ACC_SENS) * G_TO_MPS2 - accelY_offset;
  accelZ_mps2 = (accelZ / ACC_SENS) * G_TO_MPS2 - accelZ_offset;
}

// ---------------------- Lectura giroscopio ----------------------
void recordGyroRegisters() {
  Wire.beginTransmission(0x68);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0x68,6);
  while(Wire.available()<6);
  gyroX = (int16_t)(Wire.read()<<8 | Wire.read());
  gyroY = (int16_t)(Wire.read()<<8 | Wire.read());
  gyroZ = (int16_t)(Wire.read()<<8 | Wire.read());
}

void processGyroData() {
  gyroX_dps = (gyroX / GYRO_SENS) - gyroX_offset;
  gyroY_dps = (gyroY / GYRO_SENS) - gyroY_offset;
  gyroZ_dps = (gyroZ / GYRO_SENS) - gyroZ_offset;
}



/*
CODIGO FUNCIONA BIEN

#include <Wire.h>

// Lecturas crudas
int16_t accelX, accelY, accelZ;
int16_t gyroX, gyroY, gyroZ;

// Lecturas físicas
float accelX_mps2, accelY_mps2, accelZ_mps2;
float gyroX_dps, gyroY_dps, gyroZ_dps;

// Offsets calculados en calibración
float accelX_offset = 0;
float accelY_offset = 0;
float accelZ_offset = 0;
float gyroX_offset = 0;
float gyroY_offset = 0;
float gyroZ_offset = 0;

// Constantes de conversión
const float ACC_SENS = 16384.0;   // ±2g
const float GYRO_SENS = 131.0;    // ±250°/s
const float G_TO_MPS2 = 9.80665;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  setupMPU();
  calibrateMPU();  // Calibración automática
  Serial.println("Inicio completo. Leyendo datos...");
}

void loop() {
  recordAccelRegisters();
  recordGyroRegisters();
  processAccelData();
  processGyroData();
  printData();
  delay(1000);  // ~100 Hz
}

// ---------------------- Configuración MPU ----------------------
void setupMPU() {
  // Wake up MPU
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  // Configurar giroscopio ±250°/s
  Wire.beginTransmission(0x68);
  Wire.write(0x1B);
  Wire.write(0x00);
  Wire.endTransmission();

  // Configurar acelerómetro ±2g
  Wire.beginTransmission(0x68);
  Wire.write(0x1C);
  Wire.write(0x00);
  Wire.endTransmission();
}

// ---------------------- Calibración ----------------------
void calibrateMPU() {
  long sumAx=0, sumAy=0, sumAz=0;
  long sumGx=0, sumGy=0, sumGz=0;
  const int N = 500;

  Serial.println("Calibrando MPU6050, mantenga el sensor quieto...");

  for(int i=0; i<N; i++){
    recordAccelRegisters();
    recordGyroRegisters();
    sumAx += accelX;
    sumAy += accelY;
    sumAz += accelZ;
    sumGx += gyroX;
    sumGy += gyroY;
    sumGz += gyroZ;
    delay(5);
  }

  // Convertir a unidades físicas
  accelX_offset = (sumAx / N) / ACC_SENS * G_TO_MPS2;
  accelY_offset = (sumAy / N) / ACC_SENS * G_TO_MPS2;
  accelZ_offset = ((sumAz / N) / ACC_SENS * G_TO_MPS2) - G_TO_MPS2; // Z compensa gravedad

  gyroX_offset = (sumGx / N) / GYRO_SENS;
  gyroY_offset = (sumGy / N) / GYRO_SENS;
  gyroZ_offset = (sumGz / N) / GYRO_SENS;

  Serial.println("Calibración completa.");
}

// ---------------------- Lectura acelerómetro ----------------------
void recordAccelRegisters() {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission();
  Wire.requestFrom(0x68,6);
  while(Wire.available() < 6);
  accelX = (int16_t)(Wire.read()<<8 | Wire.read());
  accelY = (int16_t)(Wire.read()<<8 | Wire.read());
  accelZ = (int16_t)(Wire.read()<<8 | Wire.read());
}

void processAccelData() {
  accelX_mps2 = (accelX / ACC_SENS) * G_TO_MPS2 - accelX_offset;
  accelY_mps2 = (accelY / ACC_SENS) * G_TO_MPS2 - accelY_offset;
  accelZ_mps2 = (accelZ / ACC_SENS) * G_TO_MPS2 - accelZ_offset;
}

// ---------------------- Lectura giroscopio ----------------------
void recordGyroRegisters() {
  Wire.beginTransmission(0x68);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0x68,6);
  while(Wire.available() < 6);
  gyroX = (int16_t)(Wire.read()<<8 | Wire.read());
  gyroY = (int16_t)(Wire.read()<<8 | Wire.read());
  gyroZ = (int16_t)(Wire.read()<<8 | Wire.read());
}

void processGyroData() {
  gyroX_dps = (gyroX / GYRO_SENS) - gyroX_offset;
  gyroY_dps = (gyroY / GYRO_SENS) - gyroY_offset;
  gyroZ_dps = (gyroZ / GYRO_SENS) - gyroZ_offset;
}

// ---------------------- Impresión de datos ----------------------
void printData() {
  Serial.print("Gyro (deg/s) X=");
  Serial.print(gyroX_dps,3);
  Serial.print(" Y=");
  Serial.print(gyroY_dps,3);
  Serial.print(" Z=");
  Serial.print(gyroZ_dps,3);

  Serial.print(" | Accel (m/s^2) X=");
  Serial.print(accelX_mps2,3);
  Serial.print(" Y=");
  Serial.print(accelY_mps2,3);
  Serial.print(" Z=");
  Serial.println(accelZ_mps2,3);
}



/*
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
}*/
// Librerias para IMU
#include <Wire.h>
#include <math.h>
#include <TinyGPS++.h>  // Libreria para GPS

// Pines para IMU
#define IMU_RX_PIN 22
#define IMU_TX_PIN 21
// Pines para GPS
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17

TinyGPSPlus gps;
HardwareSerial SerialGPS(1);

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

// Ángulos
float pitch = 0;
float roll = 0;
float yaw = 0;

// Control de tiempos sin delay()
unsigned long lastImuTime = 0;
unsigned long lastPrintTime = 0;
const int printInterval = 100; // Imprimir a 10 Hz (cada 100ms)

void setup() {
  Serial.begin(115200);
  
  // Configurar I2C a 400kHz para no ralentizar el procesador
  Wire.begin();
  Wire.setClock(400000); 

  // Calibración del IMU (GY-87)
  setupMPU();
  calibrateMPU();

  // Configurar serial GPS con un buffer más amplio
  SerialGPS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  SerialGPS.setRxBufferSize(1024); // Aumentar buffer para prevenir pérdida de datos
}

void loop() {
  // 1. Leer GPS en CADA iteración del loop (Máxima prioridad para evitar desbordamiento)
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }

  // 2. Ejecutar lectura e integración de IMU a exactamente 100 Hz (cada 10ms)
  unsigned long currentMicros = micros();
  if (currentMicros - lastImuTime >= 10000) { 
    lastImuTime = currentMicros;

    recordAccelRegisters();
    recordGyroRegisters();
    processAccelData();
    processGyroData();

    // Filtro complementario
    float pitchAcc = atan2(accelY_mps2, sqrt(accelX_mps2*accelX_mps2 + accelZ_mps2*accelZ_mps2)) * 180.0 / PI;
    float rollAcc  = atan2(-accelX_mps2, accelZ_mps2) * 180.0 / PI;

    pitch = alpha * (pitch + gyroX_dps * dt) + (1 - alpha) * pitchAcc;
    roll  = alpha * (roll + gyroY_dps * dt)  + (1 - alpha) * rollAcc;

    yaw = yaw + gyroZ_dps * dt; 
    if (yaw >= 180.0) yaw -= 360.0;
    else if (yaw < -180.0) yaw += 360.0;
  }

  // 3. Imprimir solo cuando el GPS esté registrado (isValid)
  if (millis() - lastPrintTime >= printInterval) {
    lastPrintTime = millis();

    // Solo se muestra en pantalla si el GPS está registrando posición
    if (gps.location.isValid()) {
      Serial.print(pitch, 4);       Serial.print(",");
      Serial.print(roll, 4);        Serial.print(",");
      Serial.print(yaw, 4);         Serial.print(",");
      Serial.print(accelX_mps2, 4); Serial.print(",");
      Serial.print(accelY_mps2, 4); Serial.print(",");
      Serial.print(accelZ_mps2, 4); Serial.print(",");
      Serial.print(gyroX_dps, 4);   Serial.print(",");
      Serial.print(gyroY_dps, 4);   Serial.print(",");
      Serial.print(gyroZ_dps, 4);   Serial.print(",");

      Serial.print(gps.location.lat(), 6); Serial.print(",");
      Serial.print(gps.location.lng(), 6); Serial.print(",");
      Serial.println(gps.altitude.meters(), 2);
    }
  }
}

// ---------------------- Configuración MPU ----------------------
void setupMPU() {
  Wire.beginTransmission(0x68);
  Wire.write(0x6B); Wire.write(0x00); Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x1B); Wire.write(0x00); Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x1C); Wire.write(0x00); Wire.endTransmission();
}

// ---------------------- Calibración ----------------------
void calibrateMPU() {
  long sumAx=0, sumAy=0, sumAz=0;
  long sumGx=0, sumGy=0, sumGz=0;
  const int N = 500;

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
}

// ---------------------- Lectura acelerómetro seguras ----------------------
void recordAccelRegisters() {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false); // Mantener el bus activo sin soltarlo
  Wire.requestFrom(0x68, 6);
  if (Wire.available() >= 6) {
    accelX = (int16_t)(Wire.read()<<8 | Wire.read());
    accelY = (int16_t)(Wire.read()<<8 | Wire.read());
    accelZ = (int16_t)(Wire.read()<<8 | Wire.read());
  }
}

void processAccelData() {
  accelX_mps2 = (accelX / ACC_SENS) * G_TO_MPS2 - accelX_offset;
  accelY_mps2 = (accelY / ACC_SENS) * G_TO_MPS2 - accelY_offset;
  accelZ_mps2 = (accelZ / ACC_SENS) * G_TO_MPS2 - accelZ_offset;
}

// ---------------------- Lectura giroscopio seguras ----------------------
void recordGyroRegisters() {
  Wire.beginTransmission(0x68);
  Wire.write(0x43);
  Wire.endTransmission(false); // Mantener el bus activo sin soltarlo
  Wire.requestFrom(0x68, 6);
  if (Wire.available() >= 6) {
    gyroX = (int16_t)(Wire.read()<<8 | Wire.read());
    gyroY = (int16_t)(Wire.read()<<8 | Wire.read());
    gyroZ = (int16_t)(Wire.read()<<8 | Wire.read());
  }
}

void processGyroData() {
  gyroX_dps = (gyroX / GYRO_SENS) - gyroX_offset;
  gyroY_dps = (gyroY / GYRO_SENS) - gyroY_offset;
  gyroZ_dps = (gyroZ / GYRO_SENS) - gyroZ_offset;
}
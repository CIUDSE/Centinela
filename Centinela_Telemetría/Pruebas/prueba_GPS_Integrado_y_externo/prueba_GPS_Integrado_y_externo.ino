//Codigo de prueba para usar 2 gps en placa T-Beam esp32 lora junto al MPU.
#include <Wire.h>
#include <TinyGPS++.h>
#include <XPowersLib.h>

/* =====================================================
   CONFIGURACIÓN GPS
   ===================================================== */
// GPS 1 (UART1)
#define GPS1_RX_PIN 34
#define GPS1_TX_PIN 12
#define GPS1_BAUD   9600

// GPS 2 (UART2)
#define GPS2_RX_PIN 35
#define GPS2_TX_PIN 33
#define GPS2_BAUD   9600

/* =====================================================
   CONFIGURACIÓN I2C
   ===================================================== */
#define I2C_SDA 21
#define I2C_SCL 22

/* =====================================================
   OBJETOS
   ===================================================== */
TinyGPSPlus gps1;
TinyGPSPlus gps2;

HardwareSerial SerialGPS1(1);
HardwareSerial SerialGPS2(2);

XPowersAXP2101 *PMU = nullptr;

/* =====================================================
   MPU6050 VARIABLES
   ===================================================== */
#define MPU_ADDR 0x68

int16_t accelX, accelY, accelZ;
int16_t gyroX, gyroY, gyroZ;

float gForceX, gForceY, gForceZ;
float rotX, rotY, rotZ;

/* =====================================================
   PMU AXP2101
   ===================================================== */
void setupPMU()
{
  PMU = new XPowersAXP2101(Wire);

  if (!PMU->init()) {
    Serial.println("❌ No se encontró PMU AXP2101");
    delete PMU;
    PMU = nullptr;
    return;
  }

  Serial.println("✅ PMU AXP2101 inicializado");

  // Alimentar GPS1 desde ALDO3
  PMU->setALDO3Voltage(3300);
  PMU->enableALDO3();

  Serial.println("🔌 ALDO3 (3.3V) encendido para GPS1");
}

/* =====================================================
   MPU6050
   ===================================================== */
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

void readAccel()
{
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

void readGyro()
{
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

void processAccelData(){
  gForceX = (accelX / 16384.0)*9.80665;
  gForceY = (accelY / 16384.0)*9.80665; 
  gForceZ = (accelZ / 16384.0)*9.80665;
}

void processGyroData() {
  rotX = gyroX / 131.0;
  rotY = gyroY / 131.0; 
  rotZ = gyroZ / 131.0;
}

/* =====================================================
   SETUP
   ===================================================== */
void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== Iniciando sistema ===");

  // 🔑 UN SOLO BUS I2C
  Wire.begin(I2C_SDA, I2C_SCL, 400000);

  setupPMU();
  setupMPU();

  SerialGPS1.begin(GPS1_BAUD, SERIAL_8N1, GPS1_RX_PIN, GPS1_TX_PIN);
  SerialGPS2.begin(GPS2_BAUD, SERIAL_8N1, GPS2_RX_PIN, GPS2_TX_PIN);

  Serial.println("📡 GPS 1 y GPS 2 iniciados");
}

/* =====================================================
   LOOP
   ===================================================== */
void loop()
{
  /* ---------- GPS ---------- */
  while (SerialGPS1.available()) gps1.encode(SerialGPS1.read());
  while (SerialGPS2.available()) gps2.encode(SerialGPS2.read());

  /* ---------- MPU ---------- */
  readAccel();
  readGyro();

  /* ---------- OUTPUT ---------- */
  if (gps1.location.isUpdated()) {
    Serial.println("\n📍 GPS 1");
    Serial.println(gps1.location.lat(), 6);
    Serial.println(gps1.location.lng(), 6);
  }

  if (gps2.location.isUpdated()) {
    Serial.println("\n📍 GPS 2");
    Serial.println(gps2.location.lat(), 6);
    Serial.println(gps2.location.lng(), 6);
  }

  Serial.println("\n🧭 MPU6050");
  Serial.print("Accel (m/s²): ");
  Serial.print(gForceX); Serial.print(", ");
  Serial.print(gForceY); Serial.print(", ");
  Serial.println(gForceZ);

  Serial.print("Gyro (deg/s): ");
  Serial.print(rotX); Serial.print(", ");
  Serial.print(rotY); Serial.print(", ");
  Serial.println(rotZ);

  delay(500);
}

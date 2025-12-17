#include <Wire.h>
#include <TinyGPS++.h>
#include <XPowersLib.h>

#define GPS_RX_PIN 34
#define GPS_TX_PIN 12
#define GPS_BAUD   9600

#define I2C_SDA 21
#define I2C_SCL 22

XPowersAXP2101 *PMU = nullptr;
TinyGPSPlus gps;
HardwareSerial SerialGPS(1);

void setupPMU()
{
  Wire1.begin(I2C_SDA, I2C_SCL, 400000);

  PMU = new XPowersAXP2101(Wire1);

  if (!PMU->init()) {
    Serial.println(" No se encontró PMU AXP2101");
    delete PMU;
    PMU = nullptr;
    return;
  }

  Serial.println(" PMU AXP2101 inicializado");

  // ---------------------------
  // Encender el GPS (ALDO3 3.3V)
  // ---------------------------
  PMU->setALDO3Voltage(3300);   // Ajustar 3.3V
  PMU->enableALDO3();           // Encender salida ALDO3

  Serial.print("ALDO3 State: ");
Serial.println(PMU->isEnableALDO3() ? "ON" : "OFF");

Serial.print("ALDO3 Voltage: ");
Serial.println(PMU->getALDO3Voltage()); 

  Serial.println(" GPS alimentado desde ALDO3 (3.3V)");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== Iniciando PMU ===");
  setupPMU();

  Serial.println("\n=== Iniciando GPS ===");
  SerialGPS.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  Serial.println("GPS listo. Esperando datos...\n");
}

void loop() {

  while (SerialGPS.available()) {
    gps.encode(SerialGPS.read());
  }

  if (gps.satellites.isUpdated()) {
    Serial.print("Satélites: ");
    Serial.println(gps.satellites.value());
  }

  if (gps.location.isUpdated()) {
    Serial.print("Latitud: ");
    Serial.println(gps.location.lat(), 6);

    Serial.print("Longitud: ");
    Serial.println(gps.location.lng(), 6);
  }

  delay(200);
}

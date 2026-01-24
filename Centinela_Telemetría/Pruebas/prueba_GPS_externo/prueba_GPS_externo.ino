#include <TinyGPS++.h>

// Pines UART2 (los que tú dijiste)
#define RX_PIN 35   // RX del ESP32 ← TX del GPS (solo entrada, OK)
#define TX_PIN 33   // TX del ESP32 → RX del GPS
#define GPS_BAUD 9600

TinyGPSPlus gps;

// Usamos UART2
HardwareSerial gpsSerial(2);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Inicializar UART2 con los pines indicados
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial.println("GPS iniciado en UART2 (RX=35, TX=33)"); //Es decir que el pin 35 va al tx del gps, y el pin 33 va al rx del gps.
}

void loop() {
  // Leer datos crudos del GPS
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Mostrar datos cuando haya nueva posición válida
  if (gps.location.isUpdated()) {

    Serial.print("LAT: ");
    Serial.println(gps.location.lat(), 6);

    Serial.print("LON: ");
    Serial.println(gps.location.lng(), 6);

    Serial.print("Velocidad (km/h): ");
    Serial.println(gps.speed.kmph());

    Serial.print("Altitud (m): ");
    Serial.println(gps.altitude.meters());

    Serial.print("HDOP: ");
    Serial.println(gps.hdop.hdop());

    Serial.print("Satélites: ");
    Serial.println(gps.satellites.value());

    Serial.print("Fecha UTC: ");
    Serial.print(gps.date.year());
    Serial.print("/");
    Serial.print(gps.date.month());
    Serial.print("/");
    Serial.println(gps.date.day());

    Serial.print("Hora UTC: ");
    Serial.print(gps.time.hour());
    Serial.print(":");
    Serial.print(gps.time.minute());
    Serial.print(":");
    Serial.println(gps.time.second());

    Serial.println("--------------------------------");
  }
}

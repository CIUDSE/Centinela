/***************************************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Código RECEPTOR LORA (CON PANTALLA OLED) para telemetría general del rover Centinela implementando una LilyGo TTGO T-Beam V1.2.
**************************************************************************************************************************************************/
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DS18B20_CANTIDAD 10

// --------CONFIGURACIÓN DE PINES-------------------------------------
// Pines de LoRa (Chip SX1276)
#define LORA_SCK 5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_SS 18
#define LORA_RST 23
#define LORA_DIO0 26

// Pines de la pantalla OLED (I2C)
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 16  // Pin de reset

// Frecuencia de LoRa - DEBE COINCIDIR CON EL EMISOR - 433E6 (Asia), 868E6 (Europa), 915E6 (Norteamérica)
#define BAND 915E6
#define SPREADING_FACTOR 7
#define BANDWIDTH 500E3

// Tamaño de la pantalla
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Pantalla
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//--------------------------- Estructura de datos de sensores ---------------------------//
typedef struct __attribute__((packed)) 
{
  char id;
  uint16_t tiempoRecibido;
  uint16_t numPaquete;
  
  //GY87
  int16_t accel_x_16 = 0.0;           //Valores en m/s^2
  int16_t accel_y_16 = 0.0;
  int16_t accel_z_16 = 0.0;
  int32_t vel_ang_x_32 = 0.0;         //Valores en °/s
  int32_t vel_ang_y_32 = 0.0;
  int32_t vel_ang_z_32 = 0.0;
  
  //GPS1 (GPS integrado en placa T-Beam)
  int32_t lat1_32 = 0.0;
  int32_t lon1_32 = 0.0;

  //GPS2 (GPS externo Neo6m)
  int32_t lat2_32 = 0.0;
  int32_t lon2_32 = 0.0;

  //DS18B20 (Sensores de temperatura)
  int16_t temp_16[DS18B20_CANTIDAD];
} telemetryData_t;

telemetryData_t telemetryData;

//------------------------------ Setup ----------------------------------//
void setup() {
  Serial.begin(115200);
  Serial.println(BAND);
  // 1. Inicializar OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Fallo OLED");
    while (1)
      ;
  }

  // 2. Config pantalla
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("Esperando datos...");
  display.display();

  // 3. Inicializar LoRa
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  // 4. Chequeo
  if (!LoRa.begin(BAND)) {
    Serial.println("Fallo LoRa");
    while (1)
      ;
  }

  // Asignar palabra de sincronización (debe ser la misma en TX y RX)
  LoRa.setSyncWord(0xA0);

  // ESTAS LÍNEAS SON IMPORTANTES PARA LA LATENCIA
  LoRa.setSpreadingFactor(SPREADING_FACTOR);      
  LoRa.setSignalBandwidth(BANDWIDTH);  
  LoRa.setCodingRate4(5);
  Serial.println("LoRa iniciado");
}

//--------------------------- Loop ---------------------------//
void loop() {
  int packetSize = LoRa.parsePacket();
  //Comprobrar que lo recibido sea igual a los datos de la telemetria
  if (packetSize == sizeof(telemetryData_t)) 
  {
    uint8_t buffer[sizeof(telemetryData_t)];

    for (int i = 0; i < sizeof(buffer); i++) 
    {  //Recibir todos los datos a la variable buffer
      buffer[i] = LoRa.read();
    }

    memcpy(&telemetryData, buffer, sizeof(buffer));  //Copiar los datos guardados en buffer a telemetryData

    int rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();

    //Conversión de datos 
    double lat1 = telemetryData.lat1_32 / 1e6;
    double lon1 = telemetryData.lon1_32 / 1e6;

    double lat2 = telemetryData.lat2_32 / 1e6;
    double lon2 = telemetryData.lon2_32 / 1e6;
    
    double vel_ang_x = telemetryData.vel_ang_x_32 / 100.0;
    double vel_ang_y = telemetryData.vel_ang_y_32 / 100.0;
    double vel_ang_z = telemetryData.vel_ang_z_32 / 100;
    float accel_x = telemetryData.accel_x_16 / 100.0;
    float accel_y = telemetryData.accel_y_16 / 100.0;
    float accel_z = telemetryData.accel_z_16 / 100.0; 

    float temp[DS18B20_CANTIDAD];
    for(int i=0; i<DS18B20_CANTIDAD; i++)
    {
      temp[i]= telemetryData.temp_16[i]/100.0;
    }

    char datosCSV[300];

    sprintf(datosCSV,
            "$%c,%u,%u,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.6f,%.6f,%.6f,%.6f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
            telemetryData.id,
            telemetryData.tiempoRecibido,
            telemetryData.numPaquete,
            rssi,
            vel_ang_x,
            vel_ang_y,
            vel_ang_z,
            accel_x,
            accel_y,
            accel_z,
            lat1,
            lon1,
            lat2,
            lon2,
            temp[0],
            temp[1],
            temp[2],
            temp[3],
            temp[4],
            temp[5],
            temp[6],
            temp[7],
            temp[8],
            temp[9],
            temp[10]
            );
    Serial.print(datosCSV);

    // //Imprimir en el Monitor Serial
    // Serial.print("\n-> MENSAJE: ");
    // Serial.print("latitud: ");
    // Serial.println(telemetriaActual.latitud, 6);
    // Serial.print("longitud: ");
    // Serial.println(telemetriaActual.longitud, 6);
    // // Serial.print("altura: ");
    // // Serial.println(telemetriaActual.altura_gps,2);
    // Serial.print("alturaBar: ");
    // Serial.println(telemetriaActual.altura_bar, 3);
    // Serial.print("temperatura: ");
    // Serial.println(telemetriaActual.temperatura, 3);
    // Serial.print("Presion:");
    // Serial.println(telemetriaActual.presion, 3);
    // Serial.print("   (RSSI: ");
    // Serial.print(rssi);
    // Serial.print(" dBm | SNR: ");
    // Serial.print(snr);
    // Serial.println(")");

    // 3. Mostrar en la pantalla OLED
    updateDisplay(rssi, snr, lat1, lon1, temp[0]);
  }
}

// ------------------------------------------------------------------

// Función para actualizar la pantalla con los datos recibidos
void updateDisplay(int rssi, float snr, float lat1, float lon1, float temp) 
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  // Encabezado
  display.println("<< DATOS RECIBIDOS >>");
  display.drawFastHLine(0, 9, 128, SSD1306_WHITE);  // Línea divisoria

  // Datos
  display.setTextSize(0.5);  // Texto más grande para el mensaje
  display.setCursor(0, 15);
  //Mostrar todos los datos obtenidos
  display.print("latitud: ");
  display.println(lat1, 6);
  display.print("longitud: ");
  display.println(lon1, 6);
  //display.print("latitud 2: ");
  //display.println(lat2, 3);
  // display.print("temperatura: ");
  // display.println(temp, 3);
  // display.print("Presion:");
  // display.println(pres, 3);

  //Métricas (Parte inferior)
  display.setTextSize(0.5);
  display.setCursor(0, 48);
  display.print("RSSI: ");
  display.print(rssi);
  display.println(" dBm");

  display.setCursor(0, 56);
  display.print("SNR: ");
  display.print(snr, 1);  // 1 decimal
  display.println(" dB");

  display.display();  // Muestra el buffer en la pantalla física
}
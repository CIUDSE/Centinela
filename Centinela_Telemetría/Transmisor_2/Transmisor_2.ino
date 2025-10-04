// LILYGO LORA TRANSMISOR

#include <SPI.h>
#include <LoRa.h>

// --------CONFIGURACIÓN DE PINES-------------------------------------
// Definición de pines para la placa Heltec WiFi LoRa 32
#define SS      18  
#define RST     14  
#define DIO0    26  
#define SCK      5
#define MISO    19
#define MOSI    27

// Frecuencia de LoRa - DEBE COINCIDIR CON EL EMISOR - 433E6 (Asia), 868E6 (Europa), 915E6 (Norteamérica)
#define BAND    915E6 

// Declarar el contador para comprobar la conexión
long contador = 0;

// -------------------------------------------------------------------

void setup() 
{
  Serial.begin(115200);
  while (!Serial);

  SPI.begin(SCK, MISO, MOSI, SS);

  // 2. Configurar pines
  LoRa.setPins(SS, RST, DIO0);

  // 3. Inicializar el módulo LoRa
  if (!LoRa.begin(BAND)) 
  {
    Serial.println("Fallo al iniciar LoRa!");
    while (1);
  }

  // Asignar palabra de sincronización (debe ser la misma en TX y RX)
  LoRa.setSyncWord(0xA0); 

  // ESTAS LÍNEAS SON IMPORTANTES PARA LA LATENCIA 
  LoRa.setSpreadingFactor(7);     // Mínimo SF para máx velocidad
  LoRa.setSignalBandwidth(500E3); // Máximo BW para máx velocidad
  
  Serial.println("LoRa iniciado!");
}

void loop() 
{
  // Texto en pantalla
  Serial.print("Enviando paquete: ");
  Serial.println(contador);

  // 1. Iniciar el paquete de LoRa
  LoRa.beginPacket();

  // 2. Escribir el contenido del mensaje
  LoRa.print("Hola LoRa, contador: ");
  LoRa.print(contador);

  // 3. Finalizar el paquete y enviarlo
  LoRa.endPacket();

  contador++; // Incrementar el contador
  
  delay(1000); // Espera de 1 segundo antes del próximo envío (el tiempo es solo para prueba)
}
//Codigo de pruebas para sensor DS18B20
//Prueba con LilyGo lora32 y un solo sensor DS18B20

#include <OneWire.h>
#include <DallasTemperature.h>

OneWire ourWire(4);                   // Pin 4 como bus OneWire 
DallasTemperature sensors(&ourWire);  // Objeto para los sensores

void setup() {
  Serial.begin(115200);
  sensors.begin();                   // Inicializa los sensores
}

void loop() {
  sensors.requestTemperatures(); // Solicita temperaturas
  float temp= sensors.getTempCByIndex(0); //Se obtiene la temperatura en ºC

  Serial.print("Temperatura= ");
  Serial.print(temp);
  Serial.println(" C");
  delay(100);              
}


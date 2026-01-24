//Codigo para conocer las direcciones de los sesnres DS18B20

#include <OneWire.h>

OneWire ourWire(4);                //Se establece el pin 4  como bus OneWire

void setup(void) {
  Serial.begin(115200);
}

void loop(void) {
  byte addr[8];  
  Serial.println("Obteniendo direcciones:");
  while (ourWire.search(addr)) 
  {  
  Serial.print("Address = ");
  for( int i = 0; i < 8; i++) {
    Serial.print(" 0x");
    Serial.print(addr[i], HEX);
  }
  Serial.println();
}

Serial.println();
ourWire.reset_search();
delay(2000);
}
//Codigo de prueba para dos sensores DS18B20
//Se debe tomar en cuenta que se deben reemplazar la direcciones en el sketch con las direcciones correspondientes a los sensores que se tenga disponibles

#include <OneWire.h>                
#include <DallasTemperature.h>
 
OneWire ourWire(4);                //Se establece el pin 4  como bus OneWire
 
DallasTemperature sensors(&ourWire); //Se declara una variable u objeto para nuestro sensor

DeviceAddress address1 = {0x28, 0x61, 0x64, 0x35, 0xC6, 0x10, 0xBF, 0x30};//dirección del sensor 1
DeviceAddress address2 = {0x28, 0x61, 0x64, 0x35, 0xC1, 0x40, 0x7E, 0xA9};//dirección del sensor 2

void setup() 
{
  delay(1000);
  Serial.begin(115200);
  sensors.begin();   //Se inicia el sensor

  sensors.setResolution(address1, 9);
  sensors.setResolution(address2, 9);
}
 
void loop() 
{
  sensors.requestTemperatures();   //envía el comando para obtener las temperaturas
  float temp1= sensors.getTempC(address1);//Se obtiene la temperatura en °C del sensor 1
  float temp2= sensors.getTempC(address2);//Se obtiene la temperatura en °C del sensor 2

  Serial.print("Temperatura 1 = ");
  Serial.print(temp1);
  Serial.print(" C");
  Serial.print("   Temperatura 2 = ");
  Serial.print(temp2);
  Serial.println(" C");
    
  delay(100);                     
}
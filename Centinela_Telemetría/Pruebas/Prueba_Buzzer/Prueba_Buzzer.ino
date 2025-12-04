//Código de prueba de buzzer usando T-Beam Esp32 LoRa

int buzzer = 32;

void setup() 
{
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(200);
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(200);

}

void loop() 
{
  digitalWrite(buzzer, HIGH);
  delay(1000);
  digitalWrite(buzzer, LOW);
  delay(1000);

}

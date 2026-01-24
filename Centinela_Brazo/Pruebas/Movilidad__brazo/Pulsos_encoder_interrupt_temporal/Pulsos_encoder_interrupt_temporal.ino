const int pinA = 13;
const int pinB = 14;
volatile long contador = 0;
volatile int ultimoEstadoA = 0;

void IRAM_ATTR interrupcion();

void setup() {
  Serial.begin(115200);
  delay(2000); 
  
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
 
  ultimoEstadoA = digitalRead(pinA);
  Serial.print("Estado inicial pinA: ");
  Serial.println(ultimoEstadoA);
  attachInterrupt(digitalPinToInterrupt(pinA), interrupcion, CHANGE);

}

void loop() {
  static unsigned long lastPrint = 0;
  
  if (millis() - lastPrint > 500) {
    Serial.print("Contador: ");
    Serial.println(contador);
    lastPrint = millis();
  }
}

void IRAM_ATTR interrupcion() {
  int estadoA = digitalRead(pinA);
  int estadoB = digitalRead(pinB);
  
  if (estadoA != ultimoEstadoA) {
    if (estadoB != estadoA) {
      contador++;
    } else {
      contador--;
    }
  }
  ultimoEstadoA = estadoA;
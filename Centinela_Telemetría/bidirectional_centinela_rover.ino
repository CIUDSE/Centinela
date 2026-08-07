/*
 * CENTINELA - Firmware del ROVER
 * - Envía telemetría periódicamente (no bloqueante)
 * - Escucha comandos LoRa entrantes (solenoide directo / secuencia morse)
 * - Responde con ACK por cada comando recibido
 * - Reproduce Morse en el solenoide sin usar delay() (no bloqueante)
 *
 * Librería: arduino-LoRa (sandeepmistry) - ajusta pines a tu hardware (T-Beam / T3 LoRa32)
 */

#include <SPI.h>
#include <LoRa.h>

// ---------- Pines (ajusta a tu placa) ----------
#define LORA_SS   18
#define LORA_RST  14
#define LORA_DIO0 26
#define SOLENOID_PIN 25

// ---------- Protocolo ----------
enum PacketType : uint8_t {
  PKT_TELEMETRY = 0x01,
  PKT_SOLENOID  = 0x02,
  PKT_MORSE     = 0x03,
  PKT_ACK       = 0x04
};

// ---------- Telemetría (no bloqueante) ----------
unsigned long lastTelemetryMs = 0;
const unsigned long TELEMETRY_INTERVAL_MS = 1000; // ajusta a tu tasa actual

// ---------- Deduplicación de comandos ----------
// Evita re-ejecutar un comando si el ground station reenvía por no recibir el ACK a tiempo
uint8_t lastProcessedSeq = 0xFF; // valor imposible inicial
bool haveProcessedAny = false;

// ---------- Tabla de Morse ----------
const char* morseLetters[26] = {
  ".-","-...","-.-.","-..",".","..-.","--.","....","..",".---","-.-",".-..","--",
  "-.","---",".--.","--.-",".-.","...","-","..-","...-",".--","-..-","-.--","--.."
};
const char* morseDigits[10] = {
  "-----",".----","..---","...--","....-",".....","-....","--...","---..","----."
};

const char* charToMorse(char c) {
  c = toupper(c);
  if (c >= 'A' && c <= 'Z') return morseLetters[c - 'A'];
  if (c >= '0' && c <= '9') return morseDigits[c - '0'];
  return nullptr; // desconocido (se salta)
}

// ---------- Reproductor de Morse no bloqueante ----------
struct MorsePlayer {
  char text[33] = {0};
  uint16_t unitMs = 100;
  int textIndex = 0;
  const char* currentCode = nullptr;
  int symbolIndex = 0;
  enum State { IDLE, SYMBOL_ON, SYMBOL_OFF, LETTER_GAP, WORD_GAP, DONE } state = DONE;
  unsigned long stateStart = 0;
} morsePlayer;

void startMorse(const char* txt, uint16_t unit) {
  strncpy(morsePlayer.text, txt, sizeof(morsePlayer.text) - 1);
  morsePlayer.text[sizeof(morsePlayer.text) - 1] = '\0';
  morsePlayer.unitMs = unit;
  morsePlayer.textIndex = 0;
  morsePlayer.currentCode = nullptr;
  morsePlayer.symbolIndex = 0;
  morsePlayer.state = MorsePlayer::IDLE;
  morsePlayer.stateStart = millis();
}

void updateMorsePlayer() {
  if (morsePlayer.state == MorsePlayer::DONE) return;
  unsigned long now = millis();
  unsigned long elapsed = now - morsePlayer.stateStart;

  switch (morsePlayer.state) {
    case MorsePlayer::IDLE: {
      char c = morsePlayer.text[morsePlayer.textIndex];
      if (c == '\0') {
        digitalWrite(SOLENOID_PIN, LOW);
        morsePlayer.state = MorsePlayer::DONE;
        return;
      }
      if (c == ' ') {
        morsePlayer.textIndex++;
        morsePlayer.state = MorsePlayer::WORD_GAP;
        morsePlayer.stateStart = now;
        return;
      }
      morsePlayer.currentCode = charToMorse(c);
      if (morsePlayer.currentCode == nullptr) {
        morsePlayer.textIndex++; // caracter desconocido, se salta
        return;
      }
      morsePlayer.symbolIndex = 0;
      digitalWrite(SOLENOID_PIN, HIGH);
      morsePlayer.state = MorsePlayer::SYMBOL_ON;
      morsePlayer.stateStart = now;
      return;
    }
    case MorsePlayer::SYMBOL_ON: {
      char sym = morsePlayer.currentCode[morsePlayer.symbolIndex];
      unsigned long dur = (sym == '.') ? morsePlayer.unitMs : (morsePlayer.unitMs * 3UL);
      if (elapsed >= dur) {
        digitalWrite(SOLENOID_PIN, LOW);
        morsePlayer.state = MorsePlayer::SYMBOL_OFF;
        morsePlayer.stateStart = now;
      }
      return;
    }
    case MorsePlayer::SYMBOL_OFF: {
      if (elapsed >= morsePlayer.unitMs) {
        morsePlayer.symbolIndex++;
        if (morsePlayer.currentCode[morsePlayer.symbolIndex] == '\0') {
          morsePlayer.textIndex++;
          morsePlayer.state = MorsePlayer::LETTER_GAP; // ya se gastó 1 unidad como SYMBOL_OFF
          morsePlayer.stateStart = now;
        } else {
          digitalWrite(SOLENOID_PIN, HIGH);
          morsePlayer.state = MorsePlayer::SYMBOL_ON;
          morsePlayer.stateStart = now;
        }
      }
      return;
    }
    case MorsePlayer::LETTER_GAP: {
      // gap total entre letras = 3 unidades (1 ya cubierta en SYMBOL_OFF)
      if (elapsed >= morsePlayer.unitMs * 2UL) {
        morsePlayer.state = MorsePlayer::IDLE;
      }
      return;
    }
    case MorsePlayer::WORD_GAP: {
      // gap total entre palabras = 7 unidades
      if (elapsed >= morsePlayer.unitMs * 7UL) {
        morsePlayer.state = MorsePlayer::IDLE;
      }
      return;
    }
    default: return;
  }
}

// ---------- Envío de telemetría (ajusta al struct que ya usas) ----------
void sendTelemetry() {
  LoRa.beginPacket();
  LoRa.write(PKT_TELEMETRY);
  // TODO: escribe aquí tu struct telemetryData existente (accelX/Y/Z, GPS, temps, etc.)
  LoRa.endPacket();
  LoRa.receive(); // vuelve a modo escucha continua
}

// ---------- ACK ----------
void sendAck(uint8_t seq) {
  LoRa.beginPacket();
  LoRa.write(PKT_ACK);
  LoRa.write(seq);
  LoRa.endPacket();
  LoRa.receive();
}

// ---------- Manejo de paquetes entrantes ----------
void handleIncomingPacket(int packetSize) {
  if (packetSize <= 0) return;

  uint8_t type = LoRa.read();

  if (type == PKT_SOLENOID) {
    uint8_t seq = LoRa.read();
    uint8_t state = LoRa.read();

    if (!haveProcessedAny || seq != lastProcessedSeq) {
      digitalWrite(SOLENOID_PIN, state ? HIGH : LOW);
      lastProcessedSeq = seq;
      haveProcessedAny = true;
    }
    sendAck(seq); // se re-envía ACK aunque sea un comando repetido (por si se perdió el ACK anterior)
  }
  else if (type == PKT_MORSE) {
    uint8_t seq = LoRa.read();
    char buf[33];
    int i = 0;
    while (LoRa.available() && i < 32) buf[i++] = (char)LoRa.read();
    buf[i] = '\0';
    uint16_t unitMs = 100;
    if (LoRa.available() >= 2) {
      unitMs = (uint16_t)LoRa.read() | ((uint16_t)LoRa.read() << 8);
    }

    if (!haveProcessedAny || seq != lastProcessedSeq) {
      startMorse(buf, unitMs);
      lastProcessedSeq = seq;
      haveProcessedAny = true;
    }
    sendAck(seq);
  }
  // PKT_TELEMETRY / PKT_ACK entrantes no aplican en el rover, se ignoran
}

void setup() {
  Serial.begin(115200);
  pinMode(SOLENOID_PIN, OUTPUT);
  digitalWrite(SOLENOID_PIN, LOW);

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(915E6)) {
    Serial.println("Fallo al iniciar LoRa");
    while (true) delay(1000);
  }
  LoRa.receive();
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize > 0) {
    handleIncomingPacket(packetSize);
  }

  unsigned long now = millis();
  if (now - lastTelemetryMs >= TELEMETRY_INTERVAL_MS) {
    lastTelemetryMs = now;
    sendTelemetry();
  }

  updateMorsePlayer();
}

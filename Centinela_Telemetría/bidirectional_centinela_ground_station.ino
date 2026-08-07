/*
 * CENTINELA - Firmware de la ESTACION TERRENA
 * - Recibe comandos por Serial desde tu app.py / Flask, por ejemplo:
 *     CMD:SOLENOID:1
 *     CMD:SOLENOID:0
 *     CMD:MORSE:SOS:150        (texto:unidad_ms)
 * - Los transmite por LoRa con un numero de secuencia
 * - Espera ACK del rover; si no llega, reintenta (no bloqueante)
 * - Reporta a Serial: OK:<seq> / FAIL:<seq> / TIMEOUT-RETRY:<seq>
 * - Sigue recibiendo telemetria del rover en paralelo (passthrough)
 *
 * Libreria: arduino-LoRa (sandeepmistry)
 */

#include <SPI.h>
#include <LoRa.h>

#define LORA_SS   18
#define LORA_RST  14
#define LORA_DIO0 26

enum PacketType : uint8_t {
  PKT_TELEMETRY = 0x01,
  PKT_SOLENOID  = 0x02,
  PKT_MORSE     = 0x03,
  PKT_ACK       = 0x04
};

// ---------- Cola de comando pendiente (uno a la vez) ----------
struct PendingCommand {
  bool active = false;
  uint8_t seq = 0;
  uint8_t type = 0;       // PKT_SOLENOID o PKT_MORSE
  uint8_t solenoidState = 0;
  char morseText[33] = {0};
  uint16_t morseUnit = 100;
  unsigned long sentAt = 0;
  uint8_t retries = 0;
} pending;

const unsigned long ACK_TIMEOUT_MS = 400;   // ajusta segun tu SF/BW de LoRa
const uint8_t MAX_RETRIES = 4;

uint8_t nextSeq = 0;

// ---------- Buffer de lectura de Serial ----------
String serialLine;

// ---------- Envio de paquetes ----------
void transmitPending() {
  LoRa.beginPacket();
  if (pending.type == PKT_SOLENOID) {
    LoRa.write(PKT_SOLENOID);
    LoRa.write(pending.seq);
    LoRa.write(pending.solenoidState);
  } else if (pending.type == PKT_MORSE) {
    LoRa.write(PKT_MORSE);
    LoRa.write(pending.seq);
    LoRa.print(pending.morseText);
    LoRa.write((uint8_t)0); // terminador
    LoRa.write(pending.morseUnit & 0xFF);
    LoRa.write((pending.morseUnit >> 8) & 0xFF);
  }
  LoRa.endPacket();
  LoRa.receive();
  pending.sentAt = millis();
}

void queueSolenoidCommand(uint8_t state) {
  pending.active = true;
  pending.type = PKT_SOLENOID;
  pending.seq = nextSeq++;
  pending.solenoidState = state;
  pending.retries = 0;
  transmitPending();
}

void queueMorseCommand(const char* text, uint16_t unit) {
  pending.active = true;
  pending.type = PKT_MORSE;
  pending.seq = nextSeq++;
  strncpy(pending.morseText, text, sizeof(pending.morseText) - 1);
  pending.morseText[sizeof(pending.morseText) - 1] = '\0';
  pending.morseUnit = unit;
  pending.retries = 0;
  transmitPending();
}

// ---------- Revisa reintentos/timeout del comando pendiente ----------
void updatePendingCommand() {
  if (!pending.active) return;

  unsigned long now = millis();
  if (now - pending.sentAt >= ACK_TIMEOUT_MS) {
    if (pending.retries >= MAX_RETRIES) {
      Serial.print("FAIL:");
      Serial.println(pending.seq);
      pending.active = false;
      return;
    }
    pending.retries++;
    Serial.print("TIMEOUT-RETRY:");
    Serial.println(pending.seq);
    transmitPending();
  }
}

// ---------- Parseo de comandos entrantes por Serial ----------
void processSerialLine(const String& line) {
  if (!line.startsWith("CMD:")) return;

  // CMD:SOLENOID:1  |  CMD:MORSE:SOS:150
  int firstColon = line.indexOf(':', 4);
  String cmdName = (firstColon == -1) ? line.substring(4) : line.substring(4, firstColon);

  if (cmdName == "SOLENOID") {
    String val = line.substring(firstColon + 1);
    queueSolenoidCommand(val.toInt() != 0 ? 1 : 0);
  }
  else if (cmdName == "MORSE") {
    int secondColon = line.indexOf(':', firstColon + 1);
    String text, unitStr;
    if (secondColon == -1) {
      text = line.substring(firstColon + 1);
      unitStr = "100";
    } else {
      text = line.substring(firstColon + 1, secondColon);
      unitStr = line.substring(secondColon + 1);
    }
    queueMorseCommand(text.c_str(), (uint16_t)unitStr.toInt());
  }
}

// ---------- Manejo de paquetes LoRa entrantes ----------
void handleIncomingPacket(int packetSize) {
  if (packetSize <= 0) return;
  uint8_t type = LoRa.read();

  if (type == PKT_ACK) {
    uint8_t seq = LoRa.read();
    if (pending.active && seq == pending.seq) {
      Serial.print("OK:");
      Serial.println(seq);
      pending.active = false;
    }
    // si el seq no coincide (ACK viejo/duplicado), se ignora
  }
  else if (type == PKT_TELEMETRY) {
    // TODO: aqui va tu parseo/forward de telemetria existente hacia app.py
    // ejemplo minimo: reenviar crudo por Serial con un prefijo
    Serial.print("TLM:");
    while (LoRa.available()) Serial.write(LoRa.read());
    Serial.println();
  }
}

void setup() {
  Serial.begin(115200);

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

  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n') {
      serialLine.trim();
      if (serialLine.length() > 0) processSerialLine(serialLine);
      serialLine = "";
    } else if (c != '\r') {
      serialLine += c;
    }
  }

  updatePendingCommand();
}

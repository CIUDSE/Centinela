#include <Wire.h>

#include "ESP32_Brazo.h"
#include "Taranis_CH.h"

struct BrazoCmd {
  uint16_t ch1;
  uint16_t ch2;
  uint16_t ch4;
  uint8_t  velocidad; // 0 = precisa, 1 = rapida
};

volatile BrazoCmd ultimoComando;
volatile bool nuevoComando = false;

// =========================================================
// Controla un motor individual de un MDD10A.
// vel: rango -255..255 (signo = direccion, magnitud = PWM)
// =========================================================
void controlarMotor(uint8_t pinDIR, uint8_t canalPWM, int vel) {
  vel = constrain(vel, -255, 255);

  bool direccion = (vel >= 0);
  int magnitud = abs(vel);

  digitalWrite(pinDIR, direccion ? HIGH : LOW);
  ledcWrite(canalPWM, magnitud);
}

// =========================================================
// Controla ambos motores de una articulacion con el mismo
// valor (redundancia de fuerza, no mezcla diferencial).
// =========================================================
void controlarArticulacion(uint8_t dirM1, uint8_t canalM1,
                            uint8_t dirM2, uint8_t canalM2,
                            int vel) {
  controlarMotor(dirM1, canalM1, vel);
  controlarMotor(dirM2, canalM2, vel);
}

void onReceiveI2C(int numBytes) {
  if (numBytes == sizeof(BrazoCmd)) {
    Wire.readBytes((uint8_t*)&ultimoComando, sizeof(BrazoCmd));
    nuevoComando = true;
  }
}

// =========================================================
// Deja todo el brazo en 0 (seguridad ante perdida de señal/
// arranque inicial)
// =========================================================
void detenerTodo() {
  controlarArticulacion(A1_M1_DIR1, PWM_CANAL_A1_M1, A1_M2_DIR2, PWM_CANAL_A1_M2, 0);
  controlarArticulacion(A2_M1_DIR1, PWM_CANAL_A2_M1, A2_M2_DIR2, PWM_CANAL_A2_M2, 0);
  controlarArticulacion(A3_M1_DIR1, PWM_CANAL_A3_M1, A3_M2_DIR2, PWM_CANAL_A3_M2, 0);
}

void setup() {
  Serial.begin(115200);

  // ---- Pines DIR como salida ----
  pinMode(A1_M1_DIR1, OUTPUT);
  pinMode(A1_M2_DIR2, OUTPUT);
  pinMode(A2_M1_DIR1, OUTPUT);
  pinMode(A2_M2_DIR2, OUTPUT);
  pinMode(A3_M1_DIR1, OUTPUT);
  pinMode(A3_M2_DIR2, OUTPUT);

  // ---- PWM Setup ----
  ledcAttach(A1_M1_PWM1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(A1_M2_PWM2, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(A2_M1_PWM1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(A2_M2_PWM2, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(A3_M1_PWM1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(A3_M2_PWM2, PWM_FREQ, PWM_RESOLUTION);

  detenerTodo(); // arranca en reposo, no en velocidad indefinida

  Wire.begin(I2C_SLAVE_ADDR_BRAZO, PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.onReceive(onReceiveI2C);
}

void loop() {
  if (nuevoComando) {
    nuevoComando = false;

    BrazoCmd cmd = ultimoComando; // copia local
    float factor = cmd.velocidad ? 1.0 : 0.3;

    int velArt1 = (map(cmd.ch1, CH_MIN, CH_MAX, -255, 255)) * factor;
    int velArt2 = (map(cmd.ch2, CH_MIN, CH_MAX, -255, 255)) * factor;
    int velArt3 = (map(cmd.ch3, CH_MIN, CH_MAX, -255, 255)) * factor;

    controlarArticulacion(A1_M1_DIR1, A1_M1_PWM1, A1_M2_DIR2, A1_M2_PWM2, velArt1);
    controlarArticulacion(A2_M1_DIR1, A2_M1_PWM1, A2_M2_DIR2, A2_M2_PWM2, velArt2);
    controlarArticulacion(A3_M1_DIR1, A3_M1_PWM1, A3_M2_DIR2, A3_M2_PWM2, velArt3);
  }
}
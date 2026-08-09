#include <Wire.h>

// =======================================================
// Pines I2C (esclavo)
// =======================================================
#define PIN_I2C_SDA   21
#define PIN_I2C_SCL   22
#define I2C_SLAVE_ADDR_BRAZO  0x08

// =======================================================
// Driver 1 - MDD10A (Articulacion 1 / CH1)
// =======================================================
#define A1_M1_PWM1  32
#define A1_M1_DIR1  19

#define A1_M2_PWM2  33
#define A1_M2_DIR2  18

// =======================================================
// Driver 2 - MDD10A (Articulacion 2 / CH2)
// =======================================================
#define A2_M1_PWM1  25   
#define A2_M1_DIR1  23   

#define A2_M2_PWM2  26
#define A2_M2_DIR2  17  

// =======================================================
// Driver 3 - MDD10A (Articulacion 3 / CH4)
// =======================================================
#define A3_M1_PWM1  27   
#define A3_M1_DIR1  4  

#define A3_M2_PWM2  13 
#define A3_M2_DIR2  16

// =======================================================
// Config PWM (LEDC del ESP32)
// =======================================================
#define PWM_FREQ        20000
#define PWM_RESOLUTION  8

// Canales LEDC (API antigua: se asignan a mano, separados del numero de pin)
#define CANAL_A1_M1  0
#define CANAL_A1_M2  1
#define CANAL_A2_M1  2
#define CANAL_A2_M2  3
#define CANAL_A3_M1  4
#define CANAL_A3_M2  5

#define CH_MAX 2012
#define CH_MIN 989

// Zona muerta: cualquier velocidad entre -10 y 10 se trata como 0 explicito
#define ZONA_MUERTA_VEL  10

// Paso maximo de cambio de PWM por ciclo de loop (soft-start / limitacion de
// pico de corriente). Ajustar segun que tan rapido se quiera que acelere:
// valores mas chicos = arranque mas suave pero mas lento; valores mas grandes
// = respuesta mas rapida pero picos de corriente mas altos.
#define PASO_MAX_RAMPA  8

const uint16_t UMBRAL_SWITCH = 1500;

struct BrazoCmd {
  uint16_t ch1;
  uint16_t ch2;
  uint16_t ch4;
  uint16_t ch6;
};

volatile BrazoCmd ultimoComando;
volatile bool nuevoComando = false;

// Estado de rampa para los motores de la garra (muñequeo + apertura/cierre)
static int velActualMuneca = 0;
static int velActualGarra  = 0;

// =========================================================
// Limita la tasa de cambio de una velocidad hacia un objetivo.
// Evita saltos instantaneos de PWM, que son la causa principal
// de los picos de corriente al arrancar o revertir un motor DC.
// =========================================================
int rampear(int actual, int objetivo, int pasoMax) {
  int diferencia = objetivo - actual;
  if (diferencia > pasoMax) diferencia = pasoMax;
  if (diferencia < -pasoMax) diferencia = -pasoMax;
  return actual + diferencia;
}

// =========================================================
// Controla un motor individual de un MDD10A.
// vel: rango -255..255 (signo = direccion, magnitud = PWM)
//
// Tres estados posibles:
//   1) vel > ZONA_MUERTA_VEL   -> avanza (DIR HIGH, PWM = magnitud)
//   2) vel < -ZONA_MUERTA_VEL  -> retrocede (DIR LOW, PWM = magnitud)
//   3) -ZONA_MUERTA_VEL <= vel <= ZONA_MUERTA_VEL -> detenido (PWM = 0 explicito)
// =========================================================
void controlarMotor(const char* etiqueta, uint8_t pinDIR, uint8_t canalPWM, int vel) {
  vel = constrain(vel, -255, 255);

  bool dentroZonaMuerta = (vel >= -ZONA_MUERTA_VEL) && (vel <= ZONA_MUERTA_VEL);

  bool direccion;
  int magnitud;

  if (dentroZonaMuerta) {
    direccion = digitalRead(pinDIR);
    magnitud = 0;
  } else {
    direccion = (vel > 0);
    magnitud = abs(vel);
  }

  digitalWrite(pinDIR, direccion ? HIGH : LOW);
  ledcWrite(canalPWM, magnitud);

  Serial.print("  ");
  Serial.print(etiqueta);
  Serial.print(" | DIR pin ");
  Serial.print(pinDIR);
  Serial.print(": ");
  Serial.print(direccion ? "HIGH" : "LOW");
  Serial.print(" | PWM canal ");
  Serial.print(canalPWM);
  Serial.print(": ");
  Serial.print(magnitud);
  Serial.println(dentroZonaMuerta ? "  (zona muerta)" : "");
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
  controlarMotor("M1", A1_M1_DIR1, CANAL_A1_M1, 0);
  controlarMotor("M2", A1_M2_DIR2, CANAL_A1_M2, 0);
  controlarMotor("M3", A2_M1_DIR1, CANAL_A2_M1, 0);
  controlarMotor("M4", A2_M2_DIR2, CANAL_A2_M2, 0);
  controlarMotor("M5", A3_M1_DIR1, CANAL_A3_M1, 0);
  velActualMuneca = 0;
  velActualGarra  = 0;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Iniciando esclavo...");

  // ---- Pines DIR como salida ----
  pinMode(A1_M1_DIR1, OUTPUT);
  pinMode(A1_M2_DIR2, OUTPUT);
  pinMode(A2_M1_DIR1, OUTPUT);
  pinMode(A2_M2_DIR2, OUTPUT);
  pinMode(A3_M1_DIR1, OUTPUT);
  pinMode(A3_M2_DIR2, OUTPUT);

  ledcSetup(CANAL_A1_M1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(A1_M1_PWM1, CANAL_A1_M1);

  ledcSetup(CANAL_A1_M2, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(A1_M2_PWM2, CANAL_A1_M2);

  ledcSetup(CANAL_A2_M1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(A2_M1_PWM1, CANAL_A2_M1);

  ledcSetup(CANAL_A2_M2, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(A2_M2_PWM2, CANAL_A2_M2);

  ledcSetup(CANAL_A3_M1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(A3_M1_PWM1, CANAL_A3_M1);

  ledcSetup(CANAL_A3_M2, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(A3_M2_PWM2, CANAL_A3_M2);
 
  Wire.begin(I2C_SLAVE_ADDR_BRAZO);
  Wire.onReceive(onReceiveI2C);

  Serial.println("Setup completado");
}

void loop() {
  if (nuevoComando) {
    nuevoComando = false;

    BrazoCmd cmd;
    noInterrupts();
    memcpy((void*)&cmd, (const void*)&ultimoComando, sizeof(BrazoCmd));
    interrupts();

    int velArt1 = (map(cmd.ch1, CH_MIN, CH_MAX, -255, 255));
    int velArt2 = (map(cmd.ch2, CH_MIN, CH_MAX, -255, 255));
    int velArt3 = (map(cmd.ch4, CH_MIN, CH_MAX, -255, 255));

    // --- Bloqueo: solo un canal puede estar activo a la vez, en todo ---
    // --- el sistema (brazo o garra), sin importar el switch ch6.     ---
    // NOTA: se deja igual que en el original para no alterar el       ---
    // comportamiento del brazo. En modo garra, ch2 ya no controla     ---
    // ningun motor (ver mas abajo) -- si el stick de ch2 tiene deriva ---
    // fuera de la zona muerta mientras se esta en modo garra, este    ---
    // bloqueo podria reclamar CANAL_CH2 y anular momentaneamente el   ---
    // muñequeo (ch1) y la apertura/cierre (ch4). Si eso se observa en ---
    // pruebas, la solucion es excluir ch2 del bloqueo cuando ch6 este ---
    // en modo garra.                                                  ---
    static enum { NINGUN_CANAL, CANAL_CH1, CANAL_CH2, CANAL_CH4 } canalActivo = NINGUN_CANAL;

    bool ch1_activo = abs(velArt1) > ZONA_MUERTA_VEL;
    bool ch2_activo = abs(velArt2) > ZONA_MUERTA_VEL;
    bool ch4_activo = abs(velArt3) > ZONA_MUERTA_VEL;

    if (canalActivo == NINGUN_CANAL) {
      if (ch1_activo) {
        canalActivo = CANAL_CH1;
      } else if (ch2_activo) {
        canalActivo = CANAL_CH2;
      } else if (ch4_activo) {
        canalActivo = CANAL_CH4;
      }
    } else if (canalActivo == CANAL_CH1 && !ch1_activo) {
      canalActivo = NINGUN_CANAL;
    } else if (canalActivo == CANAL_CH2 && !ch2_activo) {
      canalActivo = NINGUN_CANAL;
    } else if (canalActivo == CANAL_CH4 && !ch4_activo) {
      canalActivo = NINGUN_CANAL;
    }

    // Anula la velocidad de cualquier canal que no sea el bloqueado
    if (canalActivo != CANAL_CH1) velArt1 = 0;
    if (canalActivo != CANAL_CH2) velArt2 = 0;
    if (canalActivo != CANAL_CH4) velArt3 = 0;

    if (cmd.ch6 > UMBRAL_SWITCH) {
      Serial.println("CONTROL DE BRAZO");

      controlarMotor("M1", A1_M1_DIR1, CANAL_A1_M1, velArt1);
      controlarMotor("M2", A1_M2_DIR2, CANAL_A1_M2, velArt2);

      // Fuera de modo garra: se resetea la rampa para que al volver a
      // entrar a modo garra no arranque desde una velocidad vieja.
      velActualMuneca = 0;
      velActualGarra  = 0;
    } else {
      Serial.println("CONTROL DE GARRA");

      // --- Muñequeo: un solo eje (ch1), mandado a los 2 motores del ---
      // diferencial con signo invertido entre ellos porque estan     ---
      // montados espejo (viendose de frente).                        ---
      velActualMuneca = rampear(velActualMuneca, velArt1, PASO_MAX_RAMPA);

      // Apertura/cierre: motor independiente (M5), sin mezcla.
      velActualGarra = rampear(velActualGarra, velArt3, PASO_MAX_RAMPA);

      controlarMotor("M3", A2_M1_DIR1, CANAL_A2_M1,  velActualMuneca);
      controlarMotor("M4", A2_M2_DIR2, CANAL_A2_M2, -velActualMuneca);
      controlarMotor("M5", A3_M1_DIR1, CANAL_A3_M1,  velActualGarra);
    }
  }

  delay(5);
}
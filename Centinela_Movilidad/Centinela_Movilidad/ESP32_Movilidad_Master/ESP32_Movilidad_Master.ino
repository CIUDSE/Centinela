// Este codigo fue probado en Arduino IDE

// Utiliza la libreria CRSF for Arduino de Cassandra Robinson. Descargarla antes de ejecutar este script.

#include <CRSFforArduino.hpp>
#include <Wire.h>

#include "CMotor_ESP32.h"

#include "ESP32_Tren_Motriz.h"

#include "Taranis_CH.h"

CRSFforArduino *crsf = nullptr;

void onReceiveRcChannels(serialReceiverLayer::rcChannels_t *rcData);

// Umbral central para distinguir posicion alta vs baja del switch de 3 estados.
// Como solo usamos MAX y MIN (ignoramos el centro), cualquier valor >1500
// se toma como "alto" y cualquier valor <=1500 se toma como "bajo".
const uint16_t UMBRAL_SWITCH = 1500;

// --- Config PWM del tren motriz (LEDC del ESP32) ---
#define PWM_FREQ_TREN        20000
#define PWM_RESOLUTION_TREN  8

// Estructura de datos que se envia por I2C al esclavo del brazo
struct BrazoCmd {
  uint16_t ch1;
  uint16_t ch2;
  uint16_t ch4;
  uint8_t  velocidad; // 0 = precisa, 1 = rapida
};

void enviarComandoBrazo(uint16_t ch1, uint16_t ch2, uint16_t ch4, uint8_t velocidad) {
  BrazoCmd cmd;
  cmd.ch1 = ch1;
  cmd.ch2 = ch2;
  cmd.ch4 = ch4;
  cmd.velocidad = velocidad;

  Wire.beginTransmission(I2C_SLAVE_ADDR_BRAZO);
  Wire.write((uint8_t*)&cmd, sizeof(cmd));
  uint8_t error = Wire.endTransmission();

  if (error != 0) {
    Serial.print("Error I2C al enviar comando de brazo, codigo: ");
    Serial.println(error);
  }
}

void controlarTrenMotriz(uint16_t ch1_raw, uint16_t ch2_raw, uint16_t ch3_raw) {
  // --- Direccion (canal 1): binaria con zona muerta ---
  int ch1_centrado = (int)ch1_raw - UMBRAL_SWITCH;
  int direccion = 0; 

  if (ch1_centrado > ZONA_MUERTA_DIR) {
    Serial.println("adelante");

    digitalWrite(DIR_RIGHT, HIGH);
    digitalWrite(DIR_LEFT, LOW);
  } else if (ch1_centrado < -ZONA_MUERTA_DIR) {
    Serial.println("detras");

    digitalWrite(DIR_RIGHT, LOW);
    digitalWrite(DIR_LEFT, HIGH);
  }

  int potencia = map(ch3_raw, CH_MIN, CH_MAX, 0, 255);
  potencia = constrain(potencia, 0, 255);

   // --- Giro (canal 2): fraccion -1.0 a 1.0, proporcional a la potencia actual ---
  int giroRaw = map((int)ch2_raw, CH_MIN, CH_MAX, -255, 255);
  if (abs(giroRaw) < ZONA_MUERTA_GIRO) {
    giroRaw = 0;
  }

  float giroFrac = giroRaw / 255.0; // -1.0 (a fondo un lado) a 1.0 (a fondo el otro)

  int potIzq = constrain((int)(potencia - giroFrac * potencia), 0, potencia);
  int potDer = constrain((int)(potencia + giroFrac * potencia), 0, potencia);

  Serial.print("pot izq: "); Serial.println(potIzq);
  Serial.print(" pot der: "); Serial.println(potDer);

  Serial.print("potencia: "); Serial.println(potencia);

  // Van en lado contrario por eso los drivers de las llantas derechas se les pone potencia de izquierda y viceversa.

  ledcWrite(DRIVER_1_TREN_MOTRIZ, potDer); // lado izquierdo
  ledcWrite(DRIVER_2_TREN_MOTRIZ, potDer); // lado izquierdo
  ledcWrite(DRIVER_3_TREN_MOTRIZ, potIzq); // lado derecho
  ledcWrite(DRIVER_4_TREN_MOTRIZ, potIzq); // lado derecho
} 

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  // --- Tren motriz: attach PWM en los 4 drivers ---
  ledcAttach(DRIVER_1_TREN_MOTRIZ, PWM_FREQ_TREN, PWM_RESOLUTION_TREN);
  ledcAttach(DRIVER_2_TREN_MOTRIZ, PWM_FREQ_TREN, PWM_RESOLUTION_TREN);
  ledcAttach(DRIVER_3_TREN_MOTRIZ, PWM_FREQ_TREN, PWM_RESOLUTION_TREN);
  ledcAttach(DRIVER_4_TREN_MOTRIZ, PWM_FREQ_TREN, PWM_RESOLUTION_TREN);

  pinMode(DIR_LEFT, OUTPUT);
  pinMode(DIR_RIGHT, OUTPUT);

  pinMode(BRAKE, OUTPUT);

  digitalWrite(DIR_LEFT, LOW); // estado inicial (adelante) por defecto
  digitalWrite(DIR_RIGHT, HIGH);

  digitalWrite(BRAKE, LOW);

  // Arranca detenido
  ledcWrite(DRIVER_1_TREN_MOTRIZ, 0);
  ledcWrite(DRIVER_2_TREN_MOTRIZ, 0);
  ledcWrite(DRIVER_3_TREN_MOTRIZ, 0);
  ledcWrite(DRIVER_4_TREN_MOTRIZ, 0);

  crsf = new CRSFforArduino(&Serial2, PIN_CRSF_RX, PIN_CRSF_TX); // RX=16, TX=17

  if (crsf->begin() == true) {
    Serial.println("CRSF OK");
    crsf->setRcChannelsCallback(onReceiveRcChannels);
  } else {
    Serial.println("CRSF FALLO");
    crsf->end();
    delete crsf;
    crsf = nullptr;
  }
}

void loop() {
  if (crsf != nullptr) {
    crsf->update();
  }
}

void onReceiveRcChannels(serialReceiverLayer::rcChannels_t *rcData) {
  static uint32_t lastPrint = millis();
  if (millis() - lastPrint >= 100) {
    lastPrint = millis();

    // Lectura de sticks de control (ya corregidos segun el mix real de la radio)
    uint16_t ch1 = crsf->rcToUs(rcData->value[0]);
    uint16_t ch2 = crsf->rcToUs(rcData->value[1]);
    uint16_t ch3 = crsf->rcToUs(rcData->value[2]);
    uint16_t ch4 = crsf->rcToUs(rcData->value[3]);

    // Lectura de switches de modo
    uint16_t ch5_modo = crsf->rcToUs(rcData->value[4]);      // Switch de tren motriz / brazo
    uint16_t ch6_velocidad = crsf->rcToUs(rcData->value[5]); // Switch de rapido / preciso

    /// =================================================================
    // BLOQUE 1: Modo de operacion = TREN MOTRIZ (CH5 en posicion ALTA)
    // CH3 = potencia (todo el rango). Direccion (CH1) pendiente.
    // =================================================================
    if (ch5_modo > UMBRAL_SWITCH) {
      Serial.println("Modo: TREN MOTRIZ");

      if(ch6_velocidad > UMBRAL_SWITCH) {
        digitalWrite(BRAKE, LOW);
      } else {
        digitalWrite(BRAKE, HIGH);
      }

      controlarTrenMotriz(ch1, ch2, ch3);

      // --- Ifs de modo de velocidad (rapida/precisa) desactivados por ahora ---
      // if (ch6_velocidad > UMBRAL_SWITCH) {
      //   Serial.println("Modo: TREN MOTRIZ, Velocidad: RAPIDA");
      // } else {
      //   Serial.println("Modo: TREN MOTRIZ, Velocidad: PRECISA");
      // }

    // =================================================================
    // BLOQUE 2: Modo de operacion = BRAZO (CH5 en posicion BAJA)
    // Aqui los 4 sticks controlan las articulaciones del brazo robotico:
    // CH1, CH2 y CH4 = ejes/articulaciones (CH3 libre o usado para pinza)
    // =================================================================
    } else {
      // -------------------------------------------------------------
      // BLOQUE 2.1: Dentro de brazo, velocidad RAPIDA (CH6 alto)
      // Movimiento amplio y rapido del brazo, para reposicionarlo
      // rapidamente antes de hacer un movimiento fino.
      // -------------------------------------------------------------
      if (ch6_velocidad > UMBRAL_SWITCH) {
        Serial.println("Modo: BRAZO, Velocidad: RAPIDA");

        enviarComandoBrazo(ch1, ch2, ch4, 1); // 1 = velocidad rapida de factor 1.0

      // -------------------------------------------------------------
      // BLOQUE 2.2: Dentro de brazo, velocidad PRECISA (CH6 bajo)
      // Movimiento lento y controlado del brazo, para tareas
      // que requieren precision como tomar una muestra u objeto.
      // -------------------------------------------------------------

      } else {
        Serial.println("Modo: BRAZO, Velocidad: PRECISA");

        // Aqui va la logica real de control del brazo a velocidad reducida
        // controlarBrazo(ch1, ch2, ch4); // factor de velocidad = 30%
        enviarComandoBrazo(ch1, ch2, ch4, 0); // 0 = velocidad precisa de factor 0.3
      }
    }

    // Serial.print(" CH1:"); Serial.print(ch1);
    // Serial.print(" CH2:"); Serial.print(ch2);
    // Serial.print(" CH3:"); Serial.print(ch3);
    // Serial.print(" CH4:"); Serial.println(ch4);
    // Serial.print(" CH5 (modo):"); Serial.print(ch5_modo);
    // Serial.print(" CH6 (velocidad):"); Serial.println(ch6_velocidad);
  }
}
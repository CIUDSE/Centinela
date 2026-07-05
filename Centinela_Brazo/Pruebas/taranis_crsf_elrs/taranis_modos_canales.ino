// Este codigo fue probado en Arduino IDE

// Utiliza la libreria CRSF for Arduino de Cassandra Robinson. Descargarla antes de ejecutar este script.

#include <CRSFforArduino.hpp>

CRSFforArduino *crsf = nullptr;

void onReceiveRcChannels(serialReceiverLayer::rcChannels_t *rcData);

// Umbral central para distinguir posicion alta vs baja del switch de 3 estados.
// Como solo usamos MAX y MIN (ignoramos el centro), cualquier valor >1500
// se toma como "alto" y cualquier valor <=1500 se toma como "bajo".
const uint16_t UMBRAL_SWITCH = 1500;

void setup() {
  Serial.begin(115200);
  delay(1000);

  crsf = new CRSFforArduino(&Serial2, 16, 17); // RX=16, TX=17

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

    // =================================================================
    // BLOQUE 1: Modo de operacion = TREN MOTRIZ (CH5 en posicion ALTA)
    // Aqui los 4 sticks controlan las ruedas/motores del rover:
    // CH1 = direccion, CH3 = potencia de avance/reversa (CH2 y CH4 libres)
    // =================================================================
    if (ch5_modo > UMBRAL_SWITCH) {

      // -------------------------------------------------------------
      // BLOQUE 1.1: Dentro de tren motriz, velocidad RAPIDA (CH6 alto)
      // Se usa el rango completo del stick para maxima velocidad,
      // pensado para desplazamientos largos en terreno despejado.
      // -------------------------------------------------------------
      if (ch6_velocidad > UMBRAL_SWITCH) {
        Serial.println("Modo: TREN MOTRIZ, Velocidad: RAPIDA");

        // Aqui va la logica real de control de motores a velocidad completa
        // controlarTrenMotriz(ch3, ch1, 1.0); // factor de velocidad = 100%

      // -------------------------------------------------------------
      // BLOQUE 1.2: Dentro de tren motriz, velocidad PRECISA (CH6 bajo)
      // Se reduce el factor de velocidad para maniobras finas,
      // pensado para acercarse a obstaculos o zonas estrechas.
      // -------------------------------------------------------------
      } else {
        Serial.println("Modo: TREN MOTRIZ, Velocidad: PRECISA");

        // Aqui va la logica real de control de motores a velocidad reducida
        // controlarTrenMotriz(ch3, ch1, 0.3); // factor de velocidad = 30%
      }

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

        // Aqui va la logica real de control del brazo a velocidad completa
        // controlarBrazo(ch1, ch2, ch4, 1.0); // factor de velocidad = 100%

      // -------------------------------------------------------------
      // BLOQUE 2.2: Dentro de brazo, velocidad PRECISA (CH6 bajo)
      // Movimiento lento y controlado del brazo, para tareas
      // que requieren precision como tomar una muestra u objeto.
      // -------------------------------------------------------------
      } else {
        Serial.println("Modo: BRAZO, Velocidad: PRECISA");

        // Aqui va la logica real de control del brazo a velocidad reducida
        // controlarBrazo(ch1, ch2, ch4, 0.3); // factor de velocidad = 30%
      }
    }

    Serial.print(" CH1:"); Serial.print(ch1);
    Serial.print(" CH2:"); Serial.print(ch2);
    Serial.print(" CH3:"); Serial.print(ch3);
    Serial.print(" CH4:"); Serial.println(ch4);
    Serial.print(" CH5 (modo):"); Serial.print(ch5_modo);
    Serial.print(" CH6 (velocidad):"); Serial.println(ch6_velocidad);
  }
}
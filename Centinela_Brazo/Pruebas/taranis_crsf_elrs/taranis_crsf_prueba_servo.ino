// Este codigo fue probado en Arduino IDE

// Utiliza la libreria CRSF for Arduino de Cassandra Robinson. Descargarla antes de ejecutar este script.

// TO-DO LIST:
//      -> Utilizando el servo MG90S, todavia no se consigue que el servo se abra 180 grados. Se esta revisando.

#include <CRSFforArduino.hpp>

#define SERVO_PIN 4

CRSFforArduino *crsf = nullptr;
volatile uint16_t servoTarget = 1500;

void onReceiveRcChannels(serialReceiverLayer::rcChannels_t *rcData);

hw_timer_t *servoTimer = nullptr;
volatile bool writePulse = false;

void IRAM_ATTR onServoTimer() {
  writePulse = true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(SERVO_PIN, OUTPUT);
  digitalWrite(SERVO_PIN, LOW);

  // Hardware timer — fires every 20ms (50Hz)
  servoTimer = timerBegin(1000000);         // 1MHz = 1us resolution
  timerAttachInterrupt(servoTimer, &onServoTimer);
  timerAlarm(servoTimer, 20000, true, 0);   // 20000us = 20ms

  crsf = new CRSFforArduino(&Serial2, 16, 17);

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

  if (writePulse) {
    writePulse = false;
    uint16_t pulse = servoTarget;
    digitalWrite(SERVO_PIN, HIGH);
    delayMicroseconds(pulse);
    digitalWrite(SERVO_PIN, LOW);
  }
}

void onReceiveRcChannels(serialReceiverLayer::rcChannels_t *rcData) {
  servoTarget = constrain(crsf->rcToUs(rcData->value[1]), 700, 2300);

  static uint32_t lastPrint = 0;
  if (millis() - lastPrint >= 100) {
    lastPrint = millis();
    Serial.print("CH1:"); Serial.print(crsf->rcToUs(rcData->value[0]));
    Serial.print(" CH2(servo):"); Serial.print(servoTarget);
    Serial.print(" CH3:"); Serial.print(crsf->rcToUs(rcData->value[2]));
    Serial.print(" CH4:"); Serial.println(crsf->rcToUs(rcData->value[3]));
  }
}
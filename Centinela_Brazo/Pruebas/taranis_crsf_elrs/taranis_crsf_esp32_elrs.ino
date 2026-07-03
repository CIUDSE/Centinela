// Este codigo fue probado en Arduino IDE

// Utiliza la libreria CRSF for Arduino de Cassandra Robinson. Descargarla antes de ejecutar este script.

#include <CRSFforArduino.hpp>

CRSFforArduino *crsf = nullptr;

void onReceiveRcChannels(serialReceiverLayer::rcChannels_t *rcData);

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
    Serial.print("CH1:"); Serial.print(crsf->rcToUs(rcData->value[0]));
    Serial.print(" CH2:"); Serial.print(crsf->rcToUs(rcData->value[1]));
    Serial.print(" CH3:"); Serial.print(crsf->rcToUs(rcData->value[2]));
    Serial.print(" CH4:"); Serial.println(crsf->rcToUs(rcData->value[3]));
    Serial.print(" CH5:"); Serial.println(crsf->rcToUs(rcData->value[4]));
    Serial.print(" CH6:"); Serial.println(crsf->rcToUs(rcData->value[5]));
  }
}
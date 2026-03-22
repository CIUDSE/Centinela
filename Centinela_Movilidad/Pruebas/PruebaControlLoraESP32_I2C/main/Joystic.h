#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include <cstdint>

using namespace std;

class Joystick_Converter{
    public:
        Joystick_Converter(float max_speed_ms); //Constructor con los valores de las posiciones del joystick

        void speed_ms_conversion(uint16_t X_position, uint16_t Y_position, uint8_t brakes); //Convierte la posicion del joystick a velocidades para ambos motores

        float right_speed_ms = 0.0f, left_speed_ms = 0.0f;
    private:
        float expo_factor = 0.7f; //Velocidad en m/s que se le asignara al motor
        float MAX_SPEED_MS; //Velocidad maxima del motor en m/s, se asignara en el constructor dependiendo del motor que se use
        int32_t RANGO_DEADZONE = 1500, CENTRO = 32768; //Rango de la zona muerta y el valor central del joystick
        float turn_factor = 1.0f; //Multiplicador para ajustar que tan agresivo es el giro
};

#endif

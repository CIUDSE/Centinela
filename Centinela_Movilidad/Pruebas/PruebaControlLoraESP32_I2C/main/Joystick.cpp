#include "Joystic.h"
#include <cstdlib>

Joystick_Converter::Joystick_Converter(float max_speed_ms){
    MAX_SPEED_MS = max_speed_ms;
};

void Joystick_Converter::speed_ms_conversion(uint16_t X_position, uint16_t Y_position, uint8_t brakes){
    
    int32_t diffX = (int32_t)X_position - this->CENTRO;
    int32_t diffY = (int32_t)Y_position - this->CENTRO;

    if (abs(diffX) < RANGO_DEADZONE) diffX = 0; // Zona muerta para el eje X
    if (abs(diffY) < RANGO_DEADZONE) diffY = 0; // Zona muerta para el eje Y

    float normX = (float)diffX / 32768.0f; 
    float normY = (float)diffY / 32768.0f; 
    
    float expo_x = (1.0f - this->expo_factor)*normX + this->expo_factor*(normX*normX*normX); // Aplicar funcion exponencial para suavizar la respuesta
    float expo_y = (1.0f - this->expo_factor)*normY + this->expo_factor*(normY*normY*normY); // Aplicar funcion exponencial para suavizar la respuesta

    float throttle = expo_y * this->MAX_SPEED_MS; // Y controla avance o reversa
    float turn = expo_x * this->MAX_SPEED_MS * this->turn_factor; // X controla que tanto se reparte la diferencia entre lados

    // Convencion de giro:
    // X > 0  -> giro a la derecha  -> el motor izquierdo acelera y el derecho se frena o invierte.
    // X < 0  -> giro a la izquierda -> el motor derecho acelera y el izquierdo se frena o invierte.
    this->right_speed_ms = throttle - turn; // Motor derecho
    this->left_speed_ms = throttle + turn;  // Motor izquierdo

    if (this->right_speed_ms > this->MAX_SPEED_MS) this->right_speed_ms = this->MAX_SPEED_MS;
    if (this->right_speed_ms < -this->MAX_SPEED_MS) this->right_speed_ms = -this->MAX_SPEED_MS;
    if (this->left_speed_ms > this->MAX_SPEED_MS) this->left_speed_ms = this->MAX_SPEED_MS;
    if (this->left_speed_ms < -this->MAX_SPEED_MS) this->left_speed_ms = -this->MAX_SPEED_MS;

    if (brakes != 0) {
        this->right_speed_ms = 0.0f;
        this->left_speed_ms = 0.0f;
    }
}

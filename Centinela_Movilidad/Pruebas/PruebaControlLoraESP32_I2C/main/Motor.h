#ifndef MOTOR_H
#define MOTOR_H

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

class Driver1 {
    public:
        Driver1(gpio_num_t PWM1, gpio_num_t PWM2, gpio_num_t DIR1, gpio_num_t DIR2, float maxSpeed);//Constructor with pin arguments
        
        //Establece una funcion para encender los motores y establecer una velocidad
        //En este caso para detener el motor se puede poner una velocidad de 0 
        void motor1_set_speed(float speed_ms); //Establece la velocidad del motor 1    
        void motor2_set_speed(float speed_ms); //Establece la velocidad del motor 2
        
        float motor_speed_to_dutyCycle(float speed); //Convierte la velocidad en un duty cycle para el PWM donde la velocidad es en m/s
        
        void motor1_linear_increase(float ts_speed, int time_ms);


        //Cambiar el giro de los motores
        void motor1_direction_toggle(void);
        void motor2_direction_toggle(void);
        
    private:
        //Pines para darle corriente los motores 
        gpio_num_t PWM1, PWM2;

        //Pines para cambio de giro
        gpio_num_t DIR1, DIR2;
        bool DIR1_STATE, DIR2_STATE;

        //Variable para saber la velocidad de rotacion de los motores
        float motor1_speed, motor2_speed, max_speed;

        //Variables para el canal 
        ledc_channel_t ledcChannel1, ledcChannel2;
};

#endif
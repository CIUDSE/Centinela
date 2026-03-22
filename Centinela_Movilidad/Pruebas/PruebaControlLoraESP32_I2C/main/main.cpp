// Programa principal para los motores utilizando LORA -> ESP32 -> PWM
// Se utiliza CMAKE y se hace la build con esp-idf v5.5.3, flasheo con esptool.py

//Drivers//
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_err.h"

#include <cstdint>

#include "Joystic.h"
#include "Motor.h"
#include "Input_LoRa.h"

#define BLINKER_PIN GPIO_NUM_2

// --Pines de los motores -- //

//Motor derecho
#define FRONT_R_PWM GPIO_NUM_27
#define DIR_R GPIO_NUM_32

//Motor izquierdo
#define FRONT_L_PWM GPIO_NUM_26
#define DIR_L GPIO_NUM_25

// -- Pines del LoRa I2C -- //
#define I2C_SLAVE_SCL_IO GPIO_NUM_22
#define I2C_SLAVE_SDA_IO GPIO_NUM_21
#define I2C_SLAVE_PORT I2C_NUM_0
#define I2C_SLAVE_ADDR 0x08

extern "C" void app_main(void)
{
    // Setear los parametros del I2C slave para recibir datos del master
    InputLoRa myLoRa(I2C_SLAVE_SCL_IO, I2C_SLAVE_SDA_IO, I2C_SLAVE_PORT, I2C_SLAVE_ADDR);
    if (!myLoRa.is_ready()) {
        printf("No se pudo iniciar el I2C slave: %s\n", esp_err_to_name(myLoRa.get_init_status()));
        return;
    }

    float max_motor_speed = 5.0f; // Velocidad maxima del motor en m/s
    Driver1 motors(FRONT_R_PWM, FRONT_L_PWM, DIR_R, DIR_L, max_motor_speed); // Establecer velocidad maxima a 5 m/s
    Joystick_Converter joystick_converter(max_motor_speed);
    uint32_t timeout_count = 0;

    while (1) {
        esp_err_t ret = myLoRa.read_data(); // Recibir datos del master
        if (ret == ESP_ERR_TIMEOUT) {
            timeout_count++;
            if ((timeout_count % 10) == 0) {
                printf("Sin datos I2C completos despues de %lu intentos\n", static_cast<unsigned long>(timeout_count));
            }

            motors.motor1_set_speed(0.0f);
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        if (ret != ESP_OK) {
            printf("Recepcion I2C fallida: %s\n", esp_err_to_name(ret));
            motors.motor1_set_speed(0.0f);
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        timeout_count = 0;
        printf("Datos I2C -> x:%u y:%u brakes:%u\n", myLoRa.x, myLoRa.y, myLoRa.brakes);
        joystick_converter.speed_ms_conversion(myLoRa.x, myLoRa.y, myLoRa.brakes, [&motors]() { motors.motor1_direction_toggle(); });
        motors.motor1_set_speed(joystick_converter.speed_msy);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

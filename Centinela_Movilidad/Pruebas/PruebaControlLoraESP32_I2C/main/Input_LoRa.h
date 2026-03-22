#pragma once
#ifndef INPUT_LORA_H
#define INPUT_LORA_H

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include <cstddef>

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t brakes;
} __attribute__((packed)) LoRAData_t;

static_assert(sizeof(LoRAData_t) == 5, "LoRAData_t debe ocupar 5 bytes");

class InputLoRa {
    public:
        InputLoRa(gpio_num_t scl, gpio_num_t sda, i2c_port_t port, uint16_t slave_addr);

        esp_err_t read_data(TickType_t timeout_ticks = pdMS_TO_TICKS(200));
        bool is_ready() const;
        esp_err_t get_init_status() const;

        uint16_t x, y;
        uint8_t brakes;

    private:
        gpio_num_t scl;
        gpio_num_t sda;
        i2c_port_t port;
        uint16_t slave_addr;
        esp_err_t init_status;
        size_t rx_buffered_bytes;
        uint8_t rx_buffer[sizeof(LoRAData_t)];

        esp_err_t InitI2C_slave();
};

#endif // INPUT_LORA_H

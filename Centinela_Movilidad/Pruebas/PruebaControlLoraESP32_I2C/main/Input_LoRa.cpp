#include "Input_LoRa.h"
#include "esp_err.h"
#include "esp_log.h"
#include <cstring>

static const char *TAG = "I2C_SLAVE";
static constexpr size_t I2C_SLAVE_RX_BUF_LEN = 128;
static constexpr size_t I2C_SLAVE_TX_BUF_LEN = 128;

InputLoRa::InputLoRa(gpio_num_t scl, gpio_num_t sda, i2c_port_t port, uint16_t slave_addr) {
    this->scl = scl;
    this->sda = sda;
    this->port = port;
    this->slave_addr = slave_addr;
    this->x = 0;
    this->y = 0;
    this->brakes = 0;
    this->rx_buffered_bytes = 0;

    this->init_status = this->InitI2C_slave();
    if (this->init_status != ESP_OK) {
        ESP_LOGE(TAG, "No se pudo iniciar el I2C slave: %s", esp_err_to_name(this->init_status));
    }
}

esp_err_t InputLoRa::InitI2C_slave() {
    i2c_config_t config = {};
    config.mode = I2C_MODE_SLAVE;
    config.sda_io_num = this->sda;
    config.scl_io_num = this->scl;
    config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    config.slave.addr_10bit_en = 0;
    config.slave.slave_addr = this->slave_addr;

    ESP_LOGI(TAG, "Configurando slave: SDA=%d SCL=%d PORT=%d ADDR=0x%02X",
             this->sda, this->scl, this->port, this->slave_addr);

    esp_err_t ret = i2c_param_config(this->port, &config);
    if (ret != ESP_OK) {
        return ret;
    }

    return i2c_driver_install(this->port, I2C_MODE_SLAVE, I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN, 0);
}

esp_err_t InputLoRa::read_data(TickType_t timeout_ticks) {
    if (this->init_status != ESP_OK) {
        return this->init_status;
    }

    const TickType_t start_ticks = xTaskGetTickCount();
    while (this->rx_buffered_bytes < sizeof(LoRAData_t)) {
        const TickType_t elapsed_ticks = xTaskGetTickCount() - start_ticks;
        if (elapsed_ticks >= timeout_ticks) {
            this->rx_buffered_bytes = 0;
            return ESP_ERR_TIMEOUT;
        }

        const TickType_t remaining_ticks = timeout_ticks - elapsed_ticks;
        int bytes_read = i2c_slave_read_buffer(
            this->port,
            this->rx_buffer + this->rx_buffered_bytes,
            sizeof(LoRAData_t) - this->rx_buffered_bytes,
            remaining_ticks);

        if (bytes_read < 0) {
            this->rx_buffered_bytes = 0;
            ESP_LOGW(TAG, "Lectura I2C slave invalida: %d", bytes_read);
            return ESP_FAIL;
        }

        if (bytes_read == 0) {
            this->rx_buffered_bytes = 0;
            return ESP_ERR_TIMEOUT;
        }

        this->rx_buffered_bytes += static_cast<size_t>(bytes_read);
    }

    LoRAData_t buffer = {};
    memcpy(&buffer, this->rx_buffer, sizeof(buffer));
    this->rx_buffered_bytes = 0;

    this->x = buffer.x;
    this->y = buffer.y;
    this->brakes = buffer.brakes;
    ESP_LOGI(TAG, "Paquete I2C recibido: x=%u y=%u brakes=%u", this->x, this->y, this->brakes);
    return ESP_OK;
}

bool InputLoRa::is_ready() const {
    return this->init_status == ESP_OK;
}

esp_err_t InputLoRa::get_init_status() const {
    return this->init_status;
}

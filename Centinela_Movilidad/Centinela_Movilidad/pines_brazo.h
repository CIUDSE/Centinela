#ifndef PINES_BRAZO_H
#define PINES_BRAZO_H

// =======================================================
// Pines I2C (esclavo)
// =======================================================
#define PIN_I2C_SDA   21
#define PIN_I2C_SCL   22
#define I2C_SLAVE_ADDR_BRAZO  0x08

// =======================================================
// Driver 1 - MDD10A (Articulacion 1 / CH1)
// =======================================================
#define M1_PWM   32
#define M1_DIR   33

// =======================================================
// Driver 2 - MDD10A (Articulacion 2 / CH2)
// =======================================================
#define M2_PWM   25
#define M2_DIR   26

// =======================================================
// Driver 3 - MDD10A (Articulacion 3 / CH4)
// =======================================================
#define M3_PWM   27
#define M3_DIR   14

// =======================================================
// Config PWM (LEDC del ESP32)
// =======================================================
#define PWM_FREQ        20000   // 20kHz, fuera del rango audible
#define PWM_RESOLUTION  8       // 8 bits -> 0-255
#define PWM_CANAL_M1    0
#define PWM_CANAL_M2    1
#define PWM_CANAL_M3    2

#endif
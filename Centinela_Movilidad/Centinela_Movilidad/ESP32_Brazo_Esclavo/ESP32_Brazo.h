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
  #define A1_M1_PWM1  32
  #define A1_M1_DIR1  19
  
  #define A1_M2_PWM2  33
  #define A1_M2_DIR2  18

  // =======================================================
  // Driver 2 - MDD10A (Articulacion 2 / CH2)
  // =======================================================
  #define A2_M1_PWM1  25   
  #define A2_M1_DIR1  23   

  #define A2_M2_PWM2  26
  #define A2_M2_DIR2  17  

  // =======================================================
  // Driver 3 - MDD10A (Articulacion 3 / CH4)
  // =======================================================
  #define A3_M1_PWM1  27   
  #define A3_M1_DIR1  4  

  #define A3_M2_PWM2  13 
  #define A3_M2_DIR2  16

  // =======================================================
  // Config PWM (LEDC del ESP32)
  // =======================================================
  #define PWM_FREQ        20000
  #define PWM_RESOLUTION  8

#endif
# Motor_ESP32 - Control y flasheo

Proyecto para controlar 2 motores DC con ESP32 usando PWM. Se utiliza espress-idf para la build y esptool para el flasheo.

En esta version, el ESP32 funciona como receptor `I2C slave`. Eso significa que otro dispositivo `I2C master` le envia los datos de control a la direccion `0x08`, y este ESP32 los usa para calcular la velocidad del motor.

## Funcion principal

La velocidad se ajusta por software con funciones como estas:

```cpp
void motor_set_speed(float speed);
void motor1_linear_increase(float ts_speed, int time_ms);
```

- `speed`: rango aproximado `0-5 m/s`
- `motor`: el proyecto contempla 2 motores
- `DIR`: define el sentido de giro
- `PWM`: define la potencia aplicada al motor

## Que significa `I2C slave`

En este proyecto, el ESP32 esta configurado como `slave` I2C:

- No inicia la comunicacion.
- Espera a que el `master` le escriba datos.
- Escucha en la direccion `0x08`.
- Recibe un paquete de 5 bytes.

Formato esperado del paquete:

```cpp
typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t brakes;
} __attribute__((packed)) LoRAData_t;
```

- `x`: posicion horizontal del joystick
- `y`: posicion vertical del joystick
- `brakes`: estado de frenado

## Pines usados

Configuracion actual tomada de [main/main.cpp](e:\Scripts\C++\Motor_ESP32\main\main.cpp).

### I2C

- `SCL`: `GPIO22`
- `SDA`: `GPIO21`
- `Puerto`: `I2C_NUM_0`
- `Direccion slave`: `0x08`

### Motores

- Motor derecho:
  - `PWM`: `GPIO27`
  - `DIR`: `GPIO32`
- Motor izquierdo:
  - `PWM`: `GPIO26`
  - `DIR`: `GPIO25`

## Conexion basica

- Driver usado: `MDD10A`
- Alimentar los motores con la fuente adecuada
- Conectar `GND` del driver y del ESP32
- Conectar las entradas `PWM` y `DIR` del driver a los GPIO indicados
- Conectar `SDA`, `SCL` y `GND` entre el `master` I2C y este ESP32 `slave`
- Verificar que el driver soporte la corriente del motor

## Como funciona

- `PWM` controla la velocidad
- `DIR` controla el sentido de giro
- El ESP32 recibe `x`, `y` y `brakes` por I2C
- Luego convierte esos valores en una velocidad para el motor

## Build y flasheo

### Docker

Windows 10:

```powershell
docker run --rm -it -v "$($PWD.Path):/workspace" -w /workspace mi-espressif:v5.5.3
```

Windows 11:

```powershell
docker run --rm -it -v "%cd%:/workspace" -w /workspace mi-espressif:v5.5.3
```

### Compilacion con ESP-IDF

```powershell
idf.py set-target esp32
idf.py build
```

### Flasheo con esptool

Ejemplo usando `COM5`:

```powershell
py -m esptool --chip esp32 --port COM5 --baud 460800 --before default_reset --after hard_reset write_flash -z 0x1000 build\bootloader\bootloader.bin 0x8000 build\partition_table\partition-table.bin 0x10000 build\Prueba_ESP32.bin
```

## Donde cambiar configuracion

- Pines, direccion I2C y logica principal: [main/main.cpp](e:\Scripts\C++\Motor_ESP32\main\main.cpp)
- Recepcion I2C slave: [main/Input_LoRa.cpp](e:\Scripts\C++\Motor_ESP32\main\Input_LoRa.cpp)
- Control de motores: [main/Motor.cpp](e:\Scripts\C++\Motor_ESP32\main\Motor.cpp)

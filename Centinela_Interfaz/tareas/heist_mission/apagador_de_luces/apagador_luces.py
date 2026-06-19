# CORRE EN EL ROVER (CONTROL DE LUCES)


import socket
import sys
# Detección de entorno: Si no se encuentra la librería RPi.GPIO, se asume que es una simulación en laptop
try:
    import RPi.GPIO as GPIO
    SIMULACION = False
except ModuleNotFoundError:
    SIMULACION = True


def iniciar_luces_rover():
    # En simulación escucha de forma interna, sino escucha en toda la red
    IP_ROVER = "127.0.0.1" if SIMULACION else "0.0.0.0"
    PUERTO = 5006
    PIN_LUCES = 23  # Pin GPIO asignado al circuito de control de luces

    # Configuración de hardware solo si es la raspberry real
    if not SIMULACION:
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(PIN_LUCES, GPIO.OUT)
        GPIO.output(PIN_LUCES, GPIO.LOW)  # Estado inicial: apagadas

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((IP_ROVER, PUERTO))

    print(f"SISTEMA DE LUCES (MODO SIMULACION: {SIMULACION})")
    print(f"Red: Escuchando comandos en {IP_ROVER}:{PUERTO}")

    try:
        while True:
            data, addr = sock.recvfrom(1024)
            comando = data.decode('utf-8').strip().upper()

            print(f"\nComando recibido desde {addr[0]}: '{comando}'")

            if comando == "APAGAR":
                if SIMULACION:
                    print(f"[GPIO {PIN_LUCES}] -> LOW  (APAGADAS)")
                else:
                    GPIO.output(PIN_LUCES, GPIO.LOW)

            elif comando == "ENCENDER":
                if SIMULACION:
                    print(f"[GPIO {PIN_LUCES}] -> HIGH (ENCENDIDAS)")
                else:
                    GPIO.output(PIN_LUCES, GPIO.HIGH)
            else:
                print(f"Comando '{comando}' no reconocido.")

            print("-" * 50)

    except KeyboardInterrupt:
        print("\nApagando sistema de luces.")
    finally:
        if not SIMULACION:
            GPIO.cleanup()


if __name__ == "__main__":
    iniciar_luces_rover()

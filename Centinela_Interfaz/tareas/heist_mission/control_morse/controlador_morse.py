# CORRE EN EL ROVER (CONTROLADOR DE ACTUADORES POR SEÑALES MORSE)

import socket
import time

try:
    import RPi.GPIO as GPIO
    SIMULACION = False
except ModuleNotFoundError:
    SIMULACION = True

def iniciar_controlador_rover():
    IP_ROVER = "127.0.0.1" if SIMULACION else "0.0.0.0"
    PUERTO = 5005
    
    # Constante de tiempo base (PARIS Standard a 18 WPM)
    DIT = 0.0667   # 1 dot = 66.7 ms
    DAH = 0.2001   # 1 dash = 3 dots = 200.1 ms
    PIN_ACTUADOR = 18  # GPIO pin para el actuador (solenoide, etc)
    
    if not SIMULACION:
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(PIN_ACTUADOR, GPIO.OUT)
        GPIO.output(PIN_ACTUADOR, GPIO.LOW)
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((IP_ROVER, PUERTO))

    print(f"CONTROLADOR MORSE (SIMULACIÓN: {SIMULACION})")

    try:
        while True:
            data, addr = sock.recvfrom(1024)
            cadena_morse = data.decode('utf-8')
            
            print(f"\nDatos recibidos desde {addr[0]}: '{cadena_morse}'")
            print("-" * 50)
            
            # ReemplazO de los triples espacios de palabras por un token especial transitorio
            cadena_procesada = cadena_morse.replace('   ', ' W ')
            elementos = cadena_procesada.split(' ')
            
            for elemento in elementos:
                if not elemento: continue
                
                # Caso A: Cambio de palabra reglamentario (7 dots en total)
                if elemento == 'W':
                    if SIMULACION:
                        print(f"[ESPACIADO ENTRE PALABRAS]---------------[Espera: {DIT*7*1000:.1f}ms]")
                    time.sleep(DIT * 7)
                    
                # Caso B: Procesar una letra completa (puntos y rayas)
                else:
                    for i, simbolo in enumerate(elemento):
                        if simbolo == '.':
                            if SIMULACION: print(f"[GPIO {PIN_ACTUADOR}] -> HIGH (66.7ms)")
                            else: GPIO.output(PIN_ACTUADOR, GPIO.HIGH)
                            time.sleep(DIT)
                            
                            if SIMULACION: print(f"[GPIO {PIN_ACTUADOR}] -> LOW")
                            else: GPIO.output(PIN_ACTUADOR, GPIO.LOW)
                            
                        elif simbolo == '-':
                            if SIMULACION: print(f"[GPIO {PIN_ACTUADOR}] -> HIGH (200.1ms)")
                            else: GPIO.output(PIN_ACTUADOR, GPIO.HIGH)
                            time.sleep(DAH)
                            
                            if SIMULACION: print(f"[GPIO {PIN_ACTUADOR}] -> LOW")
                            else: GPIO.output(PIN_ACTUADOR, GPIO.LOW)
                        
                        # Espacio reglamentario entre elementos de la misma letra (1 dot)
                        if i < len(elemento) - 1:
                            time.sleep(DIT)
                            
                    # Espacio reglamentario al terminar la letra completa (3 dots)
                    if SIMULACION:
                        print(f"[ESPACIADO ENTRE LETRAS]---------------[Espera: {DIT*3*1000:.1f}ms]")
                    time.sleep(DIT * 3)
            
            print("-" * 50)
            print("Fin de transmisión")

    except KeyboardInterrupt:
        print("\nApagando controlador.")
    finally:
        if not SIMULACION:
            GPIO.cleanup()

if __name__ == "__main__":
    iniciar_controlador_rover()
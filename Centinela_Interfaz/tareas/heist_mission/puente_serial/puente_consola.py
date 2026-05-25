# CORRE EN EL ROVER (PUENTE ENTRE RED Y CONSOLA CON XLR3)
import socket
import sys
import time
import threading

def iniciar_puente_consola():
    # Detecta de forma local si la librería serial está disponible
    try:
        import serial
        libreria_disponible = True
    except ModuleNotFoundError:
        libreria_disponible = False

    hardware_real = False
    arduino_serial = None
    
    # Configuración de red y puertos
    IP_ROVER = "127.0.0.1"
    PUERTO_RED = 5007
    PUERTO_SERIAL = "COM3" if sys.platform.startswith("win") else "/dev/ttyAMA0"
    BAUD_RATE = 115200 
    
    # Intentar la conexión física solo si la librería existe
    if libreria_disponible:
        try:
            arduino_serial = serial.Serial(PUERTO_SERIAL, BAUD_RATE, timeout=0.1)
            hardware_real = True
            IP_ROVER = "0.0.0.0"  # Escucha en toda la red
            print(f"Conectado al XLR3 en {PUERTO_SERIAL}")
        except Exception as e:
            print(f"Puerto {PUERTO_SERIAL} ocupado o no disponible. INICIANDO MODO SIMULACIÓN de Consola.")
    else:
        print("Librería 'pyserial' no instalada. INICIANDO MODO SIMULACIÓN de Consola.")

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((IP_ROVER, PUERTO_RED))

    print(f"PUENTE SERIAL XLR3 (HARDWARE REAL: {hardware_real})")
    print("Esperando señal...")

    direccion_base = None
    loggeado = False
    prompt = "\ncompetencia-security-login: "

    # Hilo para leer constantemente el cable físico XLR3 (solo si se abrió con éxito)
    def leer_cable_serial_fisico():
        nonlocal direccion_base
        while hardware_real and arduino_serial and arduino_serial.is_open:
            try:
                # Verificar si hay bytes esperando en el cable físico
                if arduino_serial.in_available() > 0:
                    datos = arduino_serial.read(arduino_serial.in_available())
                    if direccion_base:
                        sock.sendto(datos, direccion_base)
            except Exception:
                break
            time.sleep(0.01)

    if hardware_real:
        threading.Thread(target=leer_cable_serial_fisico, daemon=True).start()

    # Bucle principal: Recibe lo que se tecleas desde la estacion terrena
    while True:
        data, addr = sock.recvfrom(1024)
        direccion_base = addr  # Guarda IP para saber a dónde responder
        comando_usuario = data.decode('utf-8')

        # CASO REAL: Reenviar directamente al cable de la competencia
        if hardware_real and arduino_serial and arduino_serial.is_open:
            arduino_serial.write(data)
        
        # CASO SIMULACIÓN: Consola de seguridad interactiva para pruebas de software
        else:
            comando_limpio = comando_usuario.strip()
            if not comando_limpio:
                sock.sendto(prompt.encode('utf-8'), direccion_base)
                continue
                
            if not loggeado:
                if comando_limpio == "BOVEDA2026":
                    loggeado = True
                    prompt = "\nroot@security-console:~# "
                    respuesta = "\n[SUCCESS] Acceso concedido.\nBienvenido a la terminal de seguridad.\n" + prompt
                else:
                    respuesta = "\n[ERROR] Contraseña incorrecta.\n" + prompt
            else:
                if comando_limpio == "ls":
                    respuesta = "\nclearance.txt  vault_logs/  secret_code.txt\n" + prompt
                elif comando_limpio == "cat secret_code.txt":
                    respuesta = "\n CÓDIGO DE LA BÓVEDA: 7412-9630-A\n" + prompt
                elif comando_limpio == "help":
                    respuesta = "\nComandos disponibles: ls, cat secret_code.txt, clear, logout\n" + prompt
                elif comando_limpio == "logout":
                    loggeado = False
                    prompt = "\ncompetencia-security-login: "
                    respuesta = "\nSesión cerrada.\n" + prompt
                else:
                    respuesta = f"\nbash: {comando_limpio}: comando no encontrado\n" + prompt
            
            sock.sendto(respuesta.encode('utf-8'), direccion_base)

if __name__ == "__main__":
    iniciar_puente_consola()
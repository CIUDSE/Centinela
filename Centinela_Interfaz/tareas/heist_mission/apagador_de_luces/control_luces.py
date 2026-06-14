# CORRE EN LA ESTACIÓN TERRENA

import socket
import sys


def iniciar_control_luces():
    IP_DESTINO = "127.0.0.1"  # Localhost por defecto para pruebas
    PUERTO = 5006  # Puerto para el sistema de iluminación

    # Para la IP del rover por consola al arrancar
    if len(sys.argv) > 1:
        IP_DESTINO = sys.argv[1]

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    print("CONTROL DE ILUMINACIÓN DEL ROVER")
    print(f"IP: {IP_DESTINO}")
    print(f"PUERTO: {PUERTO}")
    print("Comandos: 'APAGAR' o 'ENCENDER'")
    print("Escriba 'exit' para salir")

    while True:
        comando = input("\nIntroduzca comando de luces: ").strip().upper()
        if comando == "EXIT":
            break
        if not comando:
            continue

        sock.sendto(comando.encode('utf-8'), (IP_DESTINO, PUERTO))
        print(f"Comando '{comando}' enviado a {IP_DESTINO}:{PUERTO}")


if __name__ == "__main__":
    iniciar_control_luces()

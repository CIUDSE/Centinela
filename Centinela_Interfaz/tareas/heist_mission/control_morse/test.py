# Script de prueba para recibir datos en el puerto 5005 de codigo morse
# IP_RASPBERRY = " en app.py debe coincidir con la ip de destino del generador morse"
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("127.0.0.1", 5005))
print("Escuchando en puerto 5005...")
while True:
    data, addr = sock.recvfrom(1024)
    print(f"Recibido: {data.decode('utf-8')}")
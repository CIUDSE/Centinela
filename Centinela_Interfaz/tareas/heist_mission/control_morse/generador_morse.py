# CORRE EN LA ESTACIÓN TERRENA

import socket
import sys

# Diccionario Oficial Completo basado en ITU-R M.1677-1
DICCIONARIO_MORSE = {
    # Letras
    'A': '.-', 'B': '-...', 'C': '-.-.', 'D': '-..', 'E': '.', 'F': '..-.',
    'G': '--.', 'H': '....', 'I': '..', 'J': '.---', 'K': '-.-', 'L': '.-..',
    'M': '--', 'N': '-.', 'O': '---', 'P': '.--.', 'Q': '--.-', 'R': '.-.',
    'S': '...', 'T': '-', 'U': '..-', 'V': '...-', 'W': '.--', 'X': '-..-',
    'Y': '-.--', 'Z': '--..',
    # Números
    '1': '.----', '2': '..---', '3': '...--', '4': '....-', '5': '.....',
    '6': '-....', '7': '--...', '8': '---..', '9': '----.', '0': '-----',
    # Puntuación y Signos Oficiales ITU
    '.': '.-.-.-',  # Full stop (period)
    ',': '--..--',  # Comma
    ':': '---...',  # Colon
    '?': '..--..',  # Question mark
    "'": '.----.',  # Apostrophe
    '-': '-....-',  # Hyphen or dash
    '/': '-..-.',   # Fraction bar
    '(': '-.--.',   # Left parenthesis
    ')': '-.--.-',  # Right parenthesis
    '"': '.-..-.',  # Quotation marks
    '=': '-...-',   # Double hyphen / Equal
    '+': '.-.-.',   # Cross or addition
    '@': '.--.-.'   # Commercial at
}

def iniciar_base():
    IP_DESTINO = "127.0.0.1" # Localhost por defecto para pruebas
    PUERTO = 5005 # Puerto de destino para el controlador Morse
    
    if len(sys.argv) > 1: IP_DESTINO = sys.argv[1] # Permitir especificar IP de destino como argumento
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # La variable envia la información a traves de una dirección IP (por socket.AF_INET) y socket.DGRAM esel protocolo de transporte.
    
    print("GENERADOR MORSE") # Imprime en la terminal el nombre del programa.
    print(f"IP: {IP_DESTINO}") # Imprime en la terminal la IP elegida al inicio del código.

    while True:
        password = input("\nIntroduzca el código de la bóveda (o 'exit'): ").strip().upper()
        """
        Aquí se le da el espacio para que el usuario introduzca el código de la bóveda.
        """
        
        if password == "EXIT": break
        if not password: continue
        """
        Dentro del bucle while, se le solicita al usuario que introduzca el código de bóveda.
        Si el usuario introduce "exit", el bucle se rompe y el programa termina. Si no, el código continua.
        """
        
        # Procesar palabras completas respetando los espacios reglamentarios
        palabras = password.split(' ') # Separa las palabras por espacios para poder traducilas a Morse.
        frase_traducida = [] # Lista para almacenar las palabras traducidas a Morse.
        
        for palabra in palabras:
            letras_traducidas = [DICCIONARIO_MORSE[c] for c in palabra if c in DICCIONARIO_MORSE] # Traducir los caracteres válidos de cada palabra
            frase_traducida.append(' '.join(letras_traducidas)) # Las letras de una misma palabra se separan por un espacio (3 puntos en el rover)
        cadena_morse = '   '.join(frase_traducida) # Las palabras entre si se unen con 3 espacios para indicar cambio de palabra
        
        print(f"Traducción: {cadena_morse}") # Imprime en la terminal la traducción.
        sock.sendto(cadena_morse.encode('utf-8'), (IP_DESTINO, PUERTO))
        print(f"Transmitido a {IP_DESTINO}:{PUERTO}") # Imprime en la terminal la IP y el puerto al que se ha transmitido el código Morse.

if __name__ == "__main__":
    iniciar_base()

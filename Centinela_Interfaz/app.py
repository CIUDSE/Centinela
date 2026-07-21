# CORRE EN LA LAPTOP (ESTACIÓN TERRENA)
import socket
import sys
import os
import serial
import threading

from pathlib import Path
from flask import Flask, render_template, request, jsonify

# Librerias para gy-87
import serial.tools.list_ports

from icecream import ic

# Objeto central unificado de telemetría
datos_telemetria = {
    "pitch": 0.0, "roll": 0.0, "yaw": 0.0,
    "lat": 32.514, "lng": -117.038
}

from tareas.heist_mission.calculadora_cables import CalculadoraCables
from datetime import datetime
from openpyxl import Workbook

app = Flask(__name__)

IP_RASPBERRY = "192.168.1.50"  # Se utilizará RSTP
PUERTO_LUCES = 5006
PUERTO_SOLENOIDE = 5007

# Crear el socket UDP para enviar datos por la red
sock_enviador = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

EMERGENCIA_ACTIVA = False

calculadora: CalculadoraCables = CalculadoraCables()

MISSION_RECORDING = False
MISSION_LOG = []

PUERTO_MORSE = 5005

DICCIONARIO_MORSE = {
    'A': '.-', 'B': '-...', 'C': '-.-.', 'D': '-..', 'E': '.', 'F': '..-.',
    'G': '--.', 'H': '....', 'I': '..', 'J': '.---', 'K': '-.-', 'L': '.-..',
    'M': '--', 'N': '-.', 'O': '---', 'P': '.--.', 'Q': '--.-', 'R': '.-.',
    'S': '...', 'T': '-', 'U': '..-', 'V': '...-', 'W': '.--', 'X': '-..-',
    'Y': '-.--', 'Z': '--..',
    '1': '.----', '2': '..---', '3': '...--', '4': '....-', '5': '.....',
    '6': '-....', '7': '--...', '8': '---..', '9': '----.', '0': '-----',
    '.': '.-.-.-', ',': '--..--', ':': '---...', '?': '..--..',
    "'": '.----.', '-': '-....-', '/': '-..-.', '(': '-.--.',
    ')': '-.--.-', '"': '.-..-.', '=': '-...-', '+': '.-.-.', '@': '.--.-.'
}

@app.route('/')
def index():
    return render_template('index.html')


@app.route('/api/emergencia', methods=['POST'])
def gestionar_emergencia():
    global EMERGENCIA_ACTIVA

    data = request.get_json()
    accion = data.get('accion', '').upper()

    if accion == "ACTIVAR":
        EMERGENCIA_ACTIVA = True
        print("🚨 [EMERGENCIA] ¡Estación Terrena Bloqueada!")
        try:
            # Mandamos comando de apagado inmediato al Rover por seguridad
            sock_enviador.sendto(b"APAGAR", (IP_RASPBERRY, PUERTO_LUCES))
            return jsonify({"status": "emergency_active", "mensaje": "E-STOP Activado. Rover apagado."}), 200
        except Exception as e:
            return jsonify({"status": "error", "mensaje": f"Bloqueo local activo, error UDP: {e}"}), 500

    elif accion == "DESACTIVAR":
        EMERGENCIA_ACTIVA = False
        print("✅ [SISTEMA] Emergencia desactivada. Estación liberada.")
        return jsonify({"status": "ready", "mensaje": "Sistema reactivado correctamente."}), 200

    return jsonify({"status": "error", "mensaje": "Acción de emergencia inválida"}), 400


@app.route('/api/luces', methods=['POST'])
def enviar_al_rover():
    global EMERGENCIA_ACTIVA

    # Escudo de protección: Si la emergencia está activa, se bloquea el método y no se envía nada por UDP
    if EMERGENCIA_ACTIVA:
        print("❌ [BLOQUEADO] Intento de usar luces rechazado por E-STOP.")
        return jsonify({"status": "blocked", "mensaje": "⚠️ ACCIÓN RECHAZADA: El Rover está en Parada de Emergencia."}), 403

    data = request.get_json()
    comando = data.get('comando', '').upper()

    if comando in ['ENCENDER', 'APAGAR']:
        try:
            # Mandamos el comando DIRECTO a la IP de la Raspberry Pi
            sock_enviador.sendto(comando.encode('utf-8'),
                                 (IP_RASPBERRY, PUERTO_LUCES))
            print(
                f"[WEB] Comando '{comando}' enviado al Rover ({IP_RASPBERRY}:{PUERTO_LUCES})")
            return jsonify({"status": "success", "mensaje": "Comando enviado al Rover"}), 200
        except Exception as e:
            return jsonify({"status": "error", "mensaje": str(e)}), 500

    return jsonify({"status": "error", "mensaje": "Comando inválido"}), 400


@app.route('/api/cables', methods=['POST'])
def analizar_cables():
    data = request.get_json()
    entrada = data.get('cables', '')

    try:
        lista_cables = calculadora.procesar_entrada(entrada)

        if not lista_cables:
            return jsonify({"status": "error", "mensaje": "Entrada vacía. Ingresa los colores separados por coma."}), 400

        calculadora.analizar_panel(lista_cables)

        numero = list(calculadora.cable_a_cortar.keys())[0]
        color = list(calculadora.cable_a_cortar.values())[0]

        return jsonify({
            "status": "success",
            "cable_numero": numero,
            "cable_color": color,
            "cables_analizados": lista_cables
        }), 200

    except ValueError as e:
        return jsonify({"status": "error", "mensaje": str(e)}), 400
    except Exception as e:
        return jsonify({"status": "error", "mensaje": f"Error inesperado: {e}"}), 500


@app.route('/api/solenoide', methods=['POST'])
def controlar_solenoide():
    global EMERGENCIA_ACTIVA

    # Escudo de protección contra E-STOP
    if EMERGENCIA_ACTIVA:
        print("❌ [BLOQUEADO] Intento de activar solenoide rechazado por E-STOP.")
        return jsonify({"status": "blocked", "mensaje": "⚠️ ACCIÓN RECHAZADA: El Rover está en Parada de Emergencia."}), 403

    data = request.get_json()
    comando = data.get('comando', '')

    if comando == 'PULSO_RAPIDO':
        try:
            # Reutiliza el sock_enviador para lanzar el paquete UDP al puerto 5007 de la Raspi
            sock_enviador.sendto(
                b"PULSO_RAPIDO", (IP_RASPBERRY, PUERTO_SOLENOIDE))
            print(
                f"[WEB] ¡PULSO_RAPIDO enviado al Solenoide! ({IP_RASPBERRY}:{PUERTO_SOLENOIDE})")
            return jsonify({"status": "success", "mensaje": "Pulso enviado de forma segura"}), 200
        except Exception as e:
            return jsonify({"status": "error", "mensaje": f"Error de red UDP: {e}"}), 500

    return jsonify({"status": "error", "mensaje": "Comando de solenoide desconocido"}), 400


@app.route('/api/imu', methods=['GET'])
def obtener_imu():
    return jsonify(datos_telemetria)

# API Única para la Estación Terrena --- Para el mapa   
@app.route('/api/telemetria', methods=['GET'])
def obtener_telemetria():
    return datos_telemetria


# Almacenamiento de coordenadas
@app.route("/api/mission/start", methods=["POST"])
def mission_start():

    global MISSION_RECORDING
    global MISSION_LOG

    MISSION_RECORDING = True
    MISSION_LOG = []

    print("[MISSION] Recording started.")

    return jsonify({"status": "success"})


@app.route("/api/mission/stop", methods=["POST"])
def mission_stop():

    global MISSION_RECORDING
    save_mission_to_excel()
    MISSION_RECORDING = False

    print("[MISSION] Recording stopped.")

    return jsonify({"status": "success"})


@app.route("/api/mission/point", methods=["POST"])
def mission_point():

    global MISSION_LOG

    if not MISSION_RECORDING:
        return jsonify({"status": "ignored"})

    data = request.get_json()

    MISSION_LOG.append({
        "Tiempo": datetime.now().strftime("%H:%M:%S"),
        "Latitud": data["latitude"],
        "Longitud": data["longitude"],
        "Tipo": "Ruta"
    })

    return jsonify({"status": "success"})


@app.route("/api/mission/waypoint", methods=["POST"])
def mission_waypoint():

    global MISSION_LOG

    if not MISSION_RECORDING:
        return jsonify({"status": "ignored"})

    data = request.get_json()

    MISSION_LOG.append({

        "Tiempo": datetime.now().strftime("%H:%M:%S"),
        "Latitud": data["latitude"],
        "Longitud": data["longitude"],
        "Tipo": "Waypoint"

    })

    return jsonify({"status": "success"})

@app.route('/api/morse', methods=['POST'])
def enviar_morse():
    if EMERGENCIA_ACTIVA:
        return jsonify({"status": "blocked", "mensaje": "E-STOP activo."}), 403

    data = request.get_json()
    codigo = data.get('codigo', '').strip().upper()

    if not codigo:
        return jsonify({"status": "error", "mensaje": "Codigo vacio."}), 400

    palabras = codigo.split(' ')
    frase_traducida = []
    caracteres_invalidos = []

    for palabra in palabras:
        letras = []
        for c in palabra:
            if c in DICCIONARIO_MORSE:
                letras.append(DICCIONARIO_MORSE[c])
            else:
                caracteres_invalidos.append(c)
        if letras:
            frase_traducida.append(' '.join(letras))

    if caracteres_invalidos:
        return jsonify({"status": "error", "mensaje": f"Caracteres no validos: {caracteres_invalidos}"}), 400

    cadena_morse = '   '.join(frase_traducida)

    try:
        sock_enviador.sendto(cadena_morse.encode('utf-8'), (IP_RASPBERRY, PUERTO_MORSE))
        print(f"[MORSE] Enviado: {cadena_morse}")
        return jsonify({"status": "success", "cadena_morse": cadena_morse}), 200
    except Exception as e:
        return jsonify({"status": "error", "mensaje": str(e)}), 500

def save_mission_to_excel():

    # Create the "missions" folder if it doesn't exist
    missions_folder = Path("missions")
    missions_folder.mkdir(exist_ok=True)

    # Create the workbook
    wb = Workbook()
    ws = wb.active
    ws.title = "Mission"

    # Column headers
    ws.append([
        "Tiempo",
        "Latitud",
        "Longitud",
        "Tipo"
    ])

    # Write mission data
    for row in MISSION_LOG:
        ws.append([
            row["Tiempo"],
            row["Latitud"],
            row["Longitud"],
            row["Tipo"]
        ])

    # Save inside the missions folder
    filename = missions_folder / f"Mission_{datetime.now():%Y%m%d_%H%M%S}.xlsx"

    wb.save(filename)

    print(f"[MISSION] Mission saved to {filename}")

if __name__ == "__main__":
    # Si quieres pasar la IP de la Raspberry por consola al prender la web:
    # Ejemplo: python app.py 192.168.1.55
    if len(sys.argv) > 1:
        IP_RASPBERRY = sys.argv[1]

    print(
        f"Interfaz iniciada. Los comandos de luces se enviarán a la Raspberry en: {IP_RASPBERRY}:{PUERTO_LUCES}")
    app.run(debug=True, host='0.0.0.0', port=5000)

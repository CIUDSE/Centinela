# CORRE EN LA LAPTOP (ESTACIÓN TERRENA)
import socket
import sys
import os
import serial
import threading

from pathlib import Path
from flask import Flask, render_template, request, jsonify

from tareas.heist_mission.calculadora_cables import CalculadoraCables
from datetime import datetime
from openpyxl import Workbook

app = Flask(__name__)

SERIAL_PORT = "COM5"      # Change to your COM port
BAUDRATE = 115200         # Must match Serial.begin()

latest_telemetry = {
    "latitude": 0,
    "longitude": 0,
    "altitude": 0,
}


def serial_reader():

    global latest_telemetry

    try:

        ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)

        print(f"[SERIAL] Connected to {SERIAL_PORT}")

        while True:

            line = ser.readline().decode("utf-8").strip()

            if not line:
                continue

            try:

                parts = line.split(",")

                latest_telemetry["latitude"] = float(parts[0])
                latest_telemetry["longitude"] = float(parts[1])
                latest_telemetry["altitude"] = float(parts[2])

                print(latest_telemetry)

            except Exception:

                print(f"[SERIAL] Invalid packet: {line}")

    except Exception as e:

        print(f"[SERIAL ERROR] {e}")


IP_RASPBERRY = "192.168.1.50"  # Se utilizará RSTP
PUERTO_LUCES = 5006
PUERTO_SOLENOIDE = 5007

# Crear el socket UDP para enviar datos por la red
sock_enviador = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

EMERGENCIA_ACTIVA = False

calculadora: CalculadoraCables = CalculadoraCables()

MISSION_RECORDING = False
MISSION_LOG = []


# -------- RUTEO DE PAGINAS --------

@app.route('/')
def index():
    return render_template('index.html', pagina='home')


@app.route('/heist')
def heist():
    return render_template('index.html', pagina='heist')


@app.route('/snackRun')
def snack_run():
    return render_template('index.html', pagina='snackRun')


# -------- FIN DE RUTEO DE PAGINAS --------

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


@app.route("/api/telemetry/latest")
def telemetry():

    return jsonify(latest_telemetry)


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

    threading.Thread(
        target=serial_reader,
        daemon=True
    ).start()

    print(
        f"Interfaz iniciada. Los comandos de luces se enviarán a la Raspberry en: {IP_RASPBERRY}:{PUERTO_LUCES}")
    app.run(debug=True, host='0.0.0.0', port=5000)

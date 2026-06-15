# CORRE EN LA LAPTOP (ESTACIÓN TERRENA)
from flask import Flask, render_template, request, jsonify
import socket
import sys

app = Flask(__name__)

IP_RASPBERRY = "192.168.1.50" # Se utilizará RSTP
PUERTO_LUCES = 5006

# Crear el socket UDP para enviar datos por la red
sock_enviador = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

EMERGENCIA_ACTIVA = False


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


if __name__ == "__main__":
    # Si quieres pasar la IP de la Raspberry por consola al prender la web:
    # Ejemplo: python app.py 192.168.1.55
    if len(sys.argv) > 1:
        IP_RASPBERRY = sys.argv[1]

    print(
        f"Interfaz iniciada. Los comandos de luces se enviarán a la Raspberry en: {IP_RASPBERRY}:{PUERTO_LUCES}")
    app.run(debug=True, host='0.0.0.0', port=5000)

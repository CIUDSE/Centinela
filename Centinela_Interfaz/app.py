# CORRE EN LA LAPTOP (ESTACIÓN TERRENA)
import socket
import sys
import os
import serial
import threading

from pathlib import Path
from flask import Flask, render_template, request, jsonify

# Librerias para gy-87
import serial
import serial.tools.list_ports
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, QLabel, QComboBox, QPushButton, QFrame)
from PyQt5.QtCore import QThread, pyqtSignal, pyqtSlot


import serial
import threading
from icecream import ic

# 🛠️ AJUSTA ESTOS PUERTOS SEGÚN TUS PLACAS REALES (pueden ser 'COM16' y 'COM17' en Windows)
PUERTO_IMU = 'COM16'  
PUERTO_GPS = 'COM12'  
BAUDIOS = 115200

# Objeto central unificado de telemetría
datos_telemetria = {
    "pitch": 0.0, "roll": 0.0, "yaw": 0.0,
    "lat": 32.514, "lng": -117.038
}

# Hilo 1: Lee el ESP32 del Giroscopio (Transmite: pitch,roll,yaw)
def lectura_imu_background():
    global datos_telemetria
    try:
        ser_imu = serial.Serial(PUERTO_IMU, BAUDIOS, timeout=1)
        ser_imu.reset_input_buffer()
        ic(f"📡 Conectado exitosamente al ESP32 de la IMU en {PUERTO_IMU}")
        while True:
            if ser_imu.in_waiting > 0:
                linea = ser_imu.readline().decode('utf-8', errors='ignore').strip()
                if linea and not any(k in linea for k in ["Calibrando", "completa"]):
                    partes = linea.split(',')
                    if len(partes) >= 2:
                        try:
                            datos_telemetria["pitch"] = float(partes[0])
                            datos_telemetria["roll"] = float(partes[1])
                            # Aplicamos la inversión del Yaw de una vez aquí si fuera necesario
                            datos_telemetria["yaw"] = -float(partes[2]) if len(partes) >= 3 else 0.0
                        except ValueError:
                            continue
    except Exception as e:
        ic(f"❌ Error crítico en puerto IMU: {e}")

# Hilo 2: Lee el ESP32 del GPS (Transmite: latitud,longitud)
def lectura_gps_background():
    global datos_telemetria
    try:
        ser_gps = serial.Serial(PUERTO_GPS, BAUDIOS, timeout=1)
        ser_gps.reset_input_buffer()
        ic(f"🛰️ Conectado exitosamente al ESP32 del GPS en {PUERTO_GPS}")
        while True:
            if ser_gps.in_waiting > 0:
                linea = ser_gps.readline().decode('utf-8', errors='ignore').strip()
                if linea:
                    partes = linea.split(',')
                    if len(partes) >= 2:
                        try:
                            nueva_lat = float(partes[9])
                            nueva_lng = float(partes[10])
                            # Evitamos sobrescribir con ceros si el GPS pierde señal temporalmente
                            if nueva_lat != 0.0 and nueva_lng != 0.0:
                                datos_telemetria["lat"] = nueva_lat
                                datos_telemetria["lng"] = nueva_lng
                        except ValueError:
                            continue
    except Exception as e:
        ic(f"❌ Error crítico en puerto GPS: {e}")

# Lanzamos ambos hilos de forma paralela e independiente
threading.Thread(target=lectura_imu_background, daemon=True).start()
threading.Thread(target=lectura_gps_background, daemon=True).start()


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


# Inicio de GY-87
# =========================================================================
# CLASE 1: HILO DE LECTURA SERIAL (QThread)
# Esta clase corre de forma independiente en segundo plano.
# Evita que el bucle de tu interfaz principal de Centinela se trabe.
# =========================================================================
@app.route('/api/imu', methods=['GET'])
def obtener_imu():
    return datos_imu

class SerialReaderThread(QThread):
   # Definimos las señales para comunicarnos de forma segura con la GUI
   # imu_signal emitirá una tupla con los 8 datos del GY-87:
   # (pitch, roll, ax, ay, az, gx, gy, gz)
   imu_signal = pyqtSignal(tuple)
   status_signal = pyqtSignal(str, bool) # (mensaje, es_exito)

   def __init__(self, port, baudrate=115200):
       super().__init__()
       self.port = port
       self.baudrate = baudrate
       self.running = True
       self.serial_port = None

   def run(self):
       try:
           # Intentamos abrir el puerto serial elegido
           self.serial_port = serial.Serial(self.port, self.baudrate, timeout=1)
           self.status_signal.emit(f"Conectado exitosamente a {self.port}", True)
       except Exception as e:
           self.status_signal.emit(f"Error de conexión: {str(e)}", False)
           return

       while self.running:
           if self.serial_port and self.serial_port.is_open:
               try:
                   # Leemos una línea completa terminada en salto de línea (\n)
                   linea = self.serial_port.readline().decode('utf-8', errors='ignore').strip()
                   if linea:
                       # Buscamos omitir texto transitorio de calibración
                       if "Calibrando" in linea or "completa" in linea:
                           continue
                      
                       # Dividimos la trama CSV separada por comas
                       valores = linea.split(',')
                      
                       # El sensor GY-87 transmite exactamente 8 parámetros
                       if len(valores) == 8:
                           # Convertimos la lista de strings a flotantes
                           valores_float = tuple(map(float, valores))
                           # Emitimos la tupla para que la capture la interfaz
                           self.imu_signal.emit(valores_float)
               except Exception as e:
                   self.status_signal.emit(f"Error de lectura serial: {str(e)}", False)
                   break
           self.msleep(10) # Pausa de 10ms (~100Hz) para no saturar el hilo

   def stop(self):
       self.running = False
       if self.serial_port and self.serial_port.is_open:
           self.serial_port.close()
       self.wait()

# =========================================================================
# CLASE 2: INTERFAZ DE CENTINELA (MainWindow)
# Aquí mostramos los datos recolectados por el lector serial.
# =========================================================================
class MainWindow(QMainWindow):
   def __init__(self):
       super().__init__()
       self.setWindowTitle("Centinela - Visualización y Lectura de Telemetría GY-87")
       self.resize(800, 500)
      
       # Guardaremos una referencia al hilo lector
       self.hilo_serial = None
      
       # Paleta Aeroespacial Oscura
       self.setStyleSheet("""
           QMainWindow { background-color: #0d1117; }
           QLabel { color: #f0f6fc; font-family: 'Segoe UI', Arial; font-size: 14px; }
           #title_section { font-size: 16px; font-weight: bold; color: #58a6ff; border-bottom: 1px solid #30363d; padding-bottom: 5px; }
           #card { background-color: #161b22; border: 1px solid #30363d; border-radius: 6px; padding: 12px; }
           #card_title { font-size: 11px; font-weight: bold; color: #8b949e; text-transform: uppercase; }
           #card_val { font-size: 22px; font-weight: bold; font-family: monospace; color: #56b419; }
           #console { background-color: #010409; border: 1px solid #21262d; border-radius: 4px; padding: 8px; font-family: monospace; font-size: 11px; }
       """)

       self.init_ui()

   def init_ui(self):
       widget_central = QWidget()
       self.setCentralWidget(widget_central)
       layout_principal = QVBoxLayout(widget_central)
       layout_principal.setSpacing(15)

       # ---------------- 1. PANEL DE CONEXIÓN ----------------
       lbl_con = QLabel("CONEXIÓN DEL SENSOR GY-87")
       lbl_con.setObjectName("title_section")
       layout_principal.addWidget(lbl_con)

       layout_con = QHBoxLayout()
       self.combo_puertos = QComboBox()
       self.refrescar_puertos()
       layout_con.addWidget(self.combo_puertos)

       self.btn_conectar = QPushButton("INICIAR LECTURA")
       self.btn_conectar.setStyleSheet("background-color: #238636; color: white; font-weight: bold; padding: 6px 15px; border-radius: 4px;")
       self.btn_conectar.clicked.connect(self.conmutar_conexion)
       layout_con.addWidget(self.btn_conectar)
       layout_principal.addLayout(layout_con)

       # ---------------- 2. PANEL DE LECTURAS NUMÉRICAS ----------------
       lbl_telemetria = QLabel("TELEMETRÍA EN TIEMPO REAL")
       lbl_telemetria.setObjectName("title_section")
       layout_principal.addWidget(lbl_telemetria)

       # Contenedor para tarjetas
       layout_tarjetas = QHBoxLayout()
      
       # Tarjeta 1: Orientación (Filtro Complementario)
       self.lbl_val_pitch = self.crear_tarjeta(layout_tarjetas, "PITCH (CABECEO)", "0.00°")
       self.lbl_val_roll = self.crear_tarjeta(layout_tarjetas, "ROLL (ALABEO)", "0.00°")
      
       # Tarjeta 2: Aceleración
       self.lbl_val_ax = self.crear_tarjeta(layout_tarjetas, "ACEL. X ($m/s^2$)", "0.00")
       self.lbl_val_ay = self.crear_tarjeta(layout_tarjetas, "ACEL. Y ($m/s^2$)", "0.00")
       self.lbl_val_az = self.crear_tarjeta(layout_tarjetas, "ACEL. Z ($m/s^2$)", "0.00")
      
       layout_principal.addLayout(layout_tarjetas)

       # ---------------- 3. CONSOLA DE DIAGNÓSTICO ----------------
       self.lbl_consola = QLabel("System Log: Esperando inicio de conexión...")
       self.lbl_consola.setObjectName("console")
       self.lbl_consola.setWordWrap(True)
       layout_principal.addWidget(self.lbl_consola)


   def crear_tarjeta(self, parent_layout, titulo, valor_default):
       """Genera un componente visual estructurado para mostrar datos."""
       frame = QFrame()
       frame.setObjectName("card")
       vbox = QVBoxLayout(frame)
      
       lbl_t = QLabel(titulo)
       lbl_t.setObjectName("card_title")
       lbl_v = QLabel(valor_default)
       lbl_v.setObjectName("card_val")
      
       vbox.addWidget(lbl_t)
       vbox.addWidget(lbl_v)
       parent_layout.addWidget(frame)
       return lbl_v


   def refrescar_puertos(self):
       """Busca y enlista todos los puertos seriales activos en tu ordenador."""
       self.combo_puertos.clear()
       puertos = [port.device for port in serial.tools.list_ports.comports()]
       if puertos:
           self.combo_puertos.addItems(puertos)
       else:
           self.combo_puertos.addItem("No hay puertos detectados")


   def conmutar_conexion(self):
       """Inicia o detiene el lector del puerto serial."""
       if self.hilo_serial is not None and self.hilo_serial.isRunning():
           self.hilo_serial.stop()
           self.hilo_serial = None
           self.btn_conectar.setText("INICIAR LECTURA")
           self.btn_conectar.setStyleSheet("background-color: #238636; color: white; font-weight: bold; padding: 6px 15px;")
           self.mostrar_status("Adquisición detenida por el usuario.", False)
       else:
           puerto_seleccionado = self.combo_puertos.currentText()
           if "No hay" in puerto_seleccionado:
               self.mostrar_status("Error: Selecciona un puerto serial válido.", False)
               return


           # Inicializamos e iniciamos el hilo
           self.hilo_serial = SerialReaderThread(port=puerto_seleccionado)
           self.hilo_serial.imu_signal.connect(self.procesar_datos_imu)
           self.hilo_serial.status_signal.connect(self.mostrar_status)
           self.hilo_serial.start()


           self.btn_conectar.setText("DETENER LECTURA")
           self.btn_conectar.setStyleSheet("background-color: #da3633; color: white; font-weight: bold; padding: 6px 15px;")


   @pyqtSlot(tuple)
   def procesar_datos_imu(self, datos):
       """Recibe la tupla del hilo serial y actualiza las etiquetas de la UI."""
       pitch, roll, ax, ay, az, gx, gy, gz = datos


       # Actualizamos las tarjetas de texto
       self.lbl_val_pitch.setText(f"{pitch:+.2f}°")
       self.lbl_val_roll.setText(f"{roll:+.2f}°")
       self.lbl_val_ax.setText(f"{ax:+.2f}")
       self.lbl_val_ay.setText(f"{ay:+.2f}")
       self.lbl_val_az.setText(f"{az:+.2f}")


   @pyqtSlot(str, bool)
   def mostrar_status(self, mensaje, es_exito):
       color = "#58a6ff" if es_exito else "#ff7b72"
       self.lbl_consola.setText(f"System Log: <span style='color: {color};'>{mensaje}</span>")


   def closeEvent(self, event):
       """Apaga de manera limpia el puerto serie al cerrar la app para evitar bloquear el puerto COM."""
       if self.hilo_serial is not None:
           self.hilo_serial.stop()
       event.accept()
# Fin de GY-87


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

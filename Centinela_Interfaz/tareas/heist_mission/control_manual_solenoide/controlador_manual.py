# 1. TITULO O DESCRIPCION DEL ARCHIVO

# Controlador de Pulso Manual del Solenoide (Corre en el Rover)
# Proposito: Recibir comandos de pulso desde la interfaz y activar el solenoide durante 300ms de forma segura usando un hilo separado.
# Puerto: 5007 (independiente del canal Morse en 5005)

# 2. IMPORTS

import socket
import time
import threading
from typing import Protocol

try:
    import RPi.GPIO as GPIO
    SIMULACION: bool = False
except ModuleNotFoundError:
    SIMULACION: bool = True


# 3. PROTOCOL O INTERFACES

class ControladorPulsoProtocol(Protocol):
    """
    Contrato minimo para cualquier clase encargada de ejecutar pulsos individuales sobre un actuador fisico del rover.
    """

    def iniciar_servidor(self) -> None:
        """Inicia el socket UDP y escucha comandos de pulso entrantes."""
        ...

    def disparar_pulso(self) -> None:
        """Ejecuta un pulso unico de duracion fija sobre el actuador."""
        ...

# 4. FUNCIONES AUXILIARES

def ejecutar_pulso_en_hilo(pin: int, duracion: float) -> None:
    """
    Activa y desactiva el pin GPIO en un hilo separado para no bloquear el bucle principal del servidor UDP.

    Parameters
    ----------
    pin : int
        Numero de pin GPIO (BCM) conectado al solenoide.
    duracion : float
        Tiempo en segundos que el solenoide permanece activo.
    """
    if not SIMULACION:
        GPIO.output(pin, GPIO.HIGH)
    else:
        print("[HARDWARE] -> SOLENOIDE ACTIVO (HIGH)")

    time.sleep(duracion)

    if not SIMULACION:
        GPIO.output(pin, GPIO.LOW)
    else:
        print("[HARDWARE] -> SOLENOIDE APAGADO (LOW)")

# 5. CLASE PRINCIPAL

class ControladorPulsoRover(ControladorPulsoProtocol):
    """
    Escucha comandos UDP desde la interfaz web y ejecuta pulsos individuales sobre el solenoide sin bloquear el hilo principal.

    Attributes
    ----------
    pin : int
        Numero de pin GPIO (BCM) conectado al solenoide.
    puerto : int
        Puerto UDP donde se escuchan los comandos entrantes.
    duracion_pulso : float
        Duracion fija del pulso en segundos.
    """

    # Constantes de clase
    PIN: int = 18
    PUERTO: int = 5007
    DURACION_PULSO: float = 0.300  # 300ms, tiempo seguro para oprimir un boton

    def __init__(self) -> None:
        self.pin: int = self.PIN
        self.puerto: int = self.PUERTO
        self.duracion_pulso: float = self.DURACION_PULSO

        self._configurar_hardware()

    # Metodos publicos                                                    

    def iniciar_servidor(self) -> None:
        """
        Abre el socket UDP y entra al bucle principal de escucha.

        Cada vez que llega el comando 'PULSO_RAPIDO', lanza el pulso en un hilo separado para no bloquear la recepcion de nuevos comandos.

        Raises
        ------
        socket.error
            Si el puerto ya esta ocupado o no se puede enlazar.
        """
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind(("0.0.0.0", self.puerto))

        print(f"Controlador de pulso activo en puerto {self.puerto} (SIMULACION: {SIMULACION})")

        try:
            while True:
                data, addr = sock.recvfrom(512)
                mensaje = data.decode('utf-8').strip()

                if mensaje == "PULSO_RAPIDO":
                    print(f"Comando recibido desde {addr[0]}. Disparando pulso...")
                    self.disparar_pulso()

        except KeyboardInterrupt:
            print("\nApagando controlador.")
        finally:
            sock.close()
            if not SIMULACION:
                GPIO.cleanup()

    def disparar_pulso(self) -> None:
        """
        Lanza el pulso del solenoide en un hilo separado.
        El uso de threading evita que el time.sleep del pulso bloquee el bucle principal, permitiendo recibir nuevos comandos UDP mientras el solenoide esta activo.
        """
        hilo = threading.Thread(
            target=ejecutar_pulso_en_hilo,
            args=(self.pin, self.duracion_pulso)
        )
        hilo.start()

    # Metodos privados                                                     

    def _configurar_hardware(self) -> None:
        """
        Inicializa el pin GPIO del solenoide si no esta en modo simulacion.
        """
        if not SIMULACION:
            GPIO.setmode(GPIO.BCM)
            GPIO.setup(self.pin, GPIO.OUT)
            GPIO.output(self.pin, GPIO.LOW)

# 6. EJEMPLO DE USO
if __name__ == "__main__":
    controlador = ControladorPulsoRover()
    controlador.iniciar_servidor()
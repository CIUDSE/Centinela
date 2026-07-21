# CORRE EN LA ESTACIÓN TERRENA
import socket
import sys
import threading # Tener cuidado con threading ya que se ejecutara la tarea en el segundo hilo
import time
from typing import Protocol, Optional
from icecream import ic
class RoverProtocol(Protocol):
    def __init__(self, ip: str, puerto: int) -> None:
        ...

    def guardar_log(self, tipo:str, mensaje:str) -> None:
        ...

    def escuchar_rover(self) -> None:
        ...

    def enviar_comando(self, comando: str) -> None:
        ...
    
    def cerrar_conexion(self) -> None:
        ...

    def iniciar_control_base(self, input_command:Optional[str] = "") -> None:
        ...
class RoverConnection(RoverProtocol):
    """
    Gestiona la comunicación UDP entre la estación terrena y el rover.

    La clase encapsula el envío y recepción de comandos, el monitoreo de
    la conexión y el registro de toda la comunicación en un archivo de log.
    
    Atributes:
    ----------
    `ip`:str Dirección IP del puente inalámbrico.
    `puerto`:int Puerto UDP utilizado para la comunicación.

    """

    
    def __init__(self, ip: str, puerto: int) -> None:
        self.ip = ip
        self.puerto = puerto
        self.sock:socket.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.settimeout(1)  # Establece un tiempo de espera para la conexión

        self.activo = True

    def guardar_log(self, tipo: str, mensaje: str) -> None:
        """
        Guarda un evento de comunicación en el archivo de log con un timestamp.

        Paramters
        ----------
        `tipo`:str Tipo de mensaje (por ejemplo, "TX" para transmisión, "RX" para recepción).
        `mensaje`:str Contenido del mensaje a registrar.
        """
        
        hora = time.strftime("%Y-%m-%d %H:%M:%S")
        
        with open("rover_log.txt", "a", encoding="utf-8") as archivo:
            archivo.write(f"[{hora}] {tipo}: {mensaje}\n")

    def escuchar_rover(self) -> None:
        """
        Escucha continuamente los paquetes enviados por el rover

        Cada mensaje recibido se imprime en pantalla, se registra en el archivo log y actualiza el tiempo del último paquete recibido.

        Parameters
        ----------
        `None`

        Returns
        ----------
        `None`
        """
        
        while self.activo:
            try:
                data, _ = self.sock.recvfrom(4096)
                mensaje = data.decode('utf-8', errors='ignore')
                print(mensaje, end='', flush=True)
                self.guardar_log("RX", mensaje.strip())
            except socket.timeout:
                continue
            except OSError:
                    break

            except Exception as e:
                self.guardar_log("ERROR", f"Error al recibir datos: {e}")
                break



    def enviar_comando(self, comando: str) -> None:
        """
        Envia un comando al rover mediante UDP

        Parameters
        ----------
        `comando`:str Comando que será enviado al rover.   
        """
        
        try:
            self.sock.sendto(
                f"{comando}\n".encode("utf-8"),
                (self.ip, self.puerto)
            )

            self.guardar_log("TX", comando)

        except OSError:
            pass

    def cerrar_conexion(self) -> None:
        """
        Finaliza la comunicación y libera el socket UDP
        """
        self.activo = False
        try:
            self.sock.close()
        except Exception:
            pass

    def iniciar_control_base(self, input_command:Optional[str] = "") -> None:
        """
        Inicia la terminal de control de la estación terrena, estableciendo la comunicación con el rover a través del puente inalámbrico.

        Crea la conexión con el rover, inicia los hilos de recepción, y mantiene un ciclo continuo para enviar comandos escritos por el operador.

        Returns
        ----------
        `None`
        """
        IP_DESTINO:str = "127.0.0.1"
        PUERTO:int = 5008

        if len(sys.argv) > 1:
            self.ip = sys.argv[1]

        if self.ip and self.puerto:

            ic("====================================")
            ic(" Terminal de Control del Rover")
            ic("====================================")
            ic(f"Puente inalámbrico: {self.ip}:{self.puerto}")
            ic("Escriba 'exit' para salir.\n")

            #Se inicia el primer hilo#
            hilo_receptor = threading.Thread(target=self.escuchar_rover, daemon=True)
            hilo_receptor.start()

            time.sleep(0.2)

            #Despierta al rover para establecer la comunicación
            self.enviar_comando("")

            try:
                while True:
                    comando = input_command if input_command else input(">> ")
                    
                    if comando.lower() == "exit":
                        break

                    self.enviar_comando(comando)

            except KeyboardInterrupt:
                pass

            finally:
                self.cerrar_conexion()
                ic("\nConexión finalizada correctamente.")

if __name__ == "__main__":

    IP_PUENTE = "127.0.0.1"  # pruebas locales
    # IP_PUENTE = "192.168.1.50"  # Raspberry real

    rover = RoverConnection(
        IP_PUENTE,
        5008
    )

    rover.iniciar_control_base()



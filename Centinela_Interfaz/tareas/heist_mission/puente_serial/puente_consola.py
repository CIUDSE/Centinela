# CORRE EN EL ROVER (PUENTE ENTRE RED Y CONSOLA CON XLR3)
import socket
import sys
import time
import threading
from typing import Protocol, Optional, Any, List, Callable, Literal
from icecream import ic
import serial

class SerialInterface(Protocol):
    def __init__(self, port: str, baudrate: int, timeout: float) -> None:
        ...
    
    def _exceptions(message: Literal["Libreria no encontrada","Error al realizar la conexión física", "Error al iniciar el puente serial","Error en la lectura del cable serial físico"]) -> Callable[...,Any]:
        ...
    
    def import_library(self) -> None:
        ...

    def conexion_fisica(self) -> None:
        ...
    

class Puente_Serial(SerialInterface):
    """
    A class that implements a serial bridge between a rover and a console using the XLR3 protocol. It can operate in both real hardware mode and simulation mode, depending on the availability of the physical serial connection.

    Attributes
    ----------
    `ip_rover` : str
        The IP address of the rover to which the bridge will connect. Default is "127.0.0.1".
    `port` : int
        The port number for the UDP socket connection. Default is 5008.
    `baudrate` : int
        The baud rate for the serial connection. Default is 115200.
    `serial_port` : str
        The serial port to which the XLR3 device is connected. Default is "COM3".
    `timeout` : float
        The timeout for the serial connection in seconds. Default is 0.1.


    """
    def __init__(self, ip_rover:Optional[str] = "127.0.0.1", port: Optional[int] = 5008, baudrate:Optional[int] = 115200,serial_port:Optional[str] = "/dev/ttyUSB0", timeout: Optional[float] = 0.1) -> None:
        self.ip_rover:str = ip_rover
        self.port = port
        self.baudrate = baudrate
        self.serial_port = serial_port
        self.timeout = timeout
        self.serial_lib = None
        self.serial_connection = None
        self.arduino_serial = None
        self.hardware_real = False
        self.socket = None
        self.direccion_base = None
        self.loggeado:bool = False
        self.prompt:str = ""

    def _exceptions(message:Literal["Libreria no encontrada","Error al realizar la conexión física", "Error al iniciar el puente serial","Error en la lectura del cable serial físico"]) -> Callable[..., Any]:
        def decorator(func: Callable[...,Any])-> Callable[...,Any]:
            def wrapper(*args, **kwargs) -> Any:
                try:
                    return func(*args, **kwargs)
                except Exception as e:
                    print(f"ERROR: {message} - Detalles: {e}")
                    raise e
            return wrapper
        return decorator

    @_exceptions("Libreria no encontrada")
    def import_library(self) -> None:
        """
        Importa la librería 'pyserial' para manejar la comunicación serial. Si la librería no está instalada, se activa el modo de simulación de consola.

        Parameters
        ----------
        `None`

        Returns
        -------
        `None`

        Raises
        ------
        `ModuleNotFoundError`
            Si la librería 'pyserial' no está instalada, se captura la excepción y se activa el modo de simulación de consola.

        Example
        -------
        >>> puente_serial = Puente_Serial()
        >>> puente_serial.import_library()
        {None}
        """
        try:
            self.serial_lib:serial = serial
        except ModuleNotFoundError:
            ic(f"Puerto {str(self.port)} Librería 'pyserial' no instalada. INICIANDO MODO SIMULACIÓN de Consola.")
            self.serial_lib = None

    @_exceptions("Error al realizar la conexión física")
    def conexion_fisica(self)-> None:
        """
        Intenta establecer una conexión física con el dispositivo XLR3 a través del puerto serial especificado. Si la conexión falla, se activa el modo de simulación de consola.
        
        Parameters
        ----------
        `None`

        Returns
        -------
        `None`

        Raises
        ------
        `Exception`
            Si el puerto está ocupado o no disponible, se captura la excepción y se activa el modo de simulación de consola.

        Example
        -------
        >>> puente_serial = Puente_Serial()
        >>> puente_serial.import_library()
        {None}
        >>> puente_serial.conexion_fisica()
        {None}
        """
        try:
            self.arduino_serial = self.serial_lib.Serial(self.serial_port, self.baudrate, timeout=self.timeout)
            self.hardware_real = True
            ic(f"Conectado al XLR3 en {self.serial_port}")
        except Exception:
            self.hardware_real = False
            self.arduino_serial = None
            ic(f"Puerto {self.serial_port} no disponible. INICIANDO MODO SIMULACIÓN.")

    @_exceptions("Error al iniciar el puente serial")
    def iniciar_puente_consola(self) -> None:
        """
        Inicia el puente serial entre el rover y la consola. Dependiendo de si se ha establecido una conexión física con el dispositivo XLR3, el puente funcionará en modo real o en modo de simulación de consola.

        Parameters
        ----------
        `None`

        Returns
        -------
        `None`

        Raises
        ------
        `Exception`
            Si ocurre un error al iniciar el puente serial, se captura la excepción y se muestra un mensaje de error.

        Example
        -------
        >>> puente_serial = Puente_Serial()
        >>> puente_serial.import_library()
        {None}
        >>> puente_serial.conexion_fisica()
        {None}
        >>> puente_serial.iniciar_puente_consola()
        {None}
        """
        self.socket:socket.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.bind(("0.0.0.0", self.port))

        ic(f"PUENTE SERIAL XLR3 (HARDWARE REAL: {self.hardware_real})")
        ic("Esperando señal...")

        #Agregar un poco de redundancia en caso de que no se efectue la definicion correcta de variables
        self.direccion_base = None
        self.loggeado = False
        self.prompt = "\ncompetencia-security-login: "

        if self.hardware_real:
            threading.Thread(target=self.leer_cable_serial_fisico, daemon=True).start()

        while True:
            data, addr = self.socket.recvfrom(1024)
            ic(f"RX {addr}: {data.decode(errors='ignore').strip()}")
            self.direccion_base = addr  # Guarda IP para saber a dónde responder
            comando_usuario = data.decode('utf-8')

            #Caso REAL: Reenviar directamente al cable de la competencia
            if self.hardware_real and self.arduino_serial and self.arduino_serial.is_open:
                self.arduino_serial.write(data)
            
            #Caso SIMULACIÓN: Consola de seguridad interactiva para pruebas de software
            else:
                comando_limpio = comando_usuario.strip()
                if not comando_limpio:
                    self.socket.sendto(self.prompt.encode('utf-8'), self.direccion_base)
                    continue

                if not self.loggeado:
                    if comando_limpio == "BOVEDA2026":
                        self.loggeado = True
                        self.prompt = "\nroot@security-console:~# "
                        respuesta = "\n[SUCCESS] Acceso concedido.\nBienvenido a la terminal de seguridad.\n" + self.prompt
                    else:
                        respuesta = "\n[ERROR] Contraseña incorrecta.\n" + self.prompt
                else:
                    if comando_limpio == "ls":
                        respuesta = "\nclearance.txt  vault_logs/  secret_code.txt\n" + self.prompt
                    elif comando_limpio == "cat secret_code.txt":
                        respuesta = "\n CÓDIGO DE LA BÓVEDA: 7412-9630-A\n" + self.prompt
                    elif comando_limpio == "help":
                        respuesta = "\nComandos disponibles: ls, cat secret_code.txt, clear, logout\n" + self.prompt
                    elif comando_limpio == "logout":
                        self.loggeado = False
                        self.prompt = "\ncompetencia-security-login: "
                        respuesta = "\nSesión cerrada.\n" + self.prompt
                    else:
                        respuesta = f"\nbash: {comando_limpio}: comando no encontrado\n" + self.prompt
                ic(f"TX: {respuesta.strip()}")
                self.socket.sendto(respuesta.encode('utf-8'), self.direccion_base)
                    

    @_exceptions("Error en la lectura del cable serial físico")
    def leer_cable_serial_fisico(self) -> None:
        """
        Lee continuamente los datos del cable serial físico conectado al dispositivo XLR3 y los reenvía a la dirección base (rover) a través del socket UDP. Este método se ejecuta en un hilo separado para permitir la comunicación bidireccional entre el rover y la consola.

        Parameters
        ----------
        `None`
        
        Returns
        -------
        `None`

        Raises
        ------
        `Exception`
            Si ocurre un error durante la lectura del cable serial físico, se captura la excepción y se muestra un mensaje de error.

        Example
        -------
        >>> puente_serial = Puente_Serial()
        >>> puente_serial.import_library()
        {None}
        >>> puente_serial.conexion_fisica()
        {None}
        >>> puente_serial.iniciar_puente_consola() 
        {None}
        
        Lee continuamente los datos del cable serial físico y los reenvía al rover a través del socket UDP, a travez del método Puente_Serial().iniciar_puente_consola().
        """
        while self.hardware_real and self.arduino_serial and self.arduino_serial.is_open:
            try:
                # Verificar si hay bytes esperando en el cable físico
                if self.arduino_serial.in_waiting > 0:
                    datos = self.arduino_serial.read(self.arduino_serial.in_waiting)
                    if self.direccion_base:
                        self.socket.sendto(datos, self.direccion_base)
            except Exception as e:
                ic(f"Error en la lectura del cable serial físico: {e}")
                break
            time.sleep(0.01)   




if __name__ == "__main__":
    puente_serial = Puente_Serial()
    puente_serial.import_library()
    puente_serial.conexion_fisica()
    puente_serial.iniciar_puente_consola()

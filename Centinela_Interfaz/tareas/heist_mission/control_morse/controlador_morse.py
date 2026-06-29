# 1. TITULO Y DESCRIPCION DEL ARCHIVO
# Controlador de Actuadores por Senales Morse (Ejecucion en el Rover)
# Proposito: Recibir secuencias Morse por UDP y transmitirlas con precision a un actuador fisico (solenoide o servo) cumpliendo el estandar PARIS a 18 WPM.

# 2. IMPORTS
import socket
import time
from typing import Protocol, Callable, Any, Optional, Literal
from functools import wraps
from icecream import ic

try:
    import RPi.GPIO as GPIO
    SIMULACION: bool = False
except ModuleNotFoundError:
    SIMULACION: bool = True

# 3. PROTOCOL
class ControladorActuadorProtocol(Protocol):
    """
    Contrato minimo para cualquier clase encargada de controlar actuadores mediante transmisiones de senales Morse en el rover.
    """
    def iniciar_servidor(self) -> None:
        """Inicia el socket receptor UDP y procesa los paquetes entrantes."""
        ...
    def procesar_cadena_morse(self, cadena_morse: str) -> None:
        """Decodifica y ejecuta el patron de pulsos en el hardware."""
        ...

# 4. FUNCIONES AUXILIARES / DECORADORES
def manejar_excepciones(
    mensaje: Literal[
        "Error al iniciar el servidor UDP",
        "Error al procesar cadena Morse",
    ]
) -> Callable[..., Any]:
    """
    Crea un decorador que captura excepciones dentro de un metodo e imprime un mensaje de depuracion con detalles del error.
    Parameters
    ----------
    mensaje : Literal[...]
        Texto descriptivo del contexto donde ocurrio el error.

    Returns
    -------
    Callable[..., Any]
        Decorador listo para aplicarse a un metodo.
    """
    def decorador(func: Callable[..., Any]) -> Callable[..., Any]:
        @wraps(func)
        def envoltura(*args: Any, **kwargs: Any) -> Any:
            try:
                return func(*args, **kwargs)
            except Exception as error:
                ic(f"ERROR: {mensaje} | Detalles: {error}")
                raise
        return envoltura
    return decorador

# 5. CLASE PRINCIPAL
class ControladorMorseRover(ControladorActuadorProtocol):
    """
    Administra el socket de red y traduce cadenas de texto Morse en pulsos fisicos de GPIO con temporizaciones del estandar PARIS a 18 WPM.
    Attributes
    ----------
    ip : str
        Direccion IP de escucha del servidor UDP.
    puerto : int
        Puerto UDP asignado para la recepcion de senales.
    pin_actuador : int
        ID del pin GPIO (BCM) conectado al solenoide o actuador.
    """

    # Constantes de clase - tiempos del estandar PARIS a 18 WPM
    DIT: float = 0.0667   # 1 punto = 66.7 ms
    DAH: float = 0.2001   # 1 raya  = 3 puntos = 200.1 ms

    def __init__(
        self,
        ip: Optional[str] = None,
        puerto: int = 5005,
        pin_actuador: int = 18,
    ) -> None:
        self.puerto: int = puerto
        self.pin_actuador: int = pin_actuador

        # Si no se define una IP, se ajusta segun el entorno de ejecucion
        if ip is None:
            self.ip: str = "127.0.0.1" if SIMULACION else "0.0.0.0"
        else:
            self.ip = ip

        self._configurar_hardware()

    # Metodos publicos                                                     

    @manejar_excepciones("Error al iniciar el servidor UDP")
    def iniciar_servidor(self) -> None:
        """
        Inicia el ciclo continuo de escucha de paquetes UDP en el rover.
        Raises
        ------
        socket.error
            Si el puerto ya esta ocupado o la IP no es valida.
        """
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind((self.ip, self.puerto))

        print(f"CONTROLADOR MORSE ACTIVO (MODO SIMULACION: {SIMULACION})")
        print(f"Escuchando en {self.ip}:{self.puerto}...")

        try:
            while True:
                data, addr = sock.recvfrom(1024)
                cadena_morse: str = data.decode('utf-8')

                print(f"\nDatos recibidos desde {addr[0]}: '{cadena_morse}'")
                print("-" * 50)

                self.procesar_cadena_morse(cadena_morse)

                print("-" * 50)
                print("Fin de transmision")

        except KeyboardInterrupt:
            print("\nApagando controlador.")
        finally:
            sock.close()
            if not SIMULACION:
                GPIO.cleanup()

    @manejar_excepciones("Error al procesar cadena Morse")
    def procesar_cadena_morse(self, cadena_morse: str) -> None:
        """
        Modula los tiempos de encendido y apagado del pin GPIO basandose en los caracteres recibidos ('.', '-' y espacios).

        Parameters
        ----------
        cadena_morse : str
            Cadena de texto cruda con la codificacion de pulsos y espacios.
            Ejemplo: '.- -... .--.'.

        Raises
        ------
        ValueError
            Si la cadena recibida esta vacia.
        """
        if not cadena_morse.strip():
            raise ValueError("La cadena Morse recibida esta vacia.")

        # Reemplazo de los triples espacios entre palabras por un token transitorio
        cadena_procesada = cadena_morse.replace('   ', ' W ')
        elementos = cadena_procesada.split(' ')

        for elemento in elementos:
            if not elemento:
                continue

            # Caso A: Espacio entre palabras completo (7 dots en total)
            if elemento == 'W':
                if SIMULACION:
                    print(f"[ESPACIADO ENTRE PALABRAS]---------------[Espera: {self.DIT*7*1000:.1f}ms]")
                time.sleep(self.DIT * 7)

            # Caso B: Procesar letras independientes
            else:
                for i, simbolo in enumerate(elemento):
                    if simbolo == '.':
                        self._ejecutar_pulso(self.DIT)
                    elif simbolo == '-':
                        self._ejecutar_pulso(self.DAH)

                    # Espacio reglamentario entre elementos de la misma letra (1 dot)
                    if i < len(elemento) - 1:
                        time.sleep(self.DIT)

                # Espacio al terminar la letra completa (3 dots)
                if SIMULACION:
                    print(f"[ESPACIADO ENTRE LETRAS]---------------[Espera: {self.DIT*3*1000:.1f}ms]")
                time.sleep(self.DIT * 3)

    # Metodos privados                                                     

    def _configurar_hardware(self) -> None:
        """
        Inicializa los pines GPIO del actuador si no esta en modo simulacion.
        """
        if not SIMULACION:
            GPIO.setmode(GPIO.BCM)
            GPIO.setup(self.pin_actuador, GPIO.OUT)
            GPIO.output(self.pin_actuador, GPIO.LOW)

    def _ejecutar_pulso(self, duracion: float) -> None:
        """
        Conmuta el estado del pin del actuador durante el tiempo indicado.

        Parameters
        ----------
        duracion : float
            Tiempo en segundos que el actuador permanece activo (DIT o DAH).
        """
        if SIMULACION:
            print(f"[GPIO {self.pin_actuador}] -> HIGH ({duracion*1000:.1f}ms)")
        else:
            GPIO.output(self.pin_actuador, GPIO.HIGH)

        time.sleep(duracion)

        if SIMULACION:
            print(f"[GPIO {self.pin_actuador}] -> LOW")
        else:
            GPIO.output(self.pin_actuador, GPIO.LOW)

# 6. EJEMPLO DE USO
if __name__ == "__main__":
    controlador = ControladorMorseRover(puerto=5005, pin_actuador=18)
    controlador.iniciar_servidor()
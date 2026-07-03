# Calculadora de Cables para Heist Mission
# Basado en las reglas del documento "Camera Access Panel"
from typing import Protocol, List, Callable, Any, Optional, Literal
from icecream import ic


class Analizar_Cables(Protocol):
    def __init__(self, reglas: Optional[Any] = None) -> None:
        ...

    def _exceptions(message: Literal["Error en el análisis del panel", "Error al procesar la entrada"]) -> Callable[..., Any]:
        ...

    def analizar_panel(self, cables: List[str]) -> int | str:
        ...

    def procesar_entrada(self, entrada) -> List[str]:
        ...


class CalculadoraCables(Analizar_Cables):
    """
    Atributes
    ---------
    `reglas`: Optional[Any]
        Esto es para que se puedan pasar reglas personalizadas si se desea, aunque por ahora no es necesario.
    `COLORES_VALIDOS`: List[str]
        Lista de colores válidos según el reglamento, para validar la entrada del usuario.
    `cable_a_cortar`: dict[int, str]
        Almacena el resultado del análisis, es decir, el número del cable que se debe cortar.
    `cantidad_cables`: int
        Almacena la cantidad de cables ingresada por el usuario, para poder usarla en otros métodos si se desea.
    """

    def __init__(self, reglas: Optional[Any] = None) -> None:
        self.reglas: Optional[Any] = reglas  # Esto en caso de ser necesario
        self.COLORES_VALIDOS: list = [
            "rojo", "blanco", "azul", "amarillo", "negro"]
        # Para almacenar el resultado del análisis y poder usarlo en otros métodos si se desea
        self.cable_a_cortar: dict[int, str] = {}
        # Para almacenar la cantidad de cables y poder usarla en otros métodos si se desea
        self.cantidad_cables: int = None

    # Definir un decorador con icecream para depurar las salidas#

    def _exceptions(message: Literal["Error en el análisis del panel", "Error al procesar la entrada"]) -> Callable[..., Any]:
        def decorator(func: Callable[..., Any]) -> Callable[..., Any]:
            def wr(*args, **kwargs) -> Any:
                try:
                    return func(*args, **kwargs)
                except Exception as e:
                    ic(f"ERROR: {message} - Detalles: {e}")
                    raise e
            return wr
        return decorator

    @_exceptions("Error en el análisis del panel")
    def analizar_panel(self, cables: List[str]) -> int | str:
        """
        Analiza el panel de cables y determina cuál debe ser cortado.

        Parameters
        ----------
        `cables`: List[str]
            Lista de colores de los cables, en orden de izquierda a derecha, ya procesados (sin espacios, en minúsculas, etc.)

        Returns
        -------
        int|str
            El número del cable que se debe cortar, o un mensaje de error si no se pudo determinar.

        Raises
        ------
        ValueError
            Si la lista de cables no cumple con las reglas del reglamento (por ejemplo, si hay un número incorrecto de cables o si los colores no son válidos).


        Example
        -------
        >>> calculadora = CalculadoraCables()'
        >>> cables = ["azul", "rojo", "amarillo"]
        >>> calculadora.analizar_panel(cables)
        {2, "rojo"}
        """
        # Aqui se tiene que agregar el analisis que se tiene de los cables
        self.cantidad_cables: int = len(cables)

        if self.cantidad_cables < 3 or self.cantidad_cables > 6:
            raise ValueError(
                f"Cantidad de cables inválida: {self.cantidad_cables}. El reglamento especifica que solo hay entre 3 y 6 cables.")

        # Revisar si no existe algún color que no sea válido
        if any(c not in self.COLORES_VALIDOS for c in cables):
            raise ValueError(
                f"Colores no válidos en la lista: {cables}. El reglamento solo permite: {', '.join(self.COLORES_VALIDOS)}")

        if self.cantidad_cables == 3:
            if "rojo" not in cables:
                self.cable_a_cortar = {2: cables[1]}
            elif cables[-1] == "blanco":
                self.cable_a_cortar = {3: cables[2]}
            elif cables.count("azul") > 1:
                self.cable_a_cortar = {
                    len(cables) - cables[::-1].index("azul"): "azul"}
            else:
                self.cable_a_cortar = {3: cables[2]}

        elif self.cantidad_cables == 4:
            if cables.count("rojo") > 1:
                self.cable_a_cortar = {
                    len(cables) - cables[::-1].index("rojo"): "rojo"}
            elif cables[-1] == "amarillo" and "rojo" not in cables:
                self.cable_a_cortar = {1: cables[0]}
            elif cables.count("azul") == 1:
                self.cable_a_cortar = {1: cables[0]}
            elif cables.count("amarillo") > 1:
                self.cable_a_cortar = {4: cables[3]}
            else:
                self.cable_a_cortar = {2: cables[1]}

        elif self.cantidad_cables == 5:
            if cables[-1] == "negro":
                self.cable_a_cortar = {4: cables[3]}
            elif cables.count("rojo") == 1 and cables.count("amarillo") > 1:
                self.cable_a_cortar = {1: cables[0]}
            elif "negro" not in cables:
                self.cable_a_cortar = {2: cables[1]}
            else:
                self.cable_a_cortar = {5: cables[4]}

        elif self.cantidad_cables == 6:
            if "amarillo" not in cables:
                self.cable_a_cortar = {3: cables[2]}
            elif cables.count("amarillo") == 1 and cables.count("blanco") > 1:
                self.cable_a_cortar = {4: cables[3]}
            elif "rojo" not in cables:
                self.cable_a_cortar = {6: cables[5]}
            else:
                self.cable_a_cortar = {2: cables[1]}
            ...

    @_exceptions("Error al procesar la entrada")
    def procesar_entrada(self, entrada: str) -> List[str]:
        """
        Este método se encarga de procesar la entrada cruda del usuario, validarla y convertirla en una lista de colores limpia y lista para ser analizada por el método `analizar_panel`.

        Parameters
        ----------
        `entrada`: str
            La entrada cruda del usuario, que se espera sea una cadena de texto con los colores separados por comas (Ej: "azul, rojo, amarillo").

        Returns
        -------
        List[str]
            Una lista de colores procesada, sin espacios, en minúsculas, y validada contra los colores permitidos.

        Raises
        ------
        ValueError
            Si la entrada no es válida (por ejemplo, si contiene colores no permitidos o si el formato es incorrecto).

        Example
        -------
        >>> calculadora = CalculadoraCables()
        >>> entrada = "azul, rojo, amarillo"
        >>> colores = calculadora.procesar_entrada(entrada)
        >>> print(colores)
        ["azul", "rojo", "amarillo"]
        """

        if entrada:
            # Procesar la entrada: separar por comas, quitar espacios y a minúsculas
            cables = [color.strip().lower() for color in entrada.split(",")]
            # Validar que SOLO se usen los colores del reglamento
            colores_incorrectos = [
                c for c in cables if c not in self.COLORES_VALIDOS]

            if colores_incorrectos:
                raise ValueError(
                    f"Colores no válidos: {colores_incorrectos}. El reglamento solo permite: {', '.join(self.COLORES_VALIDOS)}")
            return cables

        else:
            ic("La entrada está vacía. Por favor, ingrese una lista de colores separados por comas.")
            return []


if __name__ == "__main__":
    calculadora: object = CalculadoraCables()
    values: list[Any] = calculadora.procesar_entrada("azul, rojo, amarillo")
    calculadora.analizar_panel(values)
    print(calculadora.cable_a_cortar)

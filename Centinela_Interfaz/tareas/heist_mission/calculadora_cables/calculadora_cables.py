# Calculadora de Cables para Heist Mission
# Basado en las reglas del documento "Camera Access Panel"

def analizar_panel():
    # Colores que hay en el reglamento
    COLORES_VALIDOS = ["rojo", "blanco", "azul", "amarillo", "negro"]
    
    while True:
        try:
            # 1. Solicitar el número de cables
            entrada = input("1. Introduzca el número de cables en el panel o escriba exit para salir:").strip().lower()
            
            if entrada == "exit":
                print("Saliendo de la calculadora")
                break 
                
            num_cables = int(entrada)
            
            if num_cables < 3 or num_cables > 6:
                print("ERROR: El reglamento especifica que solo hay entre 3 y 6 cables.")
                continue 

            # 2. Solicitar la lista de colores
            print("\n2. Introduzca el nombre del color de los cables de izquierda a derecha,")
            colores_raw = input("separados por coma y presione enter (Ej: azul,rojo,amarillo,negro):\n")
            
            if colores_raw.strip().lower() == "exit":
                print("Saliendo de la calculadora")
                break
            
            # Procesar la entrada: separar por comas, quitar espacios y a minúsculas
            cables = [color.strip().lower() for color in colores_raw.split(",")]
            
            # Validar que la cantidad de colores coincida con el número ingresado
            if len(cables) != num_cables:
                print(f"\nERROR: Se ingresaron {len(cables)} colores, pero dijo que había {num_cables} cables.")
                continue 

            # Validar que SOLO se usen los colores del reglamento
            colores_incorrectos = [c for c in cables if c not in COLORES_VALIDOS]
            if colores_incorrectos:
                print(f"\nERROR: Colores no válidos: {colores_incorrectos}")
                print(f"El reglamento solo permite: {', '.join(COLORES_VALIDOS)}")
                continue 

            print(f"\nColores introducidos: {cables}")
            cable_a_cortar = None

            # REGLAS PARA 3 CABLES
            if num_cables == 3:
                if "rojo" not in cables:
                    cable_a_cortar = 2
                elif cables[-1] == "blanco":
                    cable_a_cortar = 3
                    cable_a_cortar = 3
                elif cables.count("azul") > 1:
                    cable_a_cortar = len(cables) - cables[::-1].index("azul")
                else:
                    cable_a_cortar = 3

            # REGLAS PARA 4 CABLES
            elif num_cables == 4:
                if cables.count("rojo") > 1:
                    cable_a_cortar = len(cables) - cables[::-1].index("rojo")
                elif cables[-1] == "amarillo" and "rojo" not in cables:
                    cable_a_cortar = 1
                elif cables.count("azul") == 1:
                    cable_a_cortar = 1
                elif cables.count("amarillo") > 1:
                    cable_a_cortar = 4
                else:
                    cable_a_cortar = 2

            # REGLAS PARA 5 CABLES
            elif num_cables == 5:
                if cables[-1] == "negro":
                    cable_a_cortar = 4
                elif cables.count("rojo") == 1 and cables.count("amarillo") > 1:
                    cable_a_cortar = 1
                elif "negro" not in cables:
                    cable_a_cortar = 2
                else:
                    cable_a_cortar = 5

            # REGLAS PARA 6 CABLES
            elif num_cables == 6:
                if "amarillo" not in cables:
                    cable_a_cortar = 3
                elif cables.count("amarillo") == 1 and cables.count("blanco") > 1:
                    cable_a_cortar = 4
                elif "rojo" not in cables:
                    cable_a_cortar = 6
                else:
                    cable_a_cortar = 2

            # IMPRESIÓN DEL RESULTADO
            print(f"\nRESPUESTA:CORTAR EL CABLE NÚMERO {cable_a_cortar} ({cables[cable_a_cortar-1 if 'cable_a_cortar' in locals() else cable_a_cortar-1].upper()})")
            print("-" * 50)

        except ValueError:
            print("ERROR: Introduzca un número valido para la cantidad de cables o escriba exit para salir.")

if __name__ == "__main__":
    analizar_panel()
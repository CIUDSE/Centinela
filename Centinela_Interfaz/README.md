# Centinela — Interfaz de Control

Interfaz de la estación terrena (ground station) para el rover Centinela.

## Requisitos

- Python **3.12.6**
- Node.js (versión 16 o superior recomendada)
- Repositorio clonado.

## Cómo ejecutar el programa

El sistema requiere **2 terminales abiertas** al mismo tiempo dentro de `~/Centinela/Centinela_Interfaz`.

### Modo simulación

Usa este modo cuando no tienes la Raspberry Pi conectada y quieres probar la lógica localmente.

**Terminal 1:**

```bash
cd ~/Centinela/Centinela_Interfaz
python3 tareas/heist_mission/apagador_de_luces/apagador_luces.py
```

**Terminal 2:**

```bash
cd ~/Centinela/Centinela_Interfaz
python3 app.py 127.0.0.1
```

### Modo real (con Raspberry Pi)

Usa este modo cuando la Raspberry Pi está conectada y accesible en la red.

**Terminal 1:**

```bash
cd ~/Centinela/Centinela_Interfaz
python3 tareas/heist_mission/apagador_de_luces/apagador_luces.py
```

**Terminal 2:**

```bash
cd ~/Centinela/Centinela_Interfaz
python3 app.py [ip_de_raspberry]
```

Reemplaza `[ip_de_raspberry]` por la IP real de la Raspberry Pi en la red (ej. `192.168.1.50`).

> **Nota:** el orden importa — primero se levanta `apagador_luces.py` en una terminal, y después `app.py` en la otra.

## Configuración inicial en una laptop nueva

Comandos para dejar el entorno listo desde cero:

```bash
# 1. Verificar/instalar Python 3.12.6
python3 --version

# 2. Clonar el repositorio (si aún no está clonado)
git clone https://github.com/CIUDSE/Centinela.git

# 3. Entrar al proyecto
cd ~/Centinela/Centinela_Interfaz

# 4. (Opcional pero recomendado) crear entorno virtual
python3 -m venv venv
source venv/bin/activate

# 5. Instalar dependencias de Python
pip install -r requirements.txt

# 6. Instalar dependencia Wetty para SSH
npm install -g wetty

# 6. Verificar que ambos scripts corren sin errores (modo simulación)
python3 tareas/heist_mission/apagador_de_luces/apagador_luces.py
# en otra terminal:
python3 app.py 127.0.0.1
```

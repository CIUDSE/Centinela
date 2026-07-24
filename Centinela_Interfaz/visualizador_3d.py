import pyqtgraph.opengl as gl
import numpy as np


# STREAMING_CHUNK: Creando clase de visualización 3D...
class IMU3DVisualizer(gl.GLViewWidget):
   """
   Subclase de visualización 3D que renderiza una representación espacial
   del satélite Centinela usando OpenGL.
   """
   def __init__(self, parent=None):
       super().__init__(parent)
       self.setCameraPosition(distance=10, elevation=25, azimuth=45)
      
       # Grid de referencia
       self.grid = gl.GLGridItem()
       self.grid.setSize(12, 12, 1)
       self.grid.setSpacing(1, 1, 1)
       self.addItem(self.grid)
      
       # Ejes globales
       self.ejes = gl.GLAxisItem()
       self.ejes.setSize(4, 4, 4)
       self.addItem(self.ejes)


       # Caja que representa el cuerpo del satélite/sensor
       self.sensor_pcb = gl.GLBoxItem(color=(59, 130, 246, 200)) # Azul translúcido
       self.sensor_pcb.setSize(x=3.0, y=1.5, z=0.4)
       self.sensor_pcb.translate(-1.5, -0.75, -0.2) # Centrar el pivote
       self.addItem(self.sensor_pcb)


       # Puntero rojo (dirección de avance / eje X)
       self.direccion_puntero = gl.GLBoxItem(color=(239, 68, 68, 255))
       self.direccion_puntero.setSize(x=1.0, y=0.4, z=0.4)
       self.direccion_puntero.translate(1.5, -0.2, -0.2)
       self.addItem(self.direccion_puntero)


   # STREAMING_CHUNK: Definiendo rotación espacial...
   def update_attitude(self, pitch, roll, yaw=0.0):
       """
       Recibe los ángulos del filtro del sensor y rota el objeto 3D en la GPU.
       """
       # Resetear transformaciones anteriores
       self.sensor_pcb.resetTransform()
       self.sensor_pcb.translate(-1.5, -0.75, -0.2)
      
       self.direccion_puntero.resetTransform()
       self.direccion_puntero.translate(1.5, -0.2, -0.2)
      
       # Aplicar rotación secuencial (Yaw -> Pitch -> Roll)
       for obj in [self.sensor_pcb, self.direccion_puntero]:
           obj.rotate(yaw, 0, 0, 1)    # Guiñada (Z)
           obj.rotate(pitch, 0, 1, 0)  # Cabeceo (Y)
           obj.rotate(roll, 1, 0, 0)   # Alabeo (X)

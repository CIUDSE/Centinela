Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela
Código desarrollado por Electrónica Rovers
Código TRANSMISOR/EMISOR para telemetría general del rover Centinela implementando una LilyGo TTGO T-Beam V1.2, Neo6m integrado, MPU 6050, DS18B20.

Para la prueba del envio de datos se utilizo este código en la placa lilygo ttgo t-beam como transmisor, mientras que se uso la lilygo con pantalla
como receptor utilizando el codigo "ReceptorStruct" en la carpeta de pruebas.

Librerias necesarias:
LoRa by Sandeep Mistry
OneWire by Jim Studt...(Y otros)
DallasTemperature
TinyGPS+

Estructura de envio de datos:
telemetryData -> {accelX, accelY, accelZ, rotX, rotY, rotZ, lat1, lon1, lat2, lon2, temp[10]}
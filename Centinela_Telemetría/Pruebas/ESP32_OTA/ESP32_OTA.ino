// Creado ChepeCarlos de ALSW - Tutorial Completo en https://nocheprogramacion.com

/*
Archivo editable con descripción de cada función escencial para su uso en cualquier proyecto
*/

#if defined(ESP32)        // Librerias para usar OTA en ESP32

#include <WiFi.h>
#include <ESPmDNS.h>

#elif defined(ESP8266)    // Librerias para usar OTA en ESP8266

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#endif

#include <WiFiUdp.h>
#include <ArduinoOTA.h>   // Libreria de Arduino OTA

// Credenciales para conectarse al internet del Esp32
const char* ssid      = "Nombre_de_la_red"      ; // Nombre de la red a conectar
const char* password  = "Contraseña_de_la_red"  ; // Contraseña de la red a conectar

// Aquí se pueden declarar las variables a usar

void setup() 
{
  Serial.begin(115200);
  Serial.println("Cargando sistema...");
  
  WiFi.mode(WIFI_STA);
 
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) 
  {
    Serial.println("Error con Wifi, reiniciando...");
    delay(5000);
    ESP.restart();
  }

  /* Credenciales para dar acceso a programar por Wifi - Quitar diagonales y flecha de comentario para usar */
  // Puerto de programacion, defecto 3232
  // ArduinoOTA.setPort(3232);

  // Nombre en Red, defecto esp3232-[MAC]
  ArduinoOTA.setHostname("Nombre_del_esp32"); //Nombre personalizado para el esp32

  // Controaseña, defecto no contraseña
  // ArduinoOTA.setPassword("1234");
  // --> ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

/* Mensaje de Inicio */
  ArduinoOTA.onStart([]() 
  {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) 
    {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }

    Serial.println("Iniciando Programación " + type);
  } );

/* Análisis de Finalizado al subir el código */
  ArduinoOTA.onEnd([]() 
  { Serial.println("\nTerminando");  }  );

/* Análisis de Progreso al subir el código */
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) 
  { Serial.printf("Progreso: %u%%\r", (progress / (total / 100)));  }   );
 
/* Análisis de Error al subir código */
  ArduinoOTA.onError([](ota_error_t error) 
  {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) 
    {   Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) 
    {   Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) 
    {   Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) 
    {   Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) 
    {   Serial.println("End Failed");   }
  } );

  ArduinoOTA.begin(); // Se puede programar un botón físico para que al presionarlo se actualice

  Serial.println("Listo");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  /* Parte de setup del codigo*/
}

void loop() 
{
  ArduinoOTA.handle();

  /* Aquí poner el código que se desea subir al ESP */
}
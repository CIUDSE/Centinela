/*****************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela

Código desarrollado por Software Rovers

Archivo JavaScript que implementa peticiones HTTP para comunicarse con los endpoints de Flask y realizar tareas del rover.
*****************************************************************************************************************************/

// Envia un comando a la API de Flask para encender o apagar las luces del rover
async function enviarComandoLuces(accion) {
  try {
    const response = await fetch("/api/luces", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ comando: accion }),
    });

    const resultado = await response.json();
    if (!response.ok) {
      alert(resultado.mensaje); // Te avisará en pantalla si está bloqueado
    } else {
      console.log(resultado.mensaje);
    }
  } catch (error) {
    console.error("Error de red:", error);
  }
}

// Recoge el input del usuario, envia la peticion a Flask
// para que corra la logica de analisis de cables
// y muestra el resultado en la interfaz
async function analizarCables() {
  const entrada = document.getElementById("cables-input").value.trim();
  const divResultado = document.getElementById("cables-resultado");
  const divError = document.getElementById("cables-error");

  divResultado.classList.add("oculto");
  divError.classList.add("oculto");

  if (!entrada) {
    divError.textContent = "⚠️ Ingresa los colores antes de analizar.";
    divError.classList.remove("oculto");
    return;
  }

  try {
    const response = await fetch("/api/cables", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ cables: entrada }),
    });

    const data = await response.json();

    if (response.ok) {
      document.getElementById("cables-numero").textContent =
        `#${data.cable_numero}`;
      const badge = document.getElementById("cables-color");
      badge.textContent = data.cable_color;
      divResultado.classList.remove("oculto");
    } else {
      divError.textContent = `❌ ${data.mensaje}`;
      divError.classList.remove("oculto");
    }
  } catch (err) {
    divError.textContent = `❌ Error de red: ${err.message}`;
    divError.classList.remove("oculto");
  }
}

// Actualiza el indicador visual de estado en la topbar
function actualizarEstadoSistema(estado) {
  const dot = document.getElementById("status-dot");
  const texto = document.getElementById("status-texto");

  if (estado === "emergency_active") {
    dot.classList.remove("dot--green");
    dot.classList.add("dot--red");
    texto.textContent = "Sistema desactivado";
  } else {
    dot.classList.remove("dot--red");
    dot.classList.add("dot--green");
    texto.textContent = "Sistema activo";
  }
}

// Envia peticion para activar modo de emergencia a la API de Flask
async function enviarEmergencia(estado) {
  try {
    const response = await fetch("/api/emergencia", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ accion: estado }),
    });

    const resultado = await response.json();
    console.log("Emergencia API:", resultado);
    actualizarEstadoSistema(resultado.status);
  } catch (error) {
    console.error("Error de red en emergencia:", error);
  }
};
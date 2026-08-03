/**
 * Módulo de Telemetría vía Web Serial API
 * Lee la trama CSV enviada por el receptor LoRa (TTGO LoRa32) y la convierte
 * en un objeto JS que refleja la estructura telemetryData_t del firmware.
 *
 * Formato de trama recibido por Serial (ver receptor LoRa con OLED):
 * $id,tiempoRecibido,numPaquete,rssi,velAngX,velAngY,velAngZ,accelX,accelY,accelZ,
 *   lat1,lon1,lat2,lon2,temp0,temp1,temp2,temp3,temp4,temp5,temp6,temp7,temp8,temp9\n
 *
 * Total: 24 campos separados por comas, precedidos por '$' y terminados en '\n'.
 */

// --------------------------- Estado del módulo ---------------------------
window.simularActivo = true;
let puertoSerial = null; // Objeto SerialPort
let lectorSerial = null; // Reader del stream de texto
let lectorWritable = null; // Writable del TextDecoderStream (para poder abortarlo)
let ejecutandoLectura = false;
let tiempoSimulado = 0;

// Aquí se guarda siempre la ÚLTIMA trama válida recibida (el "objeto" que pediste)
window.telemetriaActual = {
  id: null,
  tiempoRecibido: 0,
  numPaquete: 0,
  rssi: 0,
  velAngular: { x: 0, y: 0, z: 0 },
  accel: { x: 0, y: 0, z: 0 },
  gps1: { lat: 0, lon: 0 }, // GPS integrado del T-Beam
  gps2: { lat: 0, lon: 0 }, // GPS externo Neo6m
  temperaturas: new Array(10).fill(0),
  timestampLocal: null,
};

// Variables de Chart.js
let chartAccel = null;
let chartGyro = null;
const MAX_PUNTOS_GRAFICA = 30;

// ===========================================================================
// 1. PARSER: de línea de texto -> objeto telemetría
// ===========================================================================
function parsearTramaLoRa(linea) {
  // La trama siempre inicia con '$' (definido así en el firmware del receptor)
  if (!linea.startsWith("$")) {
    return null;
  }

  // Quitamos el '$' y separamos por comas
  const campos = linea.slice(1).split(",");

  // La estructura telemetryData_t serializa 24 valores:
  // 4 generales + 3 vel. angular + 3 accel + 4 GPS (lat1,lon1,lat2,lon2) + 10 temperaturas
  if (campos.length !== 24) {
    console.warn(`Trama con ${campos.length} campos (se esperaban 24):`, linea);
    return null;
  }

  const temperaturas = campos.slice(14, 24).map(Number);

  const telemetria = {
    id: campos[0],
    tiempoRecibido: Number(campos[1]),
    numPaquete: Number(campos[2]),
    rssi: Number(campos[3]),
    velAngular: {
      x: Number(campos[4]),
      y: Number(campos[5]),
      z: Number(campos[6]),
    },
    accel: {
      x: Number(campos[7]),
      y: Number(campos[8]),
      z: Number(campos[9]),
    },
    gps1: { lat: Number(campos[10]), lon: Number(campos[11]) },
    gps2: { lat: Number(campos[12]), lon: Number(campos[13]) },
    temperaturas,
    timestampLocal: Date.now(),
  };

  // Validación: si algún campo no se pudo convertir a número, descartamos la trama
  const valoresNumericos = [
    telemetria.tiempoRecibido,
    telemetria.numPaquete,
    telemetria.rssi,
    telemetria.velAngular.x,
    telemetria.velAngular.y,
    telemetria.velAngular.z,
    telemetria.accel.x,
    telemetria.accel.y,
    telemetria.accel.z,
    telemetria.gps1.lat,
    telemetria.gps1.lon,
    telemetria.gps2.lat,
    telemetria.gps2.lon,
    ...temperaturas,
  ];
  if (valoresNumericos.some((v) => Number.isNaN(v))) {
    console.warn("Trama descartada por valores no numéricos:", linea);
    return null;
  }

  return telemetria;
}

// ===========================================================================
// 2. Guardar el objeto y refrescar la interfaz cada vez que llega una trama
// ===========================================================================
let acumuladorYaw = 0; // Variable acumuladora par integrar en el giroscopio

function actualizarTelemetria(telemetria) {
  // Guardamos la trama más reciente en el objeto global
  window.telemetriaActual = telemetria;

  console.log(telemetria);

  // Avisamos al resto de la app (mapa, tabla, etc.) por si quieren escuchar
  document.dispatchEvent(
    new CustomEvent("telemetria-actualizada", { detail: telemetria }),
  );

  // -------------------------------------------------------------------------
  // 🟢 CÁLCULO DE ÁNGULOS (PITCH, ROLL Y YAW)
  // -------------------------------------------------------------------------
  const ax = telemetria.accel.x;
  const ay = telemetria.accel.y;
  const az = telemetria.accel.z;
  const gz = telemetria.velAngular.z;

  // Inclinaciones trigonométricas en radianes por acelerómetro
  const pitchRad = Math.atan2(-ax, Math.sqrt(ay * ay + az * az));
  const rollRad  = Math.atan2(ay, az);

  // Integración de velocidad angular a 50Hz (dt = 0.02s) para el Yaw
  acumuladorYaw += (gz * 0.02) * (Math.PI / 180);
  const yawRad = acumuladorYaw;

  // Conversión a Grados sexagesimales para la interfaz
  const RAD2DEG = 180 / Math.PI;
  const pitchDeg = (pitchRad * RAD2DEG).toFixed(2);
  const rollDeg  = (rollRad * RAD2DEG).toFixed(2);
  const yawDeg   = ((yawRad * RAD2DEG) % 360).toFixed(2);

  // -------------------------------------------------------------------------
  // 🟢 ACTUALIZAR TARJETAS NUMÉRICAS EN PANTALLA (HTML)
  // -------------------------------------------------------------------------
  const txtPitch = document.getElementById("txt-imu-pitch");
  const txtRoll  = document.getElementById("txt-imu-roll");
  const txtYaw   = document.getElementById("txt-imu-yaw");

  if (txtPitch) txtPitch.textContent = `${pitchDeg}°`;
  if (txtRoll)  txtRoll.textContent  = `${rollDeg}°`;
  if (txtYaw)   txtYaw.textContent   = `${yawDeg}°`;

  // -------------------------------------------------------------------------
  // 🎯 ROTAR MODELO 3D EN THREE.JS
  // -------------------------------------------------------------------------
  if (window.roverMesh) {
    window.roverMesh.rotation.x = pitchRad;
    window.roverMesh.rotation.z = rollRad;
    window.roverMesh.rotation.y = yawRad;
  }

  // Actualizamos gráficas de IMU (acelerómetro / giroscopio)
  actualizarGraficasIMU(
    telemetria.accel.x,
    telemetria.accel.y,
    telemetria.accel.z,
    telemetria.velAngular.x,
    telemetria.velAngular.y,
    telemetria.velAngular.z,
  );

  // Si hay coordenadas válidas (distintas de 0) en el GPS principal, avisamos al mapa
  if (telemetria.gps1.lat !== 0 && telemetria.gps1.lon !== 0) {
    document.dispatchEvent(
      new CustomEvent("actualizarGPS", {
        detail: { lat: telemetria.gps1.lat, lng: telemetria.gps1.lon },
      }),
    );
  }

  // Texto de estado en pantalla (RSSI, número de paquete), si existen esos elementos
  const elRssi = document.getElementById("txt-rssi");
  const elPaquete = document.getElementById("txt-num-paquete");
  if (elRssi) elRssi.textContent = `${telemetria.rssi} dBm`;
  if (elPaquete) elPaquete.textContent = telemetria.numPaquete;
}

// ===========================================================================
// 3. Conexión y lectura por Web Serial API
// ===========================================================================
async function conectarTelemetriaUSB() {
  const btnConectar =
    document.getElementById("btn-conectar-telemetria") ||
    document.getElementById("btn-conectar-imu");
  const txtEstado = document.getElementById("txt-estado-conexion");

  if (!("serial" in navigator)) {
    alert(
      "Tu navegador no soporta Web Serial API. Usa Google Chrome o Microsoft Edge.",
    );
    return;
  }

  // -------------------------------------------------------------------
  // Si ya estamos leyendo -> este clic significa "Desconectar"
  // -------------------------------------------------------------------
  if (ejecutandoLectura) {
    ejecutandoLectura = false; // Detiene el bucle while de lectura

    if (btnConectar) {
      btnConectar.textContent = "🔌 Conectar USB";
      btnConectar.className = "btn btn--on";
    }
    if (txtEstado) {
      txtEstado.textContent = "(Desconectado)";
      txtEstado.style.color = "#888";
    }

    try {
      if (lectorSerial) {
        await lectorSerial.cancel();
        lectorSerial.releaseLock();
        lectorSerial = null;
      }
      if (lectorWritable) {
        await lectorWritable.abort();
        lectorWritable = null;
      }
      if (puertoSerial) {
        await puertoSerial.close();
        puertoSerial = null;
      }
    } catch (err) {
      console.error("Error al liberar el puerto USB:", err);
    }
    return;
  }

  // -------------------------------------------------------------------
  // Conexión nueva
  // -------------------------------------------------------------------
  try {
    // Asegurar que no queden vinculaciones 'Paired' fantasma antes de abrir la ventana
    if ("getPorts" in navigator.serial) {
      const puertosPrevios = await navigator.serial.getPorts();
      for (const puerto of puertosPrevios) {
        if ("forget" in puerto) {
          await puerto.forget();
        }
      }
    }

    // Abre el selector nativo del navegador para elegir el COM (ej. COM4)
    puertoSerial = await navigator.serial.requestPort();

    // Debe coincidir con Serial.begin(115200) del firmware del receptor
    await puertoSerial.open({ baudRate: 115200 });

    ejecutandoLectura = true;

    if (btnConectar) {
      btnConectar.textContent = "⏹ Desconectar USB";
      btnConectar.className = "btn btn--off";
    }
    if (txtEstado) {
      txtEstado.textContent = "🟢 Conectado";
      txtEstado.style.color = "#238636";
    }

    // Apagamos la simulación al conectar hardware real
    const chkSimular = document.getElementById("chk-simular-imu");
    if (chkSimular) chkSimular.checked = false;
    window.simularActivo = false;

    // Decodificamos los bytes entrantes como texto UTF-8
    const textDecoder = new TextDecoderStream();
    lectorWritable = textDecoder.writable;
    puertoSerial.readable.pipeTo(lectorWritable).catch(() => {});

    lectorSerial = textDecoder.readable.getReader();

    let bufferTrama = ""; // Acumula texto hasta tener una línea completa

    while (ejecutandoLectura) {
      const { value, done } = await lectorSerial.read();
      if (done) break;
      if (!value) continue;

      bufferTrama += value;

      // El Serial puede entregar trozos parciales; solo procesamos
      // líneas completas (terminadas en '\n') y guardamos el resto
      const lineas = bufferTrama.split("\n");
      bufferTrama = lineas.pop(); // el último trozo puede estar incompleto

      for (const lineaCruda of lineas) {
        const linea = lineaCruda.trim();
        if (linea.length === 0) continue;

        const telemetria = parsearTramaLoRa(linea);
        if (telemetria) {
          actualizarTelemetria(telemetria);
        }
        // Si parsearTramaLoRa devuelve null, la trama se descarta y se
        // registra el motivo en consola (línea corrupta, incompleta, etc.)
      }
    }
  } catch (err) {
    console.error("Error en conexión USB:", err);
    ejecutandoLectura = false;
    if (lectorSerial) {
      try {
        lectorSerial.releaseLock();
      } catch (e) {}
      lectorSerial = null;
    }
    puertoSerial = null;
    if (btnConectar) {
      btnConectar.textContent = "🔌 Conectar USB";
      btnConectar.className = "btn btn--on";
    }
    if (txtEstado) {
      txtEstado.textContent = "(Error o Cancelado)";
      txtEstado.style.color = "#da3633";
    }
  }
}

// ===========================================================================
// 4. Gráficas Chart.js (accel / giroscopio) — igual que antes
// ===========================================================================
function inicializarGraficasIMU() {
  const ctxAccel = document
    .getElementById("chart-acelerometro")
    ?.getContext("2d");
  const ctxGyro = document.getElementById("chart-giroscopio")?.getContext("2d");
  if (!ctxAccel || !ctxGyro) return;

  const configBase = {
    type: "line",
    options: {
      responsive: true,
      animation: false,
      scales: {
        x: { display: false },
        y: {
          grid: { color: "rgba(255, 255, 255, 0.05)" },
          ticks: { color: "#777", font: { size: 9 } },
        },
      },
      plugins: {
        legend: { labels: { color: "#ccc", font: { size: 10 }, boxWidth: 10 } },
      },
    },
  };

  chartAccel = new Chart(ctxAccel, {
    ...configBase,
    data: {
      labels: Array(MAX_PUNTOS_GRAFICA).fill(""),
      datasets: [
        {
          label: "Acc X",
          borderColor: "#ff4d4d",
          data: [],
          borderWidth: 1.5,
          pointRadius: 0,
        },
        {
          label: "Acc Y",
          borderColor: "#2ecc71",
          data: [],
          borderWidth: 1.5,
          pointRadius: 0,
        },
        {
          label: "Acc Z",
          borderColor: "#3498db",
          data: [],
          borderWidth: 1.5,
          pointRadius: 0,
        },
      ],
    },
  });

  chartGyro = new Chart(ctxGyro, {
    ...configBase,
    data: {
      labels: Array(MAX_PUNTOS_GRAFICA).fill(""),
      datasets: [
        {
          label: "Gyr X",
          borderColor: "#f1c40f",
          data: [],
          borderWidth: 1.5,
          pointRadius: 0,
        },
        {
          label: "Gyr Y",
          borderColor: "#9b59b6",
          data: [],
          borderWidth: 1.5,
          pointRadius: 0,
        },
        {
          label: "Gyr Z",
          borderColor: "#e67e22",
          data: [],
          borderWidth: 1.5,
          pointRadius: 0,
        },
      ],
    },
  });
}

function actualizarGraficasIMU(ax, ay, az, gx, gy, gz) {
  if (!chartAccel || !chartGyro) return;

  const agregarDato = (chart, datos) => {
    chart.data.datasets.forEach((dataset, index) => {
      if (dataset.data.length >= MAX_PUNTOS_GRAFICA) dataset.data.shift();
      dataset.data.push(datos[index]);
    });
    chart.update("none");
  };

  agregarDato(chartAccel, [ax, ay, az]);
  agregarDato(chartGyro, [gx, gy, gz]);
}

// ===========================================================================
// 5. Simulación de prueba (sin hardware conectado)
// ===========================================================================
let timerSimulacion = null;

function loopSimulacion() {
  if (timerSimulacion) {
    clearTimeout(timerSimulacion);
    timerSimulacion = null;
  }
  
  if (!window.simularActivo) return;

  tiempoSimulado += 0.04;

  const telemetriaSimulada = {
    id: "S",
    tiempoRecibido: Math.floor(tiempoSimulado * 1000),
    numPaquete: Math.floor(tiempoSimulado / 0.04),
    rssi: -60,
    velAngular: {
      x: Math.cos(tiempoSimulado) * 10,
      y: Math.sin(tiempoSimulado) * 10,
      z: 2.0,
    },
    accel: {
      x: Math.sin(tiempoSimulado) * 0.5,
      y: Math.cos(tiempoSimulado) * 0.5,
      z: 0.98,
    },
    gps1: { lat: 32.5027, lon: -117.0038 },
    gps2: { lat: 0, lon: 0 },
    temperaturas: new Array(10).fill(25),
    timestampLocal: Date.now(),
  };

  actualizarTelemetria(telemetriaSimulada);
  timerSimulacion = setTimeout(loopSimulacion, 20); // ~50Hz
}

// ===========================================================================
// 6. Inicialización al cargar el DOM
// ===========================================================================
document.addEventListener("DOMContentLoaded", () => {
  inicializarGraficasIMU();

  const chkSimular = document.getElementById("chk-simular-imu");
  if (chkSimular) {
    chkSimular.addEventListener("change", (e) => {
      window.simularActivo = e.target.checked;
      if (window.simularActivo) loopSimulacion();
    });
  }

  const btnConectar =
    document.getElementById("btn-conectar-telemetria") ||
    document.getElementById("btn-conectar-imu");
  if (btnConectar) {
    btnConectar.addEventListener("click", conectarTelemetriaUSB);
  }

  if (window.simularActivo) loopSimulacion();
});

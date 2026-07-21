/**
 * Módulo de Telemetría Web Serial API para Sensores USB (IMU + GPS)
 */

// Variables globales del módulo
window.simularActivo = true;
let puertoSerial = null;
let lectorSerial = null;
let ejecutandoLectura = false;
let tiempoSimulado = 0;

// Variables de Chart.js
let chartAccel = null;
let chartGyro = null;
const MAX_PUNTOS_GRAFICA = 30; // Muestra los últimos 30 datos recibidos

// 1. Actualización visual en la interfaz (Números y Modelo 3D)
function actualizarValoresIMU(pitch, roll, yaw) {
    const elPitch = document.getElementById("txt-imu-pitch");
    const elRoll = document.getElementById("txt-imu-roll");
    const elYaw = document.getElementById("txt-imu-yaw");

    if (elPitch) elPitch.textContent = `${pitch.toFixed(2)}°`;
    if (elRoll) elRoll.textContent = `${roll.toFixed(2)}°`;
    if (elYaw) elYaw.textContent = `${yaw.toFixed(2)}°`;

    // Si el objeto 3D ya existe en Three.js, aplicamos la rotación
    if (typeof roverMesh !== "undefined" && roverMesh) {
        const pitchRad = (pitch * Math.PI) / 180;
        const rollRad = (roll * Math.PI) / 180;
        const yawRad = (yaw * Math.PI) / 180;

        roverMesh.rotation.set(0, 0, 0);
        roverMesh.rotateY(yawRad);   // Guiñada / Rumbo
        roverMesh.rotateZ(pitchRad); // Cabeceo
        roverMesh.rotateX(rollRad);   // Alabeo
    }
}

// 2. Bucle de Simulación Matemática
function loopSimulacion() {
    if (!window.simularActivo) return;

    tiempoSimulado += 0.04;
    const pitch = Math.sin(tiempoSimulado) * 20.0;
    const roll = Math.cos(tiempoSimulado * 0.7) * 35.0;
    const yaw = (tiempoSimulado * 15.0) % 360;

    actualizarValoresIMU(pitch, roll, yaw);

    // Simulación de valores de sensores físicos
    const ax = Math.sin(tiempoSimulado) * 0.5;
    const ay = Math.cos(tiempoSimulado) * 0.5;
    const az = 0.98; // Fuerza de Gravedad aprox.
    const gx = Math.cos(tiempoSimulado) * 10;
    const gy = Math.sin(tiempoSimulado) * 10;
    const gz = 2.0;

    actualizarGraficasIMU(ax, ay, az, gx, gy, gz);
    
    setTimeout(loopSimulacion, 20); // Refresco a 50Hz (20ms)
}

// 3. Conexión directa mediante Web Serial API
// Variable global adicional para controlar el flujo de decodificación
let lectorWritable = null;

async function conectarTelemetriaUSB() {
    if (!("serial" in navigator)) {
        alert("Tu navegador no soporta Web Serial API. Usa Google Chrome o Microsoft Edge.");
        return;

        // 1. Cuando se presiona "Desconectar USB" o hay un error:
        setEstadoIMU(false);
        setEstadoGPS(false);

        // 2. Dentro del bucle while (mientras recibimos datos por USB):
    if (datos.length >= 9) {
        // Si llegan datos válidos del IMU
        setEstadoIMU(true);

    // Si además la trama trae coordenadas válidas de GPS (Lat y Lng distintas de 0)
    if (datos.length >= 3) {
        const lat = parseFloat(datos[9]);
        const lng = parseFloat(datos[10]);
        
        if (!isNaN(lat) && !isNaN(lng) && lat !== 0.0 && lng !== 0.0) {
            setEstadoGPS(true); // ¡GPS tiene señal física activa!
        } else {
            setEstadoGPS(false); // Conectado pero buscando satélites (Lat/Lng en 0)
        }
    }
}
    }

    const btnConectar = document.getElementById("btn-conectar-telemetria") || document.getElementById("btn-conectar-imu");
    const txtEstado = document.getElementById("txt-estado-conexion");

// =========================================================
// 🔌 LÓGICA DE DESCONEXIÓN LIMPIA Y REVOCACIÓN INMEDIATA
// =========================================================
if (ejecutandoLectura) {
    ejecutandoLectura = false; // Detiene el bucle de lectura inmediatamente

    // 1. Cambiar la interfaz DE INMEDIATO para dar respuesta al usuario
    if (btnConectar) {
        btnConectar.textContent = "🔌 Conectar USB";
        btnConectar.className = "btn btn--on";
    }
    if (txtEstado) {
        txtEstado.textContent = "(Desconectado)";
        txtEstado.style.color = "#888";
    }

    // 2. Liberar recursos y borrar permisos en segundo plano
    (async () => {
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
                if ("forget" in puertoSerial) {
                    await puertoSerial.forget();
                }
                puertoSerial = null;
            }

            // Forzar olvido de cualquier otro puerto vinculado previamente
            if ("getPorts" in navigator.serial) {
                const puertosPrevios = await navigator.serial.getPorts();
                for (const puerto of puertosPrevios) {
                    if ("forget" in puerto) {
                        await puerto.forget();
                    }
                }
            }

            console.log("Puerto USB liberado y permisos revocados correctamente.");
        } catch (err) {
            console.error("Error al liberar recursos USB:", err);
        }
    })();

    return; // Sale de la función de inmediato
}

    // =========================================================
    // 🟢 LÓGICA DE CONEXIÓN USB (Con limpieza previa)
    // =========================================================
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

        // Solicitar seleccionar un nuevo puerto USB
        puertoSerial = await navigator.serial.requestPort();
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

        // Apagar simulación
        const chkSimular = document.getElementById("chk-simular-imu");
        if (chkSimular) chkSimular.checked = false;
        window.simularActivo = false;

        const textDecoder = new TextDecoderStream();
        lectorWritable = textDecoder.writable;
        
        puertoSerial.readable.pipeTo(lectorWritable).catch(() => {});

        const inputStream = textDecoder.readable;
        lectorSerial = inputStream.getReader();

        let bufferTrama = "";

        while (ejecutandoLectura) {
            const { value, done } = await lectorSerial.read();
            if (done) break;
            
            if (value) {
                bufferTrama += value;
                let lineas = bufferTrama.split("\n");
                bufferTrama = lineas.pop();

                for (let linea of lineas) {
                    linea = linea.trim();
                    if (linea.length > 0) {
                        const datos = linea.split(",");
                        
                        if (datos.length >= 9) {
                            const pitch = parseFloat(datos[0]);
                            const roll  = parseFloat(datos[1]);
                            const yaw   = parseFloat(datos[2]);

                            const ax = parseFloat(datos[3]);
                            const ay = parseFloat(datos[4]);
                            const az = parseFloat(datos[5]);

                            const gx = parseFloat(datos[6]);
                            const gy = parseFloat(datos[7]);
                            const gz = parseFloat(datos[8]);

                            if (!isNaN(pitch) && !isNaN(roll) && !isNaN(yaw)) {
                                actualizarValoresIMU(pitch, roll, yaw);
                            }

                            if (!isNaN(ax) && !isNaN(gx)) {
                                actualizarGraficasIMU(ax, ay, az, gx, gy, gz);
                            }

                            if (datos.length >= 11) {
                                const lat = parseFloat(datos[9]);
                                const lng = parseFloat(datos[10]);
                                if (!isNaN(lat) && !isNaN(lng) && lat !== 0.0 && lng !== 0.0) {
                                    document.dispatchEvent(new CustomEvent("actualizarGPS", {
                                        detail: { lat: lat, lng: lng }
                                    }));
                                }
                            }
                        }
                    }
                }
            }
        }
    } catch (err) {
        console.error("Error en conexión USB:", err);
        ejecutandoLectura = false;
        if (lectorSerial) {
            try { lectorSerial.releaseLock(); } catch(e){}
            lectorSerial = null;
        }
        if (puertoSerial) {
            puertoSerial = null;
        }
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

// 4. Configuración e Inicialización de Gráficas (Chart.js)
function inicializarGraficasIMU() {
    const ctxAccel = document.getElementById('chart-acelerometro')?.getContext('2d');
    const ctxGyro = document.getElementById('chart-giroscopio')?.getContext('2d');

    if (!ctxAccel || !ctxGyro) return;

    const configBase = {
        type: 'line',
        options: {
            responsive: true,
            animation: false, // Desactivar animación para máximo rendimiento
            scales: {
                x: { display: false },
                y: { grid: { color: 'rgba(255, 255, 255, 0.05)' }, ticks: { color: '#777', font: { size: 9 } } }
            },
            plugins: { legend: { labels: { color: '#ccc', font: { size: 10 }, boxWidth: 10 } } }
        }
    };

    // Gráfica de Aceleración X, Y, Z
    chartAccel = new Chart(ctxAccel, {
        ...configBase,
        data: {
            labels: Array(MAX_PUNTOS_GRAFICA).fill(''),
            datasets: [
                { label: 'Acc X', borderColor: '#ff4d4d', data: [], borderWidth: 1.5, pointRadius: 0 },
                { label: 'Acc Y', borderColor: '#2ecc71', data: [], borderWidth: 1.5, pointRadius: 0 },
                { label: 'Acc Z', borderColor: '#3498db', data: [], borderWidth: 1.5, pointRadius: 0 }
            ]
        }
    });

    // Gráfica de Velocidad Angular del Giroscopio (Gyro X, Y, Z)
    chartGyro = new Chart(ctxGyro, {
        ...configBase,
        data: {
            labels: Array(MAX_PUNTOS_GRAFICA).fill(''),
            datasets: [
                { label: 'Gyr X', borderColor: '#f1c40f', data: [], borderWidth: 1.5, pointRadius: 0 },
                { label: 'Gyr Y', borderColor: '#9b59b6', data: [], borderWidth: 1.5, pointRadius: 0 },
                { label: 'Gyr Z', borderColor: '#e67e22', data: [], borderWidth: 1.5, pointRadius: 0 }
            ]
        }
    });
}

function actualizarGraficasIMU(ax, ay, az, gx, gy, gz) {
    if (!chartAccel || !chartGyro) return;

    const agregarDato = (chart, datos) => {
        chart.data.datasets.forEach((dataset, index) => {
            if (dataset.data.length >= MAX_PUNTOS_GRAFICA) {
                dataset.data.shift();
            }
            dataset.data.push(datos[index]);
        });
        chart.update('none'); // Update rápido
    };

    agregarDato(chartAccel, [ax, ay, az]);
    agregarDato(chartGyro, [gx, gy, gz]);
}

// 5. Inicialización automática UNIFICADA del DOM (Al final de todo)
document.addEventListener("DOMContentLoaded", () => {
    // 1. Inicializar los lienzos de Chart.js
    if (typeof inicializarGraficasIMU === "function") {
        inicializarGraficasIMU();
    }

    // 2. Vincular el evento del Checkbox de Simulación
    const chkSimular = document.getElementById("chk-simular-imu");
    if (chkSimular) {
        chkSimular.addEventListener("change", (e) => {
            window.simularActivo = e.target.checked;
            if (window.simularActivo) {
                loopSimulacion();
            }
        });
    }

    // 3. Vincular el Botón de Conectar USB
    const btnConectar = document.getElementById("btn-conectar-telemetria") || document.getElementById("btn-conectar-imu");
    if (btnConectar) {
        btnConectar.addEventListener("click", conectarTelemetriaUSB);
    }

    // 4. Arrancar la simulación de prueba al cargar la página
    if (window.simularActivo) {
        loopSimulacion();
    }
});

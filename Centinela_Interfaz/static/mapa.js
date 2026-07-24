// Integración de GNSS (Flask API + Telemetría USB + CSV Cronológico)

document.addEventListener("DOMContentLoaded", () => {

    //-------- CREACIÓN DEL MAPA --------
    const map = L.map("map").setView([32.514, -117.038], 16);

    L.tileLayer(
        "https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png",
        {
            attribution: "&copy; OpenStreetMap contributors"
        }
    ).addTo(map);

    // -------- ÍCONO DEL ROVER --------
    const roverIcon = L.icon({
        iconUrl: "/static/Rover.png",
        iconSize: [40, 40],
        iconAnchor: [20, 20],
        popupAnchor: [0, -20]
    });

    //------ POSICIÓN INICIAL DEL ROVER --------
    let latitude = 32.514;
    let longitude = -117.038;

    // ----- MARCADOR INICIAL ROVER -----
    const marker = L.marker(
        [latitude, longitude],
        { icon: roverIcon }
    ).addTo(map);

    // ---------------------- RUTA -----------------------------
    let recordingRoute = false;
    const route = [];

    const polyline = L.polyline([], {
        color: "red",
        weight: 4
    }).addTo(map);

    // ------------------- WAYPOINTS -----------------------------
    let waypointCount = 1;
    const waypoints = [];
    const waypointMarkers = [];

    // Arreglo unificado para ordenar eventos por tiempo en el CSV
    let eventosRuta = [];

    // ------------------- BOTONES DE RUTA -----------------------
    const startButton = document.getElementById("startRouteBtn");
    const stopButton = document.getElementById("stopRouteBtn");
    const clearButton = document.getElementById("clearRouteBtn");
    const waypointButton = document.getElementById("addWaypointBtn");
    const chkSimularGPS = document.getElementById("chk-simular-gps");

    // EMPEZAR RUTA
    if (startButton) {
        startButton.addEventListener("click", () => {
            recordingRoute = true;
            fetch("/api/mission/start", { method: "POST" }).catch(() => {});
            console.log("Grabación de ruta iniciada...");
        });
    }

    // DETENER RUTA Y GENERAR CSV CRONOLÓGICO
    if (stopButton) {
        stopButton.addEventListener("click", () => {
            fetch("/api/mission/stop", { method: "POST" }).catch(() => {});

            if (!recordingRoute && eventosRuta.length === 0) {
                alert("No hay ninguna ruta o datos grabados para exportar.");
                return;
            }

            recordingRoute = false;

            if (eventosRuta.length === 0) {
                alert("La ruta finalizó pero no se registraron puntos.");
                return;
            }

            // 1. ORDENAR EVENTOS POR MARCA DE TIEMPO (Cronológicamente)
            eventosRuta.sort((a, b) => a.timestamp - b.timestamp);

            // 2. Construir la estructura del CSV
            let contenidoCSV = "Tipo,Indice/Nombre,Hora_Registro,Latitud,Longitud\n";
            let contadorRuta = 1;

            eventosRuta.forEach((evento) => {
                const horaLegible = new Date(evento.timestamp).toLocaleTimeString();

                if (evento.tipo === "Ruta") {
                    contenidoCSV += `Ruta,${contadorRuta},${horaLegible},${evento.lat},${evento.lng}\n`;
                    contadorRuta++;
                } else if (evento.tipo === "Waypoint") {
                    contenidoCSV += `Waypoint,${evento.nombre},${horaLegible},${evento.lat},${evento.lng}\n`;
                }
            });

            // 3. Generar y descargar el archivo .csv
            const blob = new Blob([contenidoCSV], { type: "text/csv;charset=utf-8;" });
            const url = URL.createObjectURL(blob);
            const enlaceDescarga = document.createElement("a");

            enlaceDescarga.href = url;
            const fecha = new Date().toISOString().slice(0, 10);
            const hora = new Date().toTimeString().slice(0, 8).replace(/:/g, "-");

            enlaceDescarga.setAttribute("download", `Ruta_GPS_Intercalada_${fecha}_${hora}.csv`);

            document.body.appendChild(enlaceDescarga);
            enlaceDescarga.click();
            document.body.removeChild(enlaceDescarga);
            URL.revokeObjectURL(url);

            console.log("CSV ordenado cronológicamente descargado con éxito.");
        });
    }

    // LIMPIAR RUTA Y WAYPOINTS
    if (clearButton) {
        clearButton.addEventListener("click", () => {
            const confirmed = confirm(
                "¿Estás seguro que quieres limpiar la ruta actual y todos los waypoints?\n\nEsta acción no se puede deshacer."
            );

            if (!confirmed) return;

            // Limpiar datos
            route.length = 0;
            eventosRuta.length = 0;
            polyline.setLatLngs([]);

            // Limpiar marcadores del mapa
            waypointMarkers.forEach(m => map.removeLayer(m));
            waypointMarkers.length = 0;
            waypoints.length = 0;
            waypointCount = 1;
        });
    }

    // AGREGAR WAYPOINT
    if (waypointButton) {
        waypointButton.addEventListener("click", () => {
            const ahora = Date.now();
            const nombreWp = `Waypoint ${waypointCount}`;

            const waypoint = {
                lat: latitude,
                lon: longitude,
                timestamp: ahora
            };

            waypoints.push(waypoint);

            eventosRuta.push({
                tipo: "Waypoint",
                nombre: nombreWp,
                lat: latitude,
                lng: longitude,
                timestamp: ahora
            });

            const waypointMarker = L.marker([waypoint.lat, waypoint.lon])
                .addTo(map)
                .bindPopup(`
                    <b>${nombreWp}</b><br>
                    Latitude: ${waypoint.lat.toFixed(6)}<br>
                    Longitude: ${waypoint.lon.toFixed(6)}
                `);

            waypointMarkers.push(waypointMarker);

            if (recordingRoute) {
                fetch("/api/mission/waypoint", {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({
                        latitude: waypoint.lat,
                        longitude: waypoint.lon,
                    }),
                }).catch(() => {});
            }

            waypointCount++;
        });
    }

    // =========================================================
    // 📍 FUNCIÓN REUTILIZABLE PARA ACTUALIZAR LA POSICIÓN ROVER
    // =========================================================
    function actualizarPosicionRover(nuevaLat, nuevaLng) {
        latitude = nuevaLat;
        longitude = nuevaLng;

        // 1. Mover el marcador
        marker.setLatLng([latitude, longitude]);

        // 2. Centrar suavemente el mapa en el Rover
        map.panTo([latitude, longitude]);

        // 3. Registrar punto si la ruta está activa
        if (recordingRoute) {
            const ahora = Date.now();

            route.push([latitude, longitude]);
            polyline.setLatLngs(route);

            eventosRuta.push({
                tipo: "Ruta",
                lat: latitude,
                lng: longitude,
                timestamp: ahora
            });

            fetch("/api/mission/point", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({
                    latitude: latitude,
                    longitude: longitude,
                }),
            }).catch(() => {});
        }
    }

    // =========================================================
    // 📡 ESCUCHAR DATOS REALES DE TELEMETRÍA (USB Web Serial)
    // =========================================================
    document.addEventListener("actualizarGPS", (e) => {
        if (chkSimularGPS && chkSimularGPS.checked) {
            chkSimularGPS.checked = false;
        }

        const { lat, lng } = e.detail;
        actualizarPosicionRover(lat, lng);
    });

    // =========================================================
    // 🎲 BUCLE DE SIMULACIÓN Y CONSULTA A API FLASK DE FALLBACK
    // =========================================================
    setInterval(() => {
        // Opción A: Si la simulación con checkbox está activa
        if (chkSimularGPS && chkSimularGPS.checked) {
            const simLat = latitude + (Math.random() - 0.5) * 0.0002;
            const simLng = longitude + (Math.random() - 0.5) * 0.0002;

            actualizarPosicionRover(simLat, simLng);
        } else {
            // Opción B: Probar consultar la API Flask local si existe
            fetch("/api/telemetry/latest")
                .then((res) => res.json())
                .then((data) => {
                    if (data && data.latitude && data.longitude) {
                        actualizarPosicionRover(data.latitude, data.longitude);
                    }
                })
                .catch(() => {});
        }
    }, 500);
    // ==========================================
    // Guardar misión al cerrar la página
    // ==========================================
  window.addEventListener("beforeunload", () => {

    navigator.sendBeacon("/api/mission/stop");

    });

    });

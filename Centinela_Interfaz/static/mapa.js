// Integracion de GNSS

document.addEventListener("DOMContentLoaded", () => {
  //-------- CREACION DEL MAPA --------
  const map = L.map("map").setView([32.514, -117.038], 16);

  // Se crean los tiles del mapa usando OpenStreetMap
  L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", {
    attribution: "&copy; OpenStreetMap contributors",
  }).addTo(map);
  // -------- FIN DE CREACION DEL MAPA --------

  // -------- ICONO DEL ROVER --------
  const roverIcon = L.icon({
    iconUrl: "/static/Rover.png",

    iconSize: [40, 40],
    iconAnchor: [20, 20],
    popupAnchor: [0, -20],
  });
  // -------- FIN DEL ICONO DEL ROVER --------

  //------ POSICION INICIAL DEL ROVER --------
  let latitude = 32.514;
  let longitude = -117.038;
  //------ FIN DE POSICION INICIAL DEL ROVER --------

  // ----- MARCADOR INICIAL ROVER -----
  const marker = L.marker([latitude, longitude], { icon: roverIcon }).addTo(
    map,
  );
  // ----- FIN DEL MARCADOR INICIAL ROVER -----

  // ---------------------- RUTA -----------------------------
  let recordingRoute = false;

  const route = [];

  const polyline = L.polyline(route, {
    color: "red",
    weight: 4,
  }).addTo(map);
  // -------------------- FIN DE RUTA -------------------------

  // ------------------- WAYPOINTS -----------------------------
  let waypointCount = 1;
  const waypoints = [];
  const waypointMarkers = [];
  // -----------------------------------------------------------

  // ------------------- BOTONES DE RUTA -----------------------
  const startButton = document.getElementById("startRouteBtn");
  const stopButton = document.getElementById("stopRouteBtn");
  const clearButton = document.getElementById("clearRouteBtn");
  const waypointButton = document.getElementById("addWaypointBtn");
  // -----------------------------------------------------------

  // EMPEZAR RUTA
  startButton.addEventListener("click", () => {
    recordingRoute = true;

    fetch("/api/mission/start", {
      method: "POST",
    });
  });
  // DETENER RUTA
  stopButton.addEventListener("click", () => {
    recordingRoute = false;

    fetch("/api/mission/stop", {
      method: "POST",
    });
  });

  // LIMPIAR RUTA y WAYPOINTS
  clearButton.addEventListener("click", () => {
    const confirmed = confirm(
      "¿Estas seguro que quieres limpiar la ruta actual y todos los waypoints?\n\nEsta acción no se puede deshacer.",
    );

    if (!confirmed) {
      return;
    }

    // Limpiar la ruta
    route.length = 0;
    polyline.setLatLngs(route);

    // Limpiar waypoints del mapa
    waypointMarkers.forEach((marker) => {
      map.removeLayer(marker);
    });

    // Vaciar los arrays de waypoints y waypointMarkers
    waypointMarkers.length = 0;
    waypoints.length = 0;

    // Rsetear el contador de waypoints
    waypointCount = 1;
  });

  // AGREGAR WAYPOINT
  waypointButton.addEventListener("click", () => {
    const waypoint = {
      lat: latitude,
      lon: longitude,
    };

    waypoints.push(waypoint);

    const waypointMarker = L.marker([waypoint.lat, waypoint.lon]).addTo(map)
      .bindPopup(`
                <b>Waypoint ${waypointCount}</b><br>
                Latitude: ${waypoint.lat.toFixed(6)}<br>
                Longitude: ${waypoint.lon.toFixed(6)}
            `);

    waypointMarkers.push(waypointMarker);
    if (recordingRoute) {
      fetch("/api/mission/waypoint", {
        method: "POST",

        headers: {
          "Content-Type": "application/json",
        },

        body: JSON.stringify({
          latitude: waypoint.lat,
          longitude: waypoint.lon,
        }),
      });
    }
    waypointCount++;
  });

  // Simulacion. IMPORTANTE: Reemplazar con datos de GPS!!!!!!!!
  setInterval(() => {
    fetch("/api/telemetry/latest")
      .then((response) => response.json())
      .then((data) => {
        latitude = data.latitude;
        longitude = data.longitude;

        marker.setLatLng([latitude, longitude]);

        if (recordingRoute) {
          route.push([latitude, longitude]);

          polyline.setLatLngs(route);

          fetch("/api/mission/point", {
            method: "POST",
            headers: {
              "Content-Type": "application/json",
            },
            body: JSON.stringify({
              latitude: latitude,
              longitude: longitude,
            }),
          });
        }
      })
      .catch((error) => {
        console.log("Telemetry error:", error);
      });
  }, 250);
});

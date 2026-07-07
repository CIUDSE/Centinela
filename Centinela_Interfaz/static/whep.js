/*****************************************************************************************************************************
Club de Investigación Univesitario de Desarrollo en Sistemas Espaciales
Misión Centinela

Código desarrollado por Software Rovers

Archivo JavaScript que implementa WebRTC con WHEP para la transmisión de video en tiempo real desde las cámaras del sistema Centinela.
*****************************************************************************************************************************/

const whepConnections = {}; // guarda el RTCPeerConnection de cada cámara

async function conectarCamara(videoElementId, whepUrl) {
  // Si ya hay una conexión activa, no abrir otra
  if (whepConnections[videoElementId]) {
    console.log(`${videoElementId} ya está conectada.`);
    return;
  }

  ocultarError("camaras-error"); // limpia error previo al reintentar

  // Crear un nuevo RTCPeerConnection y configurar el elemento de video
  const video = document.getElementById(videoElementId);
  const pc = new RTCPeerConnection({ iceServers: [] });

  pc.ontrack = (event) => {
    if (video.srcObject !== event.streams[0]) {
      video.srcObject = event.streams[0]; // Asignar el stream recibido al elemento de video
    }
  };

  // Agregar un transceptor de video en modo "recvonly" para recibir el stream de la cámara, NO audio.
  pc.addTransceiver("video", { direction: "recvonly" });

  pc.onconnectionstatechange = () => {
    console.log(`${videoElementId} estado:`, pc.connectionState);
    if (
      pc.connectionState === "failed" ||
      pc.connectionState === "disconnected"
    ) {
      mostrarError(
        "camaras-error",
        `⚠️ Se perdió la conexión con ${videoElementId}.`,
      );
      desconectarCamara(videoElementId);
    }
  };

  const offer = await pc.createOffer();
  await pc.setLocalDescription(offer);

  await new Promise((resolve) => {
    if (pc.iceGatheringState === "complete") return resolve();
    pc.addEventListener("icegatheringstatechange", () => {
      if (pc.iceGatheringState === "complete") resolve();
    });
  });

  try {
    const response = await fetch(whepUrl, {
      method: "POST",
      headers: { "Content-Type": "application/sdp" },
      body: pc.localDescription.sdp,
    });

    if (!response.ok) {
      console.error(
        `WHEP error en ${videoElementId}:`,
        response.status,
        await response.text(),
      );
      mostrarError(
        "camaras-error",
        `❌ No se pudo conectar a ${videoElementId} (HTTP ${response.status}).`,
      );
      pc.close();
      return;
    }

    const answerSDP = await response.text();
    await pc.setRemoteDescription({ type: "answer", sdp: answerSDP });

    whepConnections[videoElementId] = pc;
  } catch (err) {
    console.error(`Error conectando ${videoElementId}:`, err);
    mostrarError(
      "camaras-error",
      `❌ Error de red al conectar ${videoElementId}: ${err.message}`,
    );
    pc.close();
  }
}

function desconectarCamara(videoElementId) {
  const pc = whepConnections[videoElementId];
  if (pc) {
    pc.close();
    delete whepConnections[videoElementId];
  }
  const video = document.getElementById(videoElementId);
  if (video) video.srcObject = null;
}

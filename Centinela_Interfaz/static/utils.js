// ── Helpers genéricos de error, reutilizables en cualquier panel ──
function mostrarError(elementId, mensaje) {
  const el = document.getElementById(elementId);
  if (!el) return;
  el.textContent = mensaje;
  el.classList.remove("oculto");
}

function ocultarError(elementId) {
  const el = document.getElementById(elementId);
  if (!el) return;
  el.classList.add("oculto");
}

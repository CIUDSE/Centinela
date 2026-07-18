/*
  Estructura de `state`:
 
  Es un arreglo de 3 filas, cada fila con 4 dígitos, cada dígito es un
  objeto con el estado (0 = apagado, 1 = encendido) de sus 7 segmentos.
 
  state[r][c] = { a, b, c, d, e, f, g }
 
  Ejemplo con solo el primer dígito de la fila 0 encendido para formar
  la letra 'A' (0b01000001 → segmentos a, b, c, f, g encendidos):
 
  state = [
    [
      { a: 1, b: 1, c: 1, d: 0, e: 0, f: 1, g: 1 },  // fila 0, columna 0 → 'A'
      { a: 0, b: 0, c: 0, d: 0, e: 0, f: 0, g: 0 },  // fila 0, columna 1 → apagado
      { a: 0, b: 0, c: 0, d: 0, e: 0, f: 0, g: 0 },  // fila 0, columna 2 → apagado
      { a: 0, b: 0, c: 0, d: 0, e: 0, f: 0, g: 0 },  // fila 0, columna 3 → apagado
    ],
    [ ... 4 dígitos de la fila 1 ... ],
    [ ... 4 dígitos de la fila 2 ... ],
  ]
*/

// Coordenadas del SVG de cada segmento de un dígito de 7 segmentos
var SEGP = {
  a: "30,2 130,2 144,16 130,30 30,30 16,16",
  b: "144,16 158,30 158,128 144,142 130,128 130,30",
  c: "144,152 158,166 158,264 144,278 130,264 130,166",
  d: "30,250 130,250 144,264 130,278 30,278 16,264",
  e: "16,152 30,166 30,264 16,278 2,264 2,166",
  f: "16,16 30,30 30,128 16,142 2,128 2,30",
  g: "30,128 130,128 144,142 130,156 30,156 16,142",
};

// Orden de cada segmento. Para generar el binario se utiliza reverse() ya que el orden es inverso.
var order = ["a", "b", "c", "d", "e", "f", "g"];

var state = [];

// Llenar el estado inicial con 3 filas y 4 columnas de dígitos, todos apagados
for (var r = 0; r < 3; r++) {
  var row = [];
  for (var c = 0; c < 4; c++) {
    row.push({ a: 0, b: 0, c: 0, d: 0, e: 0, f: 0, g: 0 });
  }
  state.push(row);
}

// Construye el <svg> de un dígito en la posición (r, c): dibuja los 7
// polígonos de segmento, pintando de rojo los que están encendidos en
// `state` y de gris los apagados. Cada polygon lleva data-r/data-c/data-seg
// para que el click listener sepa qué segmento de qué dígito fue tocado.
function digitSVG(r, c) {
  var seg = state[r][c];
  var parts = order
    .map(function (k) {
      var on = seg[k];
      return (
        '<polygon class="err2-seg" data-r="' +
        r +
        '" data-c="' +
        c +
        '" data-seg="' +
        k +
        '" points="' +
        SEGP[k] +
        '" fill="' +
        (on ? "#e24b4a" : "#2a2a2a") +
        '" stroke="#555" stroke-width="1"/>'
      );
    })
    .join("");
  return (
    '<svg viewBox="0 0 160 280" width="64" height="112">' + parts + "</svg>"
  );
}

// Toma el objeto de un digito y arma el string binario en el formato 0gfedcba
function binStr(seg) {
  return (
    "0" +
    order
      .slice() // hace una copia interna para no modificar el array original
      .reverse() // invierte el orden para que quede gfedcba
      .map(function (k) {
        return seg[k];
      })
      .join("")
  );
}

// Renderiza la matriz de dígitos en la interfaz y agrega los event listeners para cambiar el estado de los segmentos
function renderError2Matrix() {
  var rowsEl = document.getElementById("err2-rows");
  var html = "";

  // Recorre las 3 filas y arma una fila de HTML por cada una, con su
  // número de fila (1, 2, 3) a la izquierda
  for (var r = 0; r < 3; r++) {
    html +=
      '<div class="err2-row"><span class="err2-row-label">' +
      (r + 1) +
      "</span>";
    for (var c = 0; c < 4; c++) {
      html +=
        '<div class="err2-digit">' +
        digitSVG(r, c) +
        '<div class="err2-digit-bin">' +
        binStr(state[r][c]) +
        "</div></div>";
    }
    html += "</div>";
  }
  // Inserta todo el HTML armado de una sola vez en el contenedor
  rowsEl.innerHTML = html;

  // Como innerHTML acaba de recrear todos los <polygon>, hay que volver
  // a engancharles el evento de click (los anteriores se perdieron)
  Array.prototype.forEach.call(
    rowsEl.querySelectorAll("polygon"),
    function (p) {
      p.addEventListener("click", function () {
        // Lee de los data-attributes en qué dígito y segmento se hizo click
        var r = +p.dataset.r,
          c = +p.dataset.c,
          seg = p.dataset.seg;
        // Invierte el bit de ese segmento (0 → 1, o 1 → 0)
        state[r][c][seg] = state[r][c][seg] ? 0 : 1;

        // Vuelve a dibujar todo con el nuevo estado y recalcula el XOR
        renderError2Matrix();
        computeError2Matrix();
      });
    },
  );
}

// Calcula el XOR posicional entre las 3 filas para cada una de las 4
// columnas, decodifica cada byte resultante a un carácter ASCII, y
// muestra tanto los bytes como la secuencia de switches (Arriba/Abajo)
function computeError2Matrix() {
  var resultEl = document.getElementById("err2-result");
  var seqEl = document.getElementById("err2-seq");
  var chars = [];
  var html = "";

  // Recorre cada una de las 4 columnas (dígitos) por separado
  for (var c = 0; c < 4; c++) {
    var byte = 0;
    // Recorre cada uno de los 7 bits de segmento (a a g)
    for (var bit = 0; bit < 7; bit++) {
      var k = order[bit];
      var x = 0;
      // XOR posicional: combina el mismo segmento k de las 3 filas
      // (fila 0 XOR fila 1 XOR fila 2) para esa columna
      for (var r = 0; r < 3; r++) {
        x ^= state[r][c][k];
      }
      // Si el resultado del XOR es 1, prende ese bit en el byte final
      // usando desplazamiento de bits (bit shift) según su posición
      if (x) byte |= 1 << bit;
    }
    // Convierte el byte numérico resultante a su carácter ASCII
    var ch = String.fromCharCode(byte);
    chars.push(ch);
    // Arma el string binario con el mismo formato "0gfedcba" para mostrarlo
    var bstr = "0" + byte.toString(2).padStart(7, "0");
    html +=
      "<div><div>" +
      bstr +
      "</div><div>" +
      byte +
      ' = "' +
      (ch.trim() ? ch : "?") +
      '"</div></div>';
  }
  resultEl.innerHTML = html;

  // Traduce cada carácter resultante a Arriba/Abajo si es U/u o D/d,
  // o lo deja tal cual entre comillas si no corresponde a un switch conocido
  var seq = chars
    .map(function (ch) {
      if (ch === "U" || ch === "u") return "Arriba";
      if (ch === "D" || ch === "d") return "Abajo";
      return '"' + ch + '"';
    })
    .join(", ");
  seqEl.textContent = "Secuencia de switches: " + seq;
}

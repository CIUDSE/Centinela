var SEGP = {
  a: "30,2 130,2 144,16 130,30 30,30 16,16",
  b: "144,16 158,30 158,128 144,142 130,128 130,30",
  c: "144,152 158,166 158,264 144,278 130,264 130,166",
  d: "30,250 130,250 144,264 130,278 30,278 16,264",
  e: "16,152 30,166 30,264 16,278 2,264 2,166",
  f: "16,16 30,30 30,128 16,142 2,128 2,30",
  g: "30,128 130,128 144,142 130,156 30,156 16,142",
};
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

function binStr(seg) {
  return (
    "0" +
    order
      .slice()
      .reverse()
      .map(function (k) {
        return seg[k];
      })
      .join("")
  );
}

function renderError2Matrix() {
  var rowsEl = document.getElementById("err2-rows");
  var html = "";
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
  rowsEl.innerHTML = html;

  Array.prototype.forEach.call(
    rowsEl.querySelectorAll("polygon"),
    function (p) {
      p.addEventListener("click", function () {
        var r = +p.dataset.r,
          c = +p.dataset.c,
          seg = p.dataset.seg;
        state[r][c][seg] = state[r][c][seg] ? 0 : 1;
        renderError2Matrix();
        computeError2Matrix();
      });
    },
  );
}

function computeError2Matrix() {
  var resultEl = document.getElementById("err2-result");
  var seqEl = document.getElementById("err2-seq");
  var chars = [];
  var html = "";

  for (var c = 0; c < 4; c++) {
    var byte = 0;
    for (var bit = 0; bit < 7; bit++) {
      var k = order[bit];
      var x = 0;
      for (var r = 0; r < 3; r++) {
        x ^= state[r][c][k];
      }
      if (x) byte |= 1 << bit;
    }
    var ch = String.fromCharCode(byte);
    chars.push(ch);
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

  var seq = chars
    .map(function (ch) {
      if (ch === "U" || ch === "u") return "Arriba";
      if (ch === "D" || ch === "d") return "Abajo";
      return '"' + ch + '"';
    })
    .join(", ");
  seqEl.textContent = "Secuencia de switches: " + seq;
}

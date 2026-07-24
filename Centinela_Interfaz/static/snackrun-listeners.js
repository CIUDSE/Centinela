document.getElementById("err2-reset").addEventListener("click", function () {
  for (var r = 0; r < 3; r++)
    for (var c = 0; c < 4; c++) for (var k in state[r][c]) state[r][c][k] = 0;
  renderError2Matrix();
  computeError2Matrix();
});

document.getElementById("err2-demo").addEventListener("click", function () {
  var bytes = [
    ["01001101", "01000010", "01100001", "00100001"],
    ["00000101", "00011000", "01011010", "01001001"],
    ["01110110", "00000100", "01100101", "01010110"],
  ];
  for (var r = 0; r < 3; r++) {
    for (var c = 0; c < 4; c++) {
      var b = bytes[r][c];
      for (var i = 0; i < 7; i++) {
        var k = order[i];
        state[r][c][k] = +b[7 - i];
      }
    }
  }
  renderError2Matrix();
  computeError2Matrix();
});

document.getElementById("err2-copy").addEventListener("click", function () {
  var text = document.getElementById("err2-seq").textContent;
  navigator.clipboard.writeText(text);
});

renderError2Matrix();
computeError2Matrix();

(function () {
  const API_BASE = 'http://localhost:8080';
  const inputEl = document.getElementById('inputCommands');
  const outputEl = document.getElementById('outputCommands');
  const btnExecute = document.getElementById('btnExecute');
  const scriptFile = document.getElementById('scriptFile');

  function appendOutput(text) {
    const current = outputEl.textContent;
    outputEl.textContent = current ? current + '\n' + text : text;
  }

  function clearOutput() {
    outputEl.textContent = '';
  }

  scriptFile.addEventListener('change', function (e) {
    const file = e.target.files && e.target.files[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = function () {
      inputEl.value = reader.result;
    };
    reader.readAsText(file);
    e.target.value = '';
  });

  btnExecute.addEventListener('click', async function () {
    const input = inputEl.value.trim();
    if (!input) {
      appendOutput('(No hay comandos para ejecutar)');
      return;
    }
    clearOutput();
    appendOutput('> Ejecutando...');

    try {
      const res = await fetch(API_BASE + '/execute', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ input: input })
      });
      const data = await res.json();
      if (data.output !== undefined) {
        appendOutput(data.output);
      } else {
        appendOutput('Error: respuesta inválida del servidor');
      }
    } catch (err) {
      appendOutput('Error de conexión: ' + err.message + '. ¿Está el servidor en marcha en el puerto 8080?');
    }
  });
})();

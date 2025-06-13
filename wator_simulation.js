var statusElement = document.getElementById('status');
var spinnerElement = document.getElementById('spinner');
var canvasElement = document.getElementById('canvas');
var outputElement = document.getElementById('output');
if (outputElement) outputElement.value = ''; // clear browser cache

var Module = {
print(...args) {
    console.log(...args);
    // These replacements are necessary if you render to raw HTML
    //text = text.replace(/&/g, "&amp;");
    //text = text.replace(/</g, "&lt;");
    //text = text.replace(/>/g, "&gt;");
    //text = text.replace('\n', '<br>', 'g');
    if (outputElement) {
    var text = args.join(' ');
    outputElement.value += text + "\n";
    outputElement.scrollTop = outputElement.scrollHeight; // focus on bottom
    }
},
canvas: canvasElement,
setStatus(text) {
    Module.setStatus.last ??= { time: Date.now(), text: '' };
    if (text === Module.setStatus.last.text) return;
    var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
    var now = Date.now();
    // if this is a progress update, skip it if too soon
    if (m && now - Module.setStatus.last.time < 30) return;
    Module.setStatus.last.time = now;
    Module.setStatus.last.text = text;
    if (m) {
    text = m[1];
    spinnerElement.hidden = false;
    } else {
    if (!text) spinnerElement.style.display = 'none';
    }
    statusElement.innerHTML = text;
},
totalDependencies: 0,
monitorRunDependencies(left) {
    this.totalDependencies = Math.max(this.totalDependencies, left);
    Module.setStatus(left ? 'Preparing... (' + (this.totalDependencies-left) + '/' + this.totalDependencies + ')' : 'All downloads complete.');
},
onRuntimeInitialized() {
    this._init_wator(100, 100);
    this.render();
},
CELL_SIZE: 6,
render() {
    let data = this._getCellData();
    const resultView = new Uint8Array(Module.HEAPU8.buffer, data, 100 * 100 * 3);
    /** @type CanvasRenderingContext2D */
    let ctx = this.canvas.getContext('2d');
    ctx.fillStyle = 'black';
    ctx.fillRect(0, 0, 100 * this.CELL_SIZE, 100 * this.CELL_SIZE);
    for (let y = 0; y < 100; y++) {
        for (let x = 0; x < 100; x++) {
            let index = (y * 100 + x) * 3;
            let alive = resultView[index];
            let color = resultView[index+1];
            let bitmap = resultView[index+2];
            if (alive) {
                ctx.fillStyle = 'green';
                ctx.fillRect(x * this.CELL_SIZE, y * this.CELL_SIZE, this.CELL_SIZE, this.CELL_SIZE);
            }
        }
    }
}
};
Module.setStatus('Downloading...');
window.onerror = (event) => {
// TODO: do not warn on ok events like simulating an infinite loop or exitStatus
Module.setStatus('Exception thrown, see JavaScript console');
spinnerElement.style.display = 'none';
Module.setStatus = (text) => {
    if (text) console.error('[post-exception status] ' + text);
};
};

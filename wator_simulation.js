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
IMAGES: [],
initImages() {
    let promises = [];
    if (!this.IMAGES.length) {
        let img = new Image();
        this.IMAGES.push(img);
        promises.push(new Promise(resolve => img.addEventListener('load', () => {resolve();}, {once: true})));
        img.src = "png/fish-0.png";
    }
    return Promise.all(promises);
},
async onRuntimeInitialized() {
    this._init_wator(15, 15);
    await this.initImages();
    this.render();
},
CELL_SIZE: 40,
COLOR_NAMES: ['red', 'green', 'blue', 'yellow', 'cyan', 'magenta', 'gray', 'orange'],
async render() {
    let data = this._getCellData();
    const resultView = new Uint8Array(Module.HEAPU8.buffer, data, 15 * 15 * 3);
    /** @type CanvasRenderingContext2D */
    let ctx = this.canvas.getContext('2d');
    ctx.fillStyle = 'black';
    ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
    // TODO?
    /*ctx.imageSmoothingEnabled = true;
    ctx.imageSmoothingQuality = "high";*/
    let tempCanvas = document.createElement("canvas");
    tempCanvas.width = this.CELL_SIZE;
    tempCanvas.height = this.CELL_SIZE;
    //let tempCanvas = document.getElementById("canvasTemp");
    let tempCanvasCtx = tempCanvas.getContext('2d');
    //tempCanvasCtx.scale(this.CELL_SIZE / 20.0, this.CELL_SIZE / 20.0);
    console.log(`img dimensions: ${this.IMAGES[0].width}x${this.IMAGES[0].height}`);
    console.log(`img natural dimensions: ${this.IMAGES[0].naturalWidth}x${this.IMAGES[0].naturalHeight}`);
    console.log(`tempcanvas dimensions: ${tempCanvas.width}x${tempCanvas.height}`);
    console.log(`tempcanvas offset: ${tempCanvas.offsetWidth}x${tempCanvas.offsetHeight}`);
    console.log(`canvas dimensions: ${this.canvas.width}x${this.canvas.height}`);
    console.log(`canvas offset: ${this.canvas.offsetWidth}x${this.canvas.offsetHeight}`);
    for (let y = 0; y < 15; y++) {
        /*ctx.strokeStyle = 'white';
        ctx.beginPath();
        ctx.moveTo(0, this.CELL_SIZE*y);
        ctx.lineTo(this.canvas.width, this.CELL_SIZE*y);
        ctx.stroke();
        ctx.moveTo(this.CELL_SIZE*y, 0);
        ctx.lineTo(this.CELL_SIZE*y, this.canvas.height);
        ctx.stroke();*/
        for (let x = 0; x < 15; x++) {
            let index = (y * 15 + x) * 3;
            let alive = resultView[index];
            let color = resultView[index+1];
            let bitmap = resultView[index+2];
            if (alive) {
                // https://stackoverflow.com/questions/61337596/html5-canvas-filling-transparent-image-with-color-and-drawing-on-top
                tempCanvasCtx.fillStyle = this.COLOR_NAMES[color];
                // this is the default value
                tempCanvasCtx.globalCompositeOperation = "source-over";
                tempCanvasCtx.fillRect(0, 0, this.CELL_SIZE, this.CELL_SIZE);
                tempCanvasCtx.globalCompositeOperation = "destination-in";
                /*const bitmap = await createImageBitmap(this.IMAGES[0], {
                    resizeWidth: this.CELL_SIZE,
                    resizeHeight: this.CELL_SIZE,
                    resizeQuality: 'high'
                });*/
                tempCanvasCtx.drawImage(this.IMAGES[0], 0, 0, 20, 20, 0, 0, this.CELL_SIZE, this.CELL_SIZE);
                //tempCanvasCtx.drawImage(bitmap, 0, 0);
                tempCanvasCtx.globalCompositeOperation = "source-over";
                ctx.drawImage(tempCanvas, 0, 0, this.CELL_SIZE, this.CELL_SIZE, x * this.CELL_SIZE, y * this.CELL_SIZE, this.CELL_SIZE, this.CELL_SIZE);
                //bitmap.close();
                //ctx.drawImage(tempCanvas, x * this.CELL_SIZE, y * this.CELL_SIZE);
                //ctx.fillStyle = this.COLOR_NAMES[color];
                //ctx.fillRect(x * this.CELL_SIZE, y * this.CELL_SIZE, this.CELL_SIZE, this.CELL_SIZE);
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

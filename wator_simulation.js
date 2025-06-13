var canvasElement = document.getElementById('canvas');

var Module = {
canvas: canvasElement,
IMAGES: [],
initImages() {
    let promises = [];
    if (!this.IMAGES.length) {
        for (type of ["fish", "shark"]) {
            for (let i = 0; i < 8; i++) {
                let img = new Image();
                this.IMAGES.push(img);
                promises.push(new Promise(resolve => img.addEventListener('load', () => {resolve();}, {once: true})));
                img.src = `png/${type}-${i}.png`;
            }
        }
    }
    return Promise.all(promises);
},
resultView: undefined,
startWator() {
    this._init_wator(15, 15);
    let data = this._getCellData();
    resultView = new Uint8Array(Module.HEAPU8.buffer, data, 15 * 15 * 3);
    this.render();
},
async onRuntimeInitialized() {
    await this.initImages();
    this.startWator();
},
CELL_SIZE: 40,
COLOR_NAMES: ['red', 'green', 'blue', 'yellow', 'cyan', 'magenta', 'gray', 'orange'],
render() {
    /** @type CanvasRenderingContext2D */
    let ctx = this.canvas.getContext('2d');
    ctx.fillStyle = 'black';
    ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
    ctx.imageSmoothingEnabled = true;
    ctx.imageSmoothingQuality = "high";
    let tempCanvas = document.createElement("canvas");
    tempCanvas.width = this.CELL_SIZE;
    tempCanvas.height = this.CELL_SIZE;
    //let tempCanvas = document.getElementById("canvasTemp");
    let tempCanvasCtx = tempCanvas.getContext('2d');
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
                // TODO - lighten shark color or something?
                tempCanvasCtx.fillStyle = this.COLOR_NAMES[color];
                // this is the default value
                tempCanvasCtx.globalCompositeOperation = "source-over";
                tempCanvasCtx.fillRect(0, 0, this.CELL_SIZE, this.CELL_SIZE);
                tempCanvasCtx.globalCompositeOperation = "destination-in";
                tempCanvasCtx.drawImage(this.IMAGES[bitmap], 0, 0, 20, 20, 0, 0, this.CELL_SIZE, this.CELL_SIZE);
                ctx.drawImage(tempCanvas, 0, 0, this.CELL_SIZE, this.CELL_SIZE, x * this.CELL_SIZE, y * this.CELL_SIZE, this.CELL_SIZE, this.CELL_SIZE);
            }
        }
    }
},
tick() {
    // does the tick
    this._draw_wator();
    this.render();
},
};

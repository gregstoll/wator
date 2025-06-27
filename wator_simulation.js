var canvasElement = document.getElementById('canvas');

SPEEDS = [1000, 750, 500, 250, 0];
CELL_SIZES = [10, 20, 30, 40, 50, 60, 70];
ADAPTIVE_SPEED_THRESHOLDS = [200, 1000, 3000];
COLOR_NAMES = ['#2fef10', '#00ffdd', '#7b68ee', '#ffee15', '#ff796c', '#0000ff', '#aa23ff', 'rainbow'];

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
interval: undefined,
cellCounts: undefined,
cellSize: CELL_SIZES[3],
nextCellSize: CELL_SIZES[3],
speed: SPEEDS[2],
generation: 0,
adaptiveSpeed: false,
fishGradients: [],
sharkGradients: [],
tempCanvas: undefined,
initGradients() {
    // name indicates where head is
    let tempCanvasCtx = this.tempCanvas.getContext('2d');
    let topRainbowFishGradient = tempCanvasCtx.createLinearGradient(0, 0, 0, this.cellSize);
    this.addRainbowColorStops(topRainbowFishGradient, true);
    let bottomRainbowFishGradient = tempCanvasCtx.createLinearGradient(0, this.cellSize, 0, 0);
    this.addRainbowColorStops(bottomRainbowFishGradient, true);
    let leftRainbowFishGradient = tempCanvasCtx.createLinearGradient(0, 0, this.cellSize, 0);
    this.addRainbowColorStops(leftRainbowFishGradient, true);
    let rightRainbowFishGradient = tempCanvasCtx.createLinearGradient(this.cellSize, 0, 0, 0);
    this.addRainbowColorStops(rightRainbowFishGradient, true);
    this.fishGradients = [topRainbowFishGradient, rightRainbowFishGradient, bottomRainbowFishGradient, leftRainbowFishGradient];

    let topRainbowSharkGradient = tempCanvasCtx.createLinearGradient(0, 0, 0, this.cellSize);
    this.addRainbowColorStops(topRainbowSharkGradient, false);
    let bottomRainbowSharkGradient = tempCanvasCtx.createLinearGradient(0, this.cellSize, 0, 0);
    this.addRainbowColorStops(bottomRainbowSharkGradient, false);
    let leftRainbowSharkGradient = tempCanvasCtx.createLinearGradient(0, 0, this.cellSize, 0);
    this.addRainbowColorStops(leftRainbowSharkGradient, false);
    let rightRainbowSharkGradient = tempCanvasCtx.createLinearGradient(this.cellSize, 0, 0, 0);
    this.addRainbowColorStops(rightRainbowSharkGradient, false);
    this.sharkGradients = [topRainbowSharkGradient, rightRainbowSharkGradient, bottomRainbowSharkGradient, leftRainbowSharkGradient];
},
startWator(keepTicking) {
    this.canvas.height = this.canvas.offsetHeight;
    this.canvas.width = this.canvas.offsetWidth;
    this.cellSize = this.nextCellSize;
    this.generation = 0;
    // TODO - could reseed some random colors or something
    this.cellCounts = [Math.floor(this.canvas.width / this.cellSize), Math.floor(this.canvas.height / this.cellSize)];
    this._init_wator(this.cellCounts[0], this.cellCounts[1]);
    document.getElementById("size").innerHTML = `${this.cellCounts[0]}x${this.cellCounts[1]}`;
    this.tempCanvas = document.createElement("canvas");
    this.tempCanvas.width = this.cellSize;
    this.tempCanvas.height = this.cellSize;
    this.initGradients();
    let data = this._getCellData();
    resultView = new Uint8Array(Module.HEAPU8.buffer, data, this.cellCounts[0] * this.cellCounts[1] * 3);
    this.render();
    if (this.interval) {
        clearTimeout(this.interval);
    }
    document.getElementById("pause").value = "pause";
    this.interval = setInterval(() => {
        this.tick();
    }, this.speed);
},
pause() {
    let button = document.getElementById("pause");
    if (this.interval) {
        clearTimeout(this.interval);
        this.interval = undefined;
        button.value = "play";
    } else {
        this.interval = setInterval(() => {
            this.tick();
        }, this.getAdaptiveSpeed());
        button.value = "pause";
    }
},
updateSpeed() {
    let val = document.getElementById("speed").value;
    this.speed = SPEEDS[val];
    if (this.interval) {
        clearTimeout(this.interval);
        this.interval = setInterval(() => {
            this.tick();
        }, this.getAdaptiveSpeed());
    }
},
updateNextCellSize() {
    let val = document.getElementById("cellSize").value;
    this.nextCellSize = CELL_SIZES[val];
},
getAdaptiveSpeed() {
    let factor = 1;
    if (this.adaptiveSpeed) {
        for (let entry in ADAPTIVE_SPEED_THRESHOLDS) {
            if (this.generation >= entry) {
                factor++;
            } else {
                break;
            }
        }
    }
    return this.speed / factor;
},
checkAdaptiveSpeed(force) {
    if (!this.interval) {
        return;
    }
    let changed = ADAPTIVE_SPEED_THRESHOLDS.includes(this.generation);
    let newSpeed = this.getAdaptiveSpeed();
    if (changed || force) {
        clearTimeout(this.interval);
        this.interval = setInterval(() => {
            this.tick();
        }, newSpeed);
    }
},
updateAdaptiveSpeed() {
    this.adaptiveSpeed = document.getElementById("adaptiveSpeed").checked;
    this.checkAdaptiveSpeed(true);
},
initEvents() {
    document.getElementById("restart").addEventListener("click", () => {Module.startWator(true);});
    document.getElementById("pause").addEventListener("click", () => {Module.pause();});
    document.getElementById("speed").addEventListener("change", () => {Module.updateSpeed();});
    // in case the page got reloaded and the sliders were not at their default value
    this.updateSpeed();
    document.getElementById("cellSize").addEventListener("change", () => {Module.updateNextCellSize();});
    this.updateNextCellSize();
    document.getElementById("adaptiveSpeed").addEventListener("change", () => {Module.updateAdaptiveSpeed();});
    this.updateAdaptiveSpeed();
    document.getElementById("canvas").addEventListener("click", () => {
        // This should happen automatically but it doesn't work on older
        // versions of Safari on iOS 17.0-18.2 :-( see
        // https://bugs.webkit.org/show_bug.cgi?id=267688
        document.getElementById("optionsPanel").hidePopover();
    });
},
async onRuntimeInitialized() {
    await this.initImages();
    this.initEvents();
    this.startWator(true);
},
addRainbowColorStops(gradient, isFish) {
    if (isFish) {
        gradient.addColorStop(0, "red");
        gradient.addColorStop(0.35, "red");
        gradient.addColorStop(0.43, "gold");
        gradient.addColorStop(0.51, "yellow");
        gradient.addColorStop(0.58, "green");
        gradient.addColorStop(0.65, "blue");
        gradient.addColorStop(0.73, "purple");
        gradient.addColorStop(1, "purple");
    } else {
        // sigh
        /*gradient.addColorStop(0.3, "lch(from red calc(l+30) c h)");
        gradient.addColorStop(0.4, "lch(from gold calc(l+30) c h)");
        gradient.addColorStop(0.5, "lch(from yellow calc(l+30) c h)");
        gradient.addColorStop(0.6, "lch(from green calc(l+30) c h)");
        gradient.addColorStop(0.7, "lch(from blue calc(l+30) c h)");
        gradient.addColorStop(0.8, "lch(from purple calc(l+30) c h)");*/
        gradient.addColorStop(0, "crimson");
        gradient.addColorStop(0.3, "crimson");
        gradient.addColorStop(0.4, "gold");
        gradient.addColorStop(0.5, "yellow");
        gradient.addColorStop(0.6, "lime");
        gradient.addColorStop(0.7, "aqua");
        gradient.addColorStop(0.8, "fuchsia");
        gradient.addColorStop(1, "fuchsia");
    }
},
render() {
    /** @type CanvasRenderingContext2D */
    let ctx = this.canvas.getContext('2d');
    ctx.fillStyle = 'black';
    ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
    ctx.imageSmoothingEnabled = true;
    ctx.imageSmoothingQuality = "high";
    //let tempCanvas = document.getElementById("canvasTemp");
    let tempCanvasCtx = this.tempCanvas.getContext('2d');
    /*console.log(`img dimensions: ${this.IMAGES[0].width}x${this.IMAGES[0].height}`);
    console.log(`img natural dimensions: ${this.IMAGES[0].naturalWidth}x${this.IMAGES[0].naturalHeight}`);
    console.log(`tempcanvas dimensions: ${tempCanvas.width}x${tempCanvas.height}`);
    console.log(`tempcanvas offset: ${tempCanvas.offsetWidth}x${tempCanvas.offsetHeight}`);
    console.log(`canvas dimensions: ${this.canvas.width}x${this.canvas.height}`);
    console.log(`canvas offset: ${this.canvas.offsetWidth}x${this.canvas.offsetHeight}`);*/
    let haveFish = false;
    let haveShark = false;

    for (let y = 0; y < this.cellCounts[1]; y++) {
        /*ctx.strokeStyle = 'white';
        ctx.beginPath();
        ctx.moveTo(0, this.cellSize*y);
        ctx.lineTo(this.canvas.width, this.cellSize*y);
        ctx.stroke();
        ctx.moveTo(this.cellSize*y, 0);
        ctx.lineTo(this.cellSize*y, this.canvas.height);
        ctx.stroke();*/
        for (let x = 0; x < this.cellCounts[0]; x++) {
            let index = (y * this.cellCounts[0] + x) * 3;
            let alive = resultView[index];
            let color = resultView[index+1];
            let bitmap = resultView[index+2];
            if (alive) {
                // https://stackoverflow.com/questions/61337596/html5-canvas-filling-transparent-image-with-color-and-drawing-on-top
                let isFish = bitmap < 8;
                //let isVertical = bitmap % 2 === 0;
                let orientation = bitmap % 4;
                if (isFish) { haveFish = true; } else { haveShark = true; }
                // lighten sharks a bit
                let colorName = COLOR_NAMES[color];
                if (colorName === "rainbow") {
                    tempCanvasCtx.fillStyle = isFish ? this.fishGradients[orientation] : this.sharkGradients[orientation];
                } else {
                    tempCanvasCtx.fillStyle = isFish ? colorName : `lch(from ${colorName} calc(l + 30) c h)`;
                }
                // this is the default value
                tempCanvasCtx.globalCompositeOperation = "source-over";
                tempCanvasCtx.fillRect(0, 0, this.cellSize, this.cellSize);
                tempCanvasCtx.globalCompositeOperation = "destination-in";
                tempCanvasCtx.drawImage(this.IMAGES[bitmap], 0, 0, 20, 20, 0, 0, this.cellSize, this.cellSize);
                ctx.drawImage(this.tempCanvas, 0, 0, this.cellSize, this.cellSize, x * this.cellSize, y * this.cellSize, this.cellSize, this.cellSize);
            }
        }
    }
    this.generation++;
    document.getElementById("generation").innerHTML = this.generation;
    this.checkAdaptiveSpeed(false);
    if (!(haveFish && haveShark)) {
        // TODO - we could set a flag here saying that when play is pressed,
        // just go ahead and keep running until the whole world is empty.
        this.pause();
    }
},
tick() {
    // does the tick
    this._draw_wator();
    this.render();
},
};

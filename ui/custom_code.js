// Check if SharedArrayBuffer is enabled
if (typeof SharedArrayBuffer !== 'function') {
    alert("SharedArrayBuffer is not supported on this browser! Please go to Chrome or another browser that supports SharedArrayBuffer");
}

var Module = {}


var addr = 0x2c84f90;
var imagedata1 = null, u8, start = performance.now();

function getaddr() {
    return Module.ccall('image_data_loc', 'number', [null], [null]);
}

var times = []

function meas() {
    var ret = performance.now() - start;
    start = performance.now();
    times.push(ret);
}

function render() {
    start = performance.now();
    Module._do_render();
    meas();
    addr = Module._image_data_loc();
    const t = Module.HEAPU8.slice(addr, addr + 960 * 540 * 4);
    u8 = new Uint8ClampedArray(t);
    imagedata1 = new ImageData(u8, 960, 540);
    const ctx = document.getElementById('canvas').getContext('2d');
    ctx.putImageData(imagedata1, 0, 0);
}

function render_light_view() {
    clearInterval(job_handler);

    Module._render_light_view();
    addr = Module._image_data_loc();
    const t = Module.HEAPU8.slice(addr, addr + 960 * 540 * 4);
    meas();
    u8 = new Uint8ClampedArray(t);
    imagedata1 = new ImageData(u8, 960, 540);
    meas();
    const ctx = document.getElementById('canvas').getContext('2d');
    ctx.putImageData(imagedata1, 0, 0);
    meas();
}

var job_handler;

function set_render_interval(interv = 75) {
    clearInterval(job_handler);
    job_handler = setInterval(() => {
        render();
    }, interv);
}

Module.onRuntimeInitialized = () => {
    set_render_interval(75)
}

document.addEventListener('keydown', (e) => {
    if (e.key === "Q") {
        for (const i of intervals) clearInterval(i);
    }
})
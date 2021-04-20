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
    const t = Module.HEAPU8.slice(addr, addr + 960*540*4);
    u8 = new Uint8ClampedArray(t);
    imagedata1 = new ImageData(u8, 960, 540);
    const ctx = document.getElementById('canvas').getContext('2d');
    ctx.putImageData(imagedata1, 0, 0);
}

function render_light_view() {
    for (const i of intervals) clearInterval(i);

    Module._render_light_view();
    addr = Module._image_data_loc();
    const t = Module.HEAPU8.slice(addr, addr + 960 * 540 * 4);
    meas();
    u8 = new Uint8ClampedArray(t);
    imagedata1 = new ImageData(u8, 1000, 800);
    meas();
    const ctx = document.getElementById('canvas').getContext('2d');
    ctx.putImageData(imagedata1, 0, 0);
    meas();
}
var intervals = [];
Module.onRuntimeInitialized = () => {
    intervals[1] = setInterval(() => {
        render();
    }, 150);
}

document.addEventListener('keydown', (e) => {
    if(e.key==="Q") {
        for (const i of intervals) clearInterval(i);
    }
})
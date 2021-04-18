var blob, memptr, mybuffer, url;
var Module = {
    onRuntimeInitialized: function () {
        memptr = Module._image();
        mybuffer = Module.HEAP8.slice(memptr, memptr + 1000000*8)
        blob = new Blob([mybuffer.buffer])
        url = URL.createObjectURL(blob)
        console.log(url);
        // window.open(url);
    }
}

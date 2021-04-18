emcc main.cpp -o index.js \
    -s EXPORTED_RUNTIME_METHODS='['ccall', 'getValue', 'UTF8ToString', 'stringToUTF8', 'stringToUTF32']' \
    -s WASM=1 -s NO_EXIT_RUNTIME=1 \
    -s EXPORTED_FUNCTIONS="['_add', '_divide', '_main', '_pr', '_malloc', '_image']"

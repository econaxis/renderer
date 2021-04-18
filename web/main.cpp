#include <emscripten/emscripten.h>
#include <stdlib.h>

#include <stdio.h>
#include <iostream>
#include <memory>

char* s = "test string";

int main() {
    srand(324);
    int64_t *a = new int64_t[50000];

    for(int i = 0; i < 50000; i++) {
        a[i] = i * 2;
    }
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    int add(int a, int b) {
        std::cout<<(void*)s<<std::endl;
        return a+b;
    }

    EMSCRIPTEN_KEEPALIVE
    int divide (int a) {
        std::cout<<(void*)s<<std::endl;
        return a/2;
    }

    EMSCRIPTEN_KEEPALIVE
    void pr(void* a) {
        std::cout<<(char*)a<<std::endl;
    }

    EMSCRIPTEN_KEEPALIVE
    void* image() {
        int* img = new int[100];
        for(int i = 0; i < 100; i++) {
            long long li = i + 12583;
            img[i] = (li * (li+3)) % (long long) 500;
            std::cout<<img[i]<<" ";
        }
        std::cout<<std::endl;
        return (void*) img;
    };
}

#pragma once
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cmath>

static void filter_grayscale(uint8_t* px, int w, int h, int ch) {
    for (int i = 0; i < w * h; i++) {
        uint8_t* p = px + i * ch;
        uint8_t g = (uint8_t)(0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]);
        p[0] = p[1] = p[2] = g;
    }
}

static void filter_threshold(uint8_t* px, int w, int h, int ch) {
    for (int i = 0; i < w * h; i++) {
        uint8_t* p = px + i * ch;
        uint8_t g = (uint8_t)(0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]);
        uint8_t v = g > 128 ? 255 : 0;
        p[0] = p[1] = p[2] = v;
    }
}

static void convolve3x3(uint8_t* px, int w, int h, int ch, const float k[9]) {
    auto* buf = new uint8_t[w * h * ch];
    memcpy(buf, px, w * h * ch);

    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            for (int c = 0; c < ch; c++) {
                float sum = 0;
                for (int ky = -1; ky <= 1; ky++)
                    for (int kx = -1; kx <= 1; kx++)
                        sum += k[(ky + 1) * 3 + (kx + 1)] * buf[((y + ky) * w + (x + kx)) * ch + c];
                px[(y * w + x) * ch + c] = (uint8_t)std::clamp((int)sum, 0, 255);
            }
        }
    }
    delete[] buf;
}

static void filter_sharpen(uint8_t* px, int w, int h, int ch) {
    const float k[9] = {
         0, -1,  0,
        -1,  5, -1,
         0, -1,  0
    };
    convolve3x3(px, w, h, ch, k);
}

static void filter_edge(uint8_t* px, int w, int h, int ch) {
    const float k[9] = {
        -1, -1, -1,
        -1,  8, -1,
        -1, -1, -1
    };
    convolve3x3(px, w, h, ch, k);
}

static void filter_blur(uint8_t* px, int w, int h, int ch) {
    const float k[9] = {
        1/9.f, 1/9.f, 1/9.f,
        1/9.f, 1/9.f, 1/9.f,
        1/9.f, 1/9.f, 1/9.f
    };
    convolve3x3(px, w, h, ch, k);
}

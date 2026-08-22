// Minimal smoke test: warps a procedurally drawn "source" and writes frames
// as PPM files you can eyeball. Replace makeTestSource() with your baked logo.
//   c++ -O2 -std=c++11 example_main.cpp -o warp && ./warp
#include "text_distortion.hpp"
#include <cstdio>
#include <vector>

static void makeTestSource(std::vector<uint8_t>& img, int W, int H) {
    // paper #f4f1ea, a few fat "ink" bars standing in for text
    for (int i = 0; i < W * H; i++) {
        img[i*4] = 0xf4; img[i*4+1] = 0xf1; img[i*4+2] = 0xea; img[i*4+3] = 255;
    }
    auto bar = [&](int x, int y, int w, int h) {
        for (int yy = y; yy < y + h; yy++)
            for (int xx = x; xx < x + w; xx++) {
                size_t i = ((size_t)yy * W + xx) * 4;
                img[i] = 0x17; img[i+1] = 0x14; img[i+2] = 0x0f;
            }
    };
    bar(W/6, H/3, W*2/3, H/14);
    bar(W/6, H/2, W/2,   H/14);
}

static void writePPM(const char* path, const uint8_t* img, int W, int H) {
    FILE* f = fopen(path, "wb");
    fprintf(f, "P6\n%d %d\n255\n", W, H);
    for (int i = 0; i < W * H; i++) fwrite(img + i * 4, 1, 3, f);
    fclose(f);
}

int main() {
    const int W = 900, H = 575;
    std::vector<uint8_t> src(W * H * 4), dst(W * H * 4);
    makeTestSource(src, W, H);

    td::Warp warp(td::Params(), 7);
    warp.addBlob(110.0f, 55.0f, 0.3f, 0.7f, 1, 1.2f);
    warp.addBlob(-90.0f, 40.0f, 0.8f, 0.2f, 2, 4.0f);
    warp.addSwirl(140.0f, 0.3f, 0.45f, 1, 2.6f);

    for (int f = 0; f < 8; f++) {                 // one frame per t-unit
        warp.render(src.data(), dst.data(), W, H, f * 1.0f);
        char name[32]; snprintf(name, sizeof name, "frame_%02d.ppm", f);
        writePPM(name, dst.data(), W, H);
    }
    printf("wrote frame_00..07.ppm\n");
    return 0;
}

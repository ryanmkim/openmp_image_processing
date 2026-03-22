#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "filters.h"

#include <omp.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

constexpr int TILE = 256;

enum Filter { NONE, GRAYSCALE, THRESHOLD, SHARPEN, EDGE, BLUR };

Filter parse_filter(const char* s) {
    if (!strcmp(s, "grayscale")) return GRAYSCALE;
    if (!strcmp(s, "threshold")) return THRESHOLD;
    if (!strcmp(s, "sharpen"))   return SHARPEN;
    if (!strcmp(s, "edge"))      return EDGE;
    if (!strcmp(s, "blur"))      return BLUR;
    if (!strcmp(s, "none"))      return NONE;
    fprintf(stderr, "unknown filter: %s\n", s);
    exit(1);
}

void apply(Filter f, uint8_t* px, int w, int h, int ch) {
    switch (f) {
        case GRAYSCALE: filter_grayscale(px, w, h, ch); break;
        case THRESHOLD: filter_threshold(px, w, h, ch); break;
        case SHARPEN:   filter_sharpen(px, w, h, ch);   break;
        case EDGE:      filter_edge(px, w, h, ch);      break;
        case BLUR:      filter_blur(px, w, h, ch);      break;
        case NONE:      break;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("usage: %s <input.png> <filter> [outdir]\n", argv[0]);
        return 1;
    }

    const char* path = argv[1];
    Filter filt = parse_filter(argv[2]);
    std::string outdir = argc > 3 ? argv[3] : "tiles_out";

    int w, h, ch;
    uint8_t* img = stbi_load(path, &w, &h, &ch, 3);
    if (!img) { fprintf(stderr, "can't load %s\n", path); return 1; }
    ch = 3;

    int cols = (w + TILE - 1) / TILE;
    int rows = (h + TILE - 1) / TILE;
    int ntiles = rows * cols;

    printf("loaded %s (%dx%d), %d tiles\n", path, w, h, ntiles);
    fs::create_directories(outdir);

    int done = 0;
    double t0 = omp_get_wtime();

    #pragma omp parallel for schedule(dynamic) shared(done)
    for (int t = 0; t < ntiles; t++) {
        int r = t / cols, c = t % cols;
        int x0 = c * TILE, y0 = r * TILE;
        int tw = std::min(TILE, w - x0);
        int th = std::min(TILE, h - y0);

        auto* tile = new uint8_t[tw * th * ch];
        for (int y = 0; y < th; y++)
            memcpy(tile + y * tw * ch, img + ((y0 + y) * w + x0) * ch, tw * ch);

        apply(filt, tile, tw, th, ch);

        char fname[256];
        snprintf(fname, sizeof fname, "%s/tile_%03d_%03d.png", outdir.c_str(), r, c);
        stbi_write_png(fname, tw, th, ch, tile, tw * ch);
        delete[] tile;

        #pragma omp atomic
        done++;
        if (omp_get_thread_num() == 0 && done % 10 == 0)
            printf("\r  %d / %d", done, ntiles);
    }

    printf("\r  %d / %d tiles, %.3fs (%d threads)\n",
           ntiles, ntiles, omp_get_wtime() - t0, omp_get_max_threads());

    stbi_image_free(img);
}

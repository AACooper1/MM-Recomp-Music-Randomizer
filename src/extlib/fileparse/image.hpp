#ifndef IMAGE_H
#define IMAGE_H

#include "lib/CImg.h"

#include <string>
#include <filesystem>

#include "common.hpp"

using namespace cimg_library;
namespace fs = std::filesystem;

struct cImgPixel {
    int x;
    int y;
    int r;
    int g;
    int b;
    int a;

    cImgPixel(int x, int y, int r, int g, int b, int a) : x(x), y(y), r(r), g(g), b(b), a(a) {}
    cImgPixel() : x(0), y(0), r(0), g(0), b(0), a(0) {}
};

class Image {
    public:
        Image(fs::path path) : path(path) { timestamp =  std::chrono::duration_cast<std::chrono::seconds>(fs::last_write_time(path).time_since_epoch()).count(); }
        Image() {}

        fs::path path;
        long long timestamp;
        void parse();
        u64 rgba32[64][32];
        std::vector<char> data;

        int databaseIndex = 0;
    private:
        void pixels_to_rgba32();
        cImgPixel pixels[64][64];
};

#endif
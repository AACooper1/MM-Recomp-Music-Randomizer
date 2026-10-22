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

void parse_image(fs::path path);

#endif
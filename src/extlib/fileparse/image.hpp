#ifndef IMAGE_H
#define IMAGE_H

#include "lib/CImg.h"

#include <string>
#include <filesystem>

#include "common.hpp"

using namespace cimg_library;
namespace fs = std::filesystem;

void parse_image(fs::path path)
{
    const char* p = path.string().c_str();
    CImg<unsigned char> image(p);

    int target_width = std::min(image.width() * 64 / image.height(), 64);
    int target_height = std::min(image.width() * 64 / image.height(), 64);
    
    image.resize(target_width, target_height, -100, -100, 5);
}

#endif
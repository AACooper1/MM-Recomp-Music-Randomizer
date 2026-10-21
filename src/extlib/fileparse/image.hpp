#ifndef IMAGE_H
#define IMAGE_H

#include "lib/CImg.h"

#include <string>
#include <filesystem>

#include "common.hpp"

using namespace cimg_library;
namespace fs = std::filesystem;

void parse_image(fs::path path);

#endif
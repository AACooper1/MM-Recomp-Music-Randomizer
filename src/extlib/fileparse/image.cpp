#ifndef cimg_display
#define cimg_display 0
#endif
#include "lib/CImg.h"
#include "image.hpp"

void Image::parse()
{
    cimg::exception_mode(1);

    CImg<unsigned char> image;
    if (!path.empty())
    {
        std::string p_str = path.string();
        const char* p = p_str.c_str();
        image.load(p);
    }
    else
    {
        image.load(raw_file->data());
    }

    int target_width = std::min(image.width() * 64 / image.height(), 64);
    int target_height = std::min(image.height() * 64 / image.width(), 64);
    
    image.resize(target_width, target_height, -100, -100, 5);

    CImg<unsigned char> canvas(64, 64, 1, image.spectrum(), 0);
    unsigned char color[] = { 0, 0, 0 };
    canvas.draw_rectangle(0, 0, 63, 63, color);

    int offset_x = (64 - target_width) / 2;
    int offset_y = (64 - target_height) / 2;

    canvas.draw_image(offset_x, offset_y, image);

    std::string fname_str = path.parent_path().filename().string();
    const char* fname = fname_str.c_str();

    for (int y = 0; y < canvas.height(); y++)
    {
        for (int x = 0; x < canvas.width(); x++)
        {
            int r = canvas(x, y, 0);                                // Always present
            int g = canvas.spectrum() >= 3 ? canvas(x, y, 1) : r;   // 1 or 2 = greyscale
            int b = canvas.spectrum() >= 3 ? canvas(x, y, 2) : r;   // 1 or 2 = greyscale
            int a = canvas.spectrum() == 2 || canvas.spectrum() == 4 ? canvas(x, y, 3) : 0xFF;  // 2 = greyscale with alpha, 4 = rgba

            pixels[y][x] = cImgPixel(x, y, r, g, b, a);
        }
    }

    pixels_to_rgba32();

    return;
}

void Image::pixels_to_rgba32()
{
    for (int y = 0; y < 64; y++)
    {
        for (int x = 0; x < 64; x+=2)
        {
            u32 px1 = pixels[y][x].r << 24 | pixels[y][x].g << 16 | pixels[y][x].b << 8 | pixels[y][x].a;
            u32 px2 = pixels[y][x + 1].r << 24 | pixels[y][x + 1].g << 16 | pixels[y][x + 1].b << 8 | pixels[y][x + 1].a;
            u64 px = (long long)px1 << 32 | px2;
            rgba32[y][x / 2] = px;
            
            for (int i = 7; i >= 0; i--)
            {
                data.push_back(px >> i * 8 & 0xFF);
            }
        }
    }
}
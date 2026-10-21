#include "image.hpp"

void parse_image(fs::path path)
{
    cimg::exception_mode(1);
    std::string p_str = path.string();
    const char* p = p_str.c_str();
    CImg<unsigned char> image(p);

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

    CImgDisplay disp(canvas, fname);
    disp.move(100, 100);
    disp.wait(20);
    disp.close();

    
}
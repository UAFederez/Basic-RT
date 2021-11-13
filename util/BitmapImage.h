#ifndef UTIL_BITMAP_IMAGE_H
#define UTIL_BITMAP_IMAGE_H

#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <cstdio>
#include <memory>
#include <fstream>

#pragma pack(1)
struct BitmapInfoHeader 
{
    uint16_t id;
    uint32_t size;
    uint32_t reserved;
    uint32_t offset;
};

struct BitmapDIBHeader
{
    uint32_t header_sz;
    uint32_t width;
    uint32_t height;
    uint16_t num_planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t bmp_size;
    uint32_t width_print;
    uint32_t height_print;
    uint32_t num_palette_colors;
    uint32_t num_important_colors;
};
#pragma pack()

void write_bmp_to_file (const char*    file_name,
                        const uint8_t* pixels,
                        const uint32_t width,
                        const uint32_t height,
                        const uint32_t bytes_per_pixel);

std::unique_ptr<uint8_t> read_from_bmp_file(const char* file_name,
                                            uint32_t*   image_width,
                                            uint32_t*   image_height,
                                            uint32_t*   bytes_per_pixel);
#endif

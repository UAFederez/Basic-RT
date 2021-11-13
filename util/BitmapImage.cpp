#include "BitmapImage.h"

void write_bmp_to_file(const char*    file_name,
                       const uint8_t* pixels,
                       const uint32_t width,
                       const uint32_t height,
                       const uint32_t bytes_per_pixel)
{
    const uint32_t padding     = (4 - ((width * bytes_per_pixel) % 4)) % 4;
    const uint32_t total_bytes = (width * height * bytes_per_pixel) + (padding * height);

    // TODO: Error checking
    FILE* output_file = fopen(file_name, "wb");

    BitmapDIBHeader bmpDibHeader;
    bmpDibHeader.header_sz = sizeof(bmpDibHeader);
    bmpDibHeader.width = width;
    bmpDibHeader.height = height;
    bmpDibHeader.num_planes = 1;
    bmpDibHeader.bits_per_pixel = 8 * bytes_per_pixel;
    bmpDibHeader.compression = 0;
    bmpDibHeader.bmp_size = total_bytes;
    bmpDibHeader.width_print = 2836;
    bmpDibHeader.height_print = 2836;
    bmpDibHeader.num_palette_colors = 0;
    bmpDibHeader.num_important_colors = 0;

    BitmapInfoHeader bmpInfoHeader;
    bmpInfoHeader.id       = 0x4d42;   // 'B', 'M'
    bmpInfoHeader.reserved = 0;
    bmpInfoHeader.offset   = sizeof(BitmapInfoHeader) + sizeof(BitmapDIBHeader);
    bmpInfoHeader.size     = total_bytes + bmpInfoHeader.offset;

    fwrite((void*)&bmpInfoHeader, sizeof(bmpInfoHeader), 1, output_file);
    fwrite((void*)&bmpDibHeader, sizeof(bmpDibHeader), 1, output_file);

    const uint32_t empty = 0;
    if(padding == 0)
    {
        for(uint32_t row = 0; row < height; row++)
            fwrite(&pixels[row * width * bytes_per_pixel], width * bytes_per_pixel, 1, output_file);
    }
    else
    {
        for(uint32_t row = 0; row < height; row++)
        {
            fwrite(&pixels[row * width * bytes_per_pixel], width * bytes_per_pixel, 1, output_file);
            fwrite((void*)&empty, padding, 1, output_file);
        }
    }

    fclose(output_file);
}

std::unique_ptr<uint8_t> read_from_bmp_file(const char* file_name,
                                            uint32_t* image_width,
                                            uint32_t* image_height,
                                            uint32_t* bytes_per_pixel)
{
    // TODO: Error checking
    std::ifstream input_file(file_name, std::ios::binary | std::ios::in);

    if(!input_file)
        return nullptr;

    BitmapDIBHeader bmpDibHeader   = {};
    BitmapInfoHeader bmpInfoHeader = {};

    input_file.read((char*)&bmpInfoHeader, sizeof(BitmapInfoHeader));
    input_file.read((char*)&bmpDibHeader , sizeof(BitmapDIBHeader));
    
    if(image_width)     *image_width     = bmpDibHeader.width;
    if(image_height)    *image_height    = bmpDibHeader.height;
    if(bytes_per_pixel) *bytes_per_pixel = bmpDibHeader.bits_per_pixel / 8;

    const uint32_t width       = bmpDibHeader.width;
    const uint32_t height      = bmpDibHeader.height;
    const uint32_t bpp         = *bytes_per_pixel;
    const uint32_t total_bytes = width * height * bpp;

    const uint32_t padding     = (4 - ( width * bpp ) % 4) % 4;
    const uint32_t pixels_w    = width * bpp;
    const uint32_t bytes_w     = pixels_w + padding;

    uint8_t* pixels = new uint8_t[ total_bytes ];

    uint32_t pad_bytes = 0;

    for(uint32_t y = 0; y < height; y++)
    {
        input_file.read((char*)(pixels + (y * bytes_w)), pixels_w);
        input_file.read((char*)&pad_bytes, padding);
    }

    return std::unique_ptr<uint8_t>(pixels);
}

#ifndef UTIL_BITMAP_IMAGE_H
#define UTIL_BITMAP_IMAGE_H

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

void write_bmp_to_file(const char*    file_name,
                       const uint8_t* pixels,
                       const uint32_t width,
                       const uint32_t height,
                       const uint32_t bytes_per_pixel)
{
    const uint32_t padding     = (4 - ((width * bytes_per_pixel) % 4)) % 4;
    const uint32_t total_bytes = (width * height * bytes_per_pixel) + (padding * height);

    FILE* output_file = fopen(file_name, "wb");

    BitmapDIBHeader bmpDibHeader;
    bmpDibHeader.header_sz = sizeof(bmpDibHeader);
    bmpDibHeader.width = width;
    bmpDibHeader.height = height;
    bmpDibHeader.num_planes = 1;
    bmpDibHeader.bits_per_pixel = 8 * bytes_per_pixel;
    bmpDibHeader.compression = 0;
    bmpDibHeader.bmp_size = total_bytes;
    bmpDibHeader.width_print = 2835;
    bmpDibHeader.height_print = 2835;
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

#endif

#ifndef IMAGEDATA_H
#define IMAGEDATA_H

#include <cstdint>
#include <memory>
#include <string>
#include "../fs/file.h"

namespace tex
{
struct ImageData
{
    // origin in the bottom-left corner
    enum class PixelType
    {
        pt_rgb,
        pt_rgba,
        pt_none
    };

    uint32_t                   width  = 0;
    uint32_t                   height = 0;
    PixelType                  type   = PixelType::pt_none;
    std::unique_ptr<uint8_t[]> data;
};

bool ReadBMP(BaseFile const & file, ImageData & image);
bool ReadTGA(BaseFile const & file, ImageData & image);

bool ReadBMP(std::string const & file_name, ImageData & image);
bool WriteTGA(std::string file_name, ImageData const & image);
}   // namespace tex
#endif   // IMAGEDATA_H

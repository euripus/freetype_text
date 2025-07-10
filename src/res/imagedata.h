#ifndef IMAGEDATA_H
#define IMAGEDATA_H

#include <cstdint>
#include <memory>
#include <string>

namespace tex
{
struct ImageData
{
    // origin is the lower-left corner
    enum class PixelType
    {
        pt_rgb,
        pt_rgba,
        pt_compressed,
        pt_float,
        pt_none
    };

    uint32_t                   width     = 0;
    uint32_t                   height    = 0;
    uint32_t                   depth     = 0;
    uint32_t                   data_size = 0;
    PixelType                  type      = PixelType::pt_none;
    std::unique_ptr<uint8_t[]> data;
};

bool ReadBMP(BaseFile const & file, ImageData & id);
bool ReadTGA(BaseFile const & file, ImageData & id);

bool WriteTGA(BaseFile & file, ImageData const & id);
}   // namespace tex
#endif   // IMAGEDATA_H

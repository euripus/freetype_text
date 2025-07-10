#include "imagedata.h"
#include <cstring>
#include <vector>
#include <fstream>
#include <iostream>

#pragma pack(push, 1)

struct BITMAPFILEHEADER
{
    uint16_t bfType;   // bmp file signature
    uint32_t bfSize;   // file size
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};

struct BITMAPINFO12   // CORE version
{
    uint32_t biSize;
    uint16_t biWidth;
    uint16_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
};

struct BITMAPINFO   // Standart version
{
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};

struct TGAHEADER
{
    uint8_t  idlength;
    uint8_t  colourmaptype;
    uint8_t  datatypecode;
    uint16_t colourmaporigin;
    uint16_t colourmaplength;
    uint8_t  colourmapdepth;
    uint16_t x_origin;
    uint16_t y_origin;
    uint16_t width;
    uint16_t height;
    uint8_t  bitsperpixel;
    uint8_t  imagedescriptor;
};

#pragma pack(pop)

namespace tex
{
//==============================================================================
//         Read BMP section
//==============================================================================
bool ReadBMP(std::string const & file_name, ImageData & id)
{
    bool   res         = false;
    bool   compressed  = false;
    bool   flip        = false;
    size_t file_length = 0;

    id.width  = 0;
    id.height = 0;
    id.type   = ImageData::PixelType::pt_none;
    if(id.data)
        id.data.reset(nullptr);

    std::ifstream        ifile(file_name, std::ios::binary);
    std::vector<uint8_t> file;

    if(ifile.is_open())
    {
        ifile.seekg(0, std::ios_base::end);
        auto length = ifile.tellg();
        ifile.seekg(0, std::ios_base::beg);

        file.resize(static_cast<size_t>(length));

        ifile.read(reinterpret_cast<char *>(file.data()), length);

        auto success = !ifile.fail() && length == ifile.gcount();
        ifile.close();

        if(!success)
            return false;
    }
    else
        return false;

    file_length = file.size();
    if(file_length == 0)
        return res;

    auto * buffer = file.data();

    auto *             pPtr    = buffer;
    BITMAPFILEHEADER * pHeader = reinterpret_cast<BITMAPFILEHEADER *>(pPtr);
    pPtr += sizeof(BITMAPFILEHEADER);
    if(pHeader->bfSize != file_length || pHeader->bfType != 0x4D42)   // little-endian
        return res;

    if(reinterpret_cast<uint32_t *>(pPtr)[0] == 12)
    {
        BITMAPINFO12 * pInfo = reinterpret_cast<BITMAPINFO12 *>(pPtr);

        if(pInfo->biBitCount != 24 && pInfo->biBitCount != 32)
            return res;

        if(pInfo->biBitCount == 24)
            id.type = ImageData::PixelType::pt_rgb;
        else
            id.type = ImageData::PixelType::pt_rgba;

        id.width  = pInfo->biWidth;
        id.height = pInfo->biHeight;
    }
    else
    {
        BITMAPINFO * pInfo = reinterpret_cast<BITMAPINFO *>(pPtr);

        if(pInfo->biBitCount != 24 && pInfo->biBitCount != 32)
            return res;

        if(pInfo->biCompression != 3 && pInfo->biCompression != 6 && pInfo->biCompression != 0)
        {
            return res;
        }

        if(pInfo->biCompression == 3 || pInfo->biCompression == 6)
        {
            compressed = true;
        }

        if(pInfo->biBitCount == 24)
            id.type = ImageData::PixelType::pt_rgb;
        else
            id.type = ImageData::PixelType::pt_rgba;

        id.width = static_cast<uint32_t>(pInfo->biWidth);

        if(pInfo->biHeight < 0)
            flip = true;
        id.height = static_cast<uint32_t>(std::abs(pInfo->biHeight));
    }

    // read data:
    pPtr                     = buffer + pHeader->bfOffBits;
    uint32_t lineLength      = 0;
    uint32_t bytes_per_pixel = (id.type == ImageData::PixelType::pt_rgb ? 3 : 4);
    auto     image           = std::make_unique<uint8_t[]>(id.width * id.height * bytes_per_pixel);
    id.data_size             = id.width * id.height * bytes_per_pixel;
    uint8_t  red, green, blue, alpha;
    uint32_t w_ind(0), h_ind(0);

    if(id.type == ImageData::PixelType::pt_rgb)
        lineLength = id.width * bytes_per_pixel + id.width % 4;
    else
        lineLength = id.width * bytes_per_pixel;

    for(uint32_t i = 0; i < id.height; ++i)
    {
        w_ind = 0;
        for(uint32_t j = 0; j < lineLength; j += bytes_per_pixel)
        {
            if(j > id.width * bytes_per_pixel)
                continue;

            if(compressed)
            {
                uint32_t count = 0;
                if(id.type == ImageData::PixelType::pt_rgba)
                {
                    alpha = pPtr[i * lineLength + j + count];
                    count++;
                }
                blue = pPtr[i * lineLength + j + count];
                count++;
                green = pPtr[i * lineLength + j + count];
                count++;
                red = pPtr[i * lineLength + j + count];
            }
            else
            {
                blue  = pPtr[i * lineLength + j + 0];
                green = pPtr[i * lineLength + j + 1];
                red   = pPtr[i * lineLength + j + 2];
                if(id.type == ImageData::PixelType::pt_rgba)   // !!!Not supported - the high byte in each
                                                               // DWORD is not used
                    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd183376(v=vs.85).aspx
                    alpha = pPtr[i * lineLength + j + 3];
            }

            image[h_ind * id.width * bytes_per_pixel + w_ind * bytes_per_pixel + 0] = red;
            image[h_ind * id.width * bytes_per_pixel + w_ind * bytes_per_pixel + 1] = green;
            image[h_ind * id.width * bytes_per_pixel + w_ind * bytes_per_pixel + 2] = blue;
            if(id.type == ImageData::PixelType::pt_rgba)
                image[h_ind * id.width * bytes_per_pixel + w_ind * bytes_per_pixel + 3] = alpha;
            w_ind++;
        }
        h_ind++;
    }

    // flip image if necessary
    if(flip)
    {
        auto temp_buf = std::make_unique<uint8_t[]>(id.width * id.height * bytes_per_pixel);

        for(uint32_t i = 0; i < id.height; i++)
        {
            std::memcpy(temp_buf.get() + i * id.width * bytes_per_pixel,
                        image.get() + (id.height - 1 - i) * id.width * bytes_per_pixel,
                        id.width * bytes_per_pixel);
        }

        image = std::move(temp_buf);
    }

    id.data = std::move(image);
    res     = true;

    return res;
}

//==============================================================================
//         TGA section
//==============================================================================
bool WriteTGA(std::string file_name, ImageData const & id)
{
    TGAHEADER tga;
    std::memset(&tga, 0, sizeof(tga));
    uint8_t bytes_per_pixel = (id.type == ImageData::PixelType::pt_rgb ? 3 : 4);

    tga.datatypecode = 2;
    tga.width        = static_cast<uint16_t>(id.width);
    tga.height       = static_cast<uint16_t>(id.height);
    tga.bitsperpixel = static_cast<uint8_t>(bytes_per_pixel * 8);
    if(id.type == ImageData::PixelType::pt_rgb)
        tga.imagedescriptor = 0x10;
    else
        tga.imagedescriptor = 0x18;

    std::vector<uint8_t> out_data;
    out_data.resize(sizeof(tga));
    std::memcpy(out_data.data(), &tga, sizeof(tga));

    uint8_t * data_ptr = id.data.get();
    uint8_t   red, green, blue, alpha;
    for(uint32_t i = 0; i < id.width * id.height * bytes_per_pixel; i += bytes_per_pixel)
    {
        red   = data_ptr[i + 0];
        green = data_ptr[i + 1];
        blue  = data_ptr[i + 2];
        if(bytes_per_pixel == 4)
            alpha = data_ptr[i + 3];

        out_data.push_back(blue);
        out_data.push_back(green);
        out_data.push_back(red);
        if(id.type == ImageData::PixelType::pt_rgba)
            out_data.push_back(alpha);
    }

    std::ofstream ofile(file_name, std::ios::binary);
    if(!ofile.is_open())
        return false;

    ofile.write(reinterpret_cast<char *>(out_data.data()), static_cast<std::streamsize>(out_data.size()));
    ofile.close();

    return true;
}

bool ReadUncompressedTGA(ImageData & image, uint8_t * data);
bool ReadCompressedTGA(ImageData & image, uint8_t * data);

bool ReadTGA(std::string const & file_name, ImageData & id)
{
    size_t file_length = 0;

    std::ifstream     ifile(file_name, std::ios::binary);
    std::vector<char> file;

    if(ifile.is_open())
    {
        ifile.seekg(0, std::ios_base::end);
        auto length = ifile.tellg();
        ifile.seekg(0, std::ios_base::beg);

        file.resize(static_cast<size_t>(length));

        ifile.read(reinterpret_cast<char *>(file.data()), length);

        auto success = !ifile.fail() && length == ifile.gcount();
        ifile.close();

        if(!success)
            return false;
    }
    else
        return false;

    file_length = file.size();
    if(file_length == 0)
        return false;

    auto * buffer = file.data();

    uint8_t *   data     = reinterpret_cast<uint8_t *>(buffer);
    TGAHEADER * p_header = reinterpret_cast<TGAHEADER *>(data);

    data += sizeof(TGAHEADER);
    if((p_header->width == 0) || (p_header->height == 0)
       || ((p_header->bitsperpixel != 24)
           && (p_header->bitsperpixel != 32)))   // Make sure all information is valid
    {
        return false;
    }

    id.width  = p_header->width;
    id.height = p_header->height;
    id.type   = p_header->bitsperpixel == 24 ? ImageData::PixelType::pt_rgb : ImageData::PixelType::pt_rgba;
    bool flip_horizontal = (p_header->imagedescriptor & 0x10);
    bool flip_vertical   = (p_header->imagedescriptor & 0x20);

    uint32_t bytes_per_pixel = p_header->bitsperpixel / 8;
    id.data_size             = id.width * id.height * bytes_per_pixel;

    if(p_header->datatypecode == 2)
    {
        ReadUncompressedTGA(id, data);
    }
    else if(p_header->datatypecode == 10)
    {
        if(!ReadCompressedTGA(id, data))
            return false;
    }

    if(flip_vertical)
    {
        auto flipped_image = std::make_unique<uint8_t[]>(id.data_size);

        for(uint32_t i = 0; i < id.height; i++)
        {
            std::memcpy(flipped_image.get() + i * id.width * bytes_per_pixel,
                        id.data.get() + (id.height - 1 - i) * id.width * bytes_per_pixel,
                        id.width * bytes_per_pixel);
        }

        id.data = std::move(flipped_image);
    }

    if(flip_horizontal)
    {
        auto flipped_image = std::make_unique<uint8_t[]>(id.data_size);

        for(uint32_t i = 0; i < id.height; i++)
        {
            for(uint32_t j = 0; j < id.width; j++)
            {
                flipped_image[id.width * bytes_per_pixel * i + j * bytes_per_pixel + 0] =
                    id.data[id.width * bytes_per_pixel * i + (id.width - j - 1) * bytes_per_pixel + 0];
                flipped_image[id.width * bytes_per_pixel * i + j * bytes_per_pixel + 1] =
                    id.data[id.width * bytes_per_pixel * i + (id.width - j - 1) * bytes_per_pixel + 1];
                flipped_image[id.width * bytes_per_pixel * i + j * bytes_per_pixel + 2] =
                    id.data[id.width * bytes_per_pixel * i + (id.width - j - 1) * bytes_per_pixel + 2];
                if(id.type == ImageData::PixelType::pt_rgba)
                    flipped_image[id.width * bytes_per_pixel * i + j * bytes_per_pixel + 3] =
                        id.data[id.width * bytes_per_pixel * i + (id.width - j - 1) * bytes_per_pixel + 3];
            }
        }

        id.data = std::move(flipped_image);
    }

    return true;
}

bool ReadUncompressedTGA(ImageData & image, uint8_t * data)
{
    uint32_t bytes_per_pixel = image.type == ImageData::PixelType::pt_rgb ? 3 : 4;
    auto     img             = std::make_unique<uint8_t[]>(image.data_size);

    for(uint32_t i = 0; i < image.width * image.height; ++i)
    {
        uint8_t red, green, blue, alpha;

        red   = data[i * bytes_per_pixel + 2];
        green = data[i * bytes_per_pixel + 1];
        blue  = data[i * bytes_per_pixel + 0];
        if(image.type == ImageData::PixelType::pt_rgba)
            alpha = data[i * bytes_per_pixel + 3];

        img[i * bytes_per_pixel + 0] = red;
        img[i * bytes_per_pixel + 1] = green;
        img[i * bytes_per_pixel + 2] = blue;
        if(image.type == ImageData::PixelType::pt_rgba)
            img[i * bytes_per_pixel + 3] = alpha;
    }

    image.data = std::move(img);

    return true;
}

bool ReadCompressedTGA(ImageData & image, uint8_t * data)
{
    uint32_t bytes_per_pixel = image.type == ImageData::PixelType::pt_rgb ? 3 : 4;
    auto     img             = std::make_unique<uint8_t[]>(image.data_size);
    uint32_t pixel_count     = image.height * image.width;
    uint32_t current_pixel   = 0;
    uint32_t current_byte    = 0;

    do
    {
        uint8_t chunk = data[0];
        data++;

        if(chunk > 128)
        {
            chunk -= 127;
            for(uint16_t counter = 0; counter < chunk; counter++)
            {
                img[current_byte + 0] = data[2];
                img[current_byte + 1] = data[1];
                img[current_byte + 2] = data[0];
                if(image.type == ImageData::PixelType::pt_rgba)
                    img[current_byte + 3] = data[3];

                current_byte += bytes_per_pixel;
                current_pixel++;

                if(current_pixel > pixel_count)   // Make sure we havent read too many pixels
                {
                    return false;
                }
            }
            data += bytes_per_pixel;
        }
        else
        {
            chunk++;
            for(uint16_t counter = 0; counter < chunk; counter++)
            {
                img[current_byte + 0] = data[2];
                img[current_byte + 1] = data[1];
                img[current_byte + 2] = data[0];
                if(image.type == ImageData::PixelType::pt_rgba)
                    img[current_byte + 3] = data[3];

                current_byte += bytes_per_pixel;
                current_pixel++;
                data += bytes_per_pixel;

                if(current_pixel > pixel_count)   // Make sure we havent read too many pixels
                {
                    return false;
                }
            }
        }
    } while(current_pixel < pixel_count);

    image.data = std::move(img);
    return true;
}
}   // namespace tex

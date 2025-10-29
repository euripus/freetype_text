#include "imagedata.h"
#include "../fs/file.h"
#include <cstring>
#include <vector>
#include <fstream>

#pragma pack(push, 1)

struct BITMAPFILEHEADER
{
    uint16_t bf_type;   // bmp file signature
    uint32_t bf_size;   // file size
    uint16_t bf_reserved1;
    uint16_t bf_reserved2;
    uint32_t bf_off_bits;
};

struct BITMAPINFO12   // CORE version
{
    uint32_t bi_size;
    uint16_t bi_width;
    uint16_t bi_height;
    uint16_t bi_planes;
    uint16_t bi_bit_count;
};

struct BITMAPINFO   // Standart version
{
    uint32_t bi_size;
    int32_t  bi_width;
    int32_t  bi_height;
    uint16_t bi_planes;
    uint16_t bi_bit_count;
    uint32_t bi_compression;
    uint32_t bi_size_image;
    int32_t  bi_x_pels_per_meter;
    int32_t  bi_y_pels_per_meter;
    uint32_t bi_clr_used;
    uint32_t bi_clr_important;
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
bool ReadBMPData(uint8_t * buffer, size_t file_size, ImageData & image);

bool ReadBMP(std::string const & file_name, ImageData & image)
{
    size_t file_length = 0;

    image.width  = 0;
    image.height = 0;
    image.type   = ImageData::PixelType::pt_none;
    if(image.data)
        image.data.reset(nullptr);

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
        return false;

    auto * buffer = file.data();

    return ReadBMPData(buffer, file_length, image);
}

bool ReadBMP(BaseFile const & file, ImageData & image)
{
    if(file.empty())
        return false;

    auto * buffer      = const_cast<uint8_t *>(reinterpret_cast<uint8_t const *>(file.getData()));
    size_t file_length = file.getFileSize();

    return ReadBMPData(buffer, file_length, image);
}

bool ReadBMPData(uint8_t * buffer, size_t file_size, ImageData & image)
{
    bool compressed = false;
    bool flip       = false;

    auto *             pPtr     = buffer;
    BITMAPFILEHEADER * pHeader  = reinterpret_cast<BITMAPFILEHEADER *>(pPtr);
    pPtr                       += sizeof(BITMAPFILEHEADER);
    if(pHeader->bf_size != file_size || pHeader->bf_type != 0x4D42)   // little-endian
        return false;

    if(reinterpret_cast<uint32_t *>(pPtr)[0] == 12)
    {
        BITMAPINFO12 * pInfo = reinterpret_cast<BITMAPINFO12 *>(pPtr);

        if(pInfo->bi_bit_count != 24 && pInfo->bi_bit_count != 32)
            return false;

        if(pInfo->bi_bit_count == 24)
            image.type = ImageData::PixelType::pt_rgb;
        else
            image.type = ImageData::PixelType::pt_rgba;

        image.width  = pInfo->bi_width;
        image.height = pInfo->bi_height;
    }
    else
    {
        BITMAPINFO * pInfo = reinterpret_cast<BITMAPINFO *>(pPtr);

        if(pInfo->bi_bit_count != 24 && pInfo->bi_bit_count != 32)
            return false;

        if(pInfo->bi_compression != 3 && pInfo->bi_compression != 6 && pInfo->bi_compression != 0)
        {
            return false;
        }

        if(pInfo->bi_compression == 3 || pInfo->bi_compression == 6)
        {
            compressed = true;
        }

        if(pInfo->bi_bit_count == 24)
            image.type = ImageData::PixelType::pt_rgb;
        else
            image.type = ImageData::PixelType::pt_rgba;

        image.width = static_cast<uint32_t>(pInfo->bi_width);

        if(pInfo->bi_height < 0)
            flip = false;
        else
            flip = true;
        image.height = static_cast<uint32_t>(std::abs(pInfo->bi_height));
    }

    // read data:
    pPtr                     = buffer + pHeader->bf_off_bits;
    uint32_t line_length     = 0;
    uint32_t bytes_per_pixel = (image.type == ImageData::PixelType::pt_rgb ? 3 : 4);
    image.data_size          = image.width * image.height * bytes_per_pixel;
    auto     data            = std::make_unique<uint8_t[]>(image.data_size);
    uint8_t  red, green, blue, alpha;
    uint32_t w_ind(0), h_ind(0);

    if(image.type == ImageData::PixelType::pt_rgb)
        line_length = ((image.width * bytes_per_pixel + 3) / 4) * 4;
    else
        line_length = image.width * bytes_per_pixel;

    for(uint32_t i = 0; i < image.height; ++i)
    {
        w_ind = 0;
        for(uint32_t j = 0; j < line_length; j += bytes_per_pixel)
        {
            if(j > image.width * bytes_per_pixel)
                break;

            if(compressed)
            {
                uint32_t count = 0;
                if(image.type == ImageData::PixelType::pt_rgba)
                {
                    alpha = pPtr[i * line_length + j + count];
                    count++;
                }
                blue = pPtr[i * line_length + j + count];
                count++;
                green = pPtr[i * line_length + j + count];
                count++;
                red = pPtr[i * line_length + j + count];
            }
            else
            {
                blue  = pPtr[i * line_length + j + 0];
                green = pPtr[i * line_length + j + 1];
                red   = pPtr[i * line_length + j + 2];
                if(image.type == ImageData::PixelType::pt_rgba)   // !!!Not supported - the high byte in each
                                                                  // DWORD is not used
                    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd183376(v=vs.85).aspx
                    alpha = pPtr[i * line_length + j + 3];
            }

            data[h_ind * image.width * bytes_per_pixel + w_ind * bytes_per_pixel + 0] = red;
            data[h_ind * image.width * bytes_per_pixel + w_ind * bytes_per_pixel + 1] = green;
            data[h_ind * image.width * bytes_per_pixel + w_ind * bytes_per_pixel + 2] = blue;
            if(image.type == ImageData::PixelType::pt_rgba)
                data[h_ind * image.width * bytes_per_pixel + w_ind * bytes_per_pixel + 3] = alpha;
            w_ind++;
        }
        h_ind++;
    }

    // flip image if necessary
    if(flip)
    {
        auto temp_buf = std::make_unique<uint8_t[]>(image.width * image.height * bytes_per_pixel);

        for(uint32_t i = 0; i < image.height; i++)
        {
            std::memcpy(temp_buf.get() + i * image.width * bytes_per_pixel,
                        data.get() + (image.height - 1 - i) * image.width * bytes_per_pixel,
                        image.width * bytes_per_pixel);
        }

        data = std::move(temp_buf);
    }

    image.data = std::move(data);

    return true;
}
//==============================================================================
//         TGA section
//==============================================================================
bool WriteTGA(std::string file_name, ImageData const & image)
{
    TGAHEADER tga;
    std::memset(&tga, 0, sizeof(tga));
    uint8_t bytes_per_pixel = (image.type == ImageData::PixelType::pt_rgb ? 3 : 4);

    tga.datatypecode = 2;
    tga.width        = static_cast<uint16_t>(image.width);
    tga.height       = static_cast<uint16_t>(image.height);
    tga.bitsperpixel = static_cast<uint8_t>(bytes_per_pixel * 8);
    if(image.type == ImageData::PixelType::pt_rgb)
        tga.imagedescriptor = 0x10;
    else
        tga.imagedescriptor = 0x18;

    std::vector<uint8_t> out_data;
    out_data.resize(sizeof(tga));
    std::memcpy(out_data.data(), &tga, sizeof(tga));

    uint8_t * data_ptr = image.data.get();
    uint8_t   red, green, blue, alpha;
    for(uint32_t i = 0; i < image.width * image.height * bytes_per_pixel; i += bytes_per_pixel)
    {
        red   = data_ptr[i + 0];
        green = data_ptr[i + 1];
        blue  = data_ptr[i + 2];
        if(bytes_per_pixel == 4)
            alpha = data_ptr[i + 3];

        out_data.push_back(blue);
        out_data.push_back(green);
        out_data.push_back(red);
        if(image.type == ImageData::PixelType::pt_rgba)
            out_data.push_back(alpha);
    }

    std::ofstream ofile(file_name, std::ios::binary);
    if(!ofile.is_open())
        return false;

    ofile.write(reinterpret_cast<char *>(out_data.data()), static_cast<std::streamsize>(out_data.size()));
    ofile.close();

    return true;
}

bool ReadRawTGAData(ImageData & image, uint8_t * buffer);
bool ReadUncompressedTGA(ImageData & image, uint8_t * data);
bool ReadCompressedTGA(ImageData & image, uint8_t * data);

bool ReadTGA(std::string const & file_name, ImageData & image)
{
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

    if(file.size() == 0)
        return false;

    auto * buffer = file.data();

    return ReadRawTGAData(image, buffer);
}

bool ReadTGA(BaseFile const & file, ImageData & image)
{
    if(file.empty())
        return false;

    auto * buffer = const_cast<uint8_t *>(reinterpret_cast<uint8_t const *>(file.getData()));

    return ReadRawTGAData(image, buffer);
}

bool ReadRawTGAData(ImageData & image, uint8_t * buffer)
{
    uint8_t *   data     = buffer;
    TGAHEADER * p_header = reinterpret_cast<TGAHEADER *>(data);

    data += sizeof(TGAHEADER);
    if((p_header->width == 0) || (p_header->height == 0)
       || ((p_header->bitsperpixel != 24)
           && (p_header->bitsperpixel != 32)))   // Make sure all information is valid
    {
        return false;
    }

    image.width  = p_header->width;
    image.height = p_header->height;
    image.type = p_header->bitsperpixel == 24 ? ImageData::PixelType::pt_rgb : ImageData::PixelType::pt_rgba;
    bool flip_horizontal = (p_header->imagedescriptor & 0x10);
    bool flip_vertical   = (p_header->imagedescriptor & 0x20);

    uint32_t bytes_per_pixel = p_header->bitsperpixel / 8;
    image.data_size          = image.width * image.height * bytes_per_pixel;

    if(p_header->datatypecode == 2)
    {
        ReadUncompressedTGA(image, data);
    }
    else if(p_header->datatypecode == 10)
    {
        if(!ReadCompressedTGA(image, data))
            return false;
    }

    if(flip_vertical)
    {
        auto flipped_image = std::make_unique<uint8_t[]>(image.data_size);

        for(uint32_t i = 0; i < image.height; i++)
        {
            std::memcpy(flipped_image.get() + i * image.width * bytes_per_pixel,
                        image.data.get() + (image.height - 1 - i) * image.width * bytes_per_pixel,
                        image.width * bytes_per_pixel);
        }

        image.data = std::move(flipped_image);
    }

    if(flip_horizontal)
    {
        auto flipped_image = std::make_unique<uint8_t[]>(image.data_size);

        for(uint32_t i = 0; i < image.height; i++)
        {
            for(uint32_t j = 0; j < image.width; j++)
            {
                flipped_image[image.width * bytes_per_pixel * i + j * bytes_per_pixel + 0] =
                    image.data[image.width * bytes_per_pixel * i + (image.width - j - 1) * bytes_per_pixel
                               + 0];
                flipped_image[image.width * bytes_per_pixel * i + j * bytes_per_pixel + 1] =
                    image.data[image.width * bytes_per_pixel * i + (image.width - j - 1) * bytes_per_pixel
                               + 1];
                flipped_image[image.width * bytes_per_pixel * i + j * bytes_per_pixel + 2] =
                    image.data[image.width * bytes_per_pixel * i + (image.width - j - 1) * bytes_per_pixel
                               + 2];
                if(image.type == ImageData::PixelType::pt_rgba)
                    flipped_image[image.width * bytes_per_pixel * i + j * bytes_per_pixel + 3] =
                        image.data[image.width * bytes_per_pixel * i + (image.width - j - 1) * bytes_per_pixel
                                   + 3];
            }
        }

        image.data = std::move(flipped_image);
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

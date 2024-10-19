#ifndef ZIP_H_INCLUDED
#define ZIP_H_INCLUDED

#include <cstdint>

#pragma pack(push, 1)
struct EOCD   // End of central directory record
{
    uint16_t disk_number;
    uint16_t start_disk_number;
    uint16_t number_central_directory_record;
    uint16_t total_central_directory_record;
    uint32_t size_of_central_directory;
    uint32_t central_directory_offset;
    uint16_t comment_length;
    // zip file comment follows
};

struct CentralDirectoryFileHeader
{
    uint32_t signature;                  // PK12
    uint16_t version_made_by;            // version made by
    uint16_t version_to_extract;         // version needed to extract
    uint16_t general_purpose_bit_flag;   // general purpose bit flag
    uint16_t compression_method;         // compression method
    uint16_t modification_time;          // last mod file time
    uint16_t modification_date;          // last mod file date
    uint32_t crc32;                      // crc-32
    uint32_t compressed_size;            // compressed size
    uint32_t uncompressed_size;          // uncompressed size
    uint16_t filename_length;            // file name length
    uint16_t extra_field_length;         // extra field length
    uint16_t file_comment_length;        // file comment length
    uint16_t disk_number;                // disk number start
    uint16_t internal_file_attributes;   // internal file attributes
    uint32_t external_file_attributes;   // external file attributes
    uint32_t local_file_header_offset;   // relative offset of local header
                                         // file name (variable size)
                                         // extra field (variable size)
                                         // file comment (variable size)
};

struct LocalFileHeader
{
    uint32_t signature;                  // local file header signature
    uint16_t version_to_extract;         // version needed to extract
    uint16_t general_purpose_bit_flag;   // general purpose bit flag
    uint16_t compression_method;         // compression method
    uint16_t modification_time;          // last mod file time
    uint16_t modification_date;          // last mod file date
    uint32_t crc32;                      // crc-32
    uint32_t compressed_size;            // compressed size
    uint32_t uncompressed_size;          // uncompressed size
    uint16_t filename_length;            // file name length
    uint16_t extra_field_length;         // extra field length
                                         // file name (variable size)
                                         // extra field (variable size)
};

#pragma pack(pop)

#endif   // ZIP_H_INCLUDED

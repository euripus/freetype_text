#include "file_system.h"
#include <random>
#include <array>
#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <zlib.h>
#include <iostream>

#include "zip.h"

// https://medium.com/@sshambir/%D0%BF%D1%80%D0%B8%D0%B2%D0%B5%D1%82-std-filesystem-4c7ed50d5634
namespace fs = std::filesystem;

// https://stackoverflow.com/questions/61030383/how-to-convert-stdfilesystemfile-time-type-to-time-t
template<typename TP>
std::time_t To_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
    return system_clock::to_time_t(sctp);
}

void GetDosTime(std::time_t rawtime, uint16_t & time, uint16_t & date)
{
    // http://stackoverflow.com/questions/15763259/unix-timestamp-to-fat-timestamp
    struct tm * timeinfo;
    timeinfo = localtime(&rawtime);

    time =
        static_cast<uint16_t>((timeinfo->tm_hour << 11) | (timeinfo->tm_min << 5) | (timeinfo->tm_sec >> 1));
    date = static_cast<uint16_t>(((timeinfo->tm_year - 80) << 9) | ((timeinfo->tm_mon + 1) << 5)
                                 | (timeinfo->tm_mday));
}

std::chrono::system_clock::time_point
    file_time_to_time_point(std::filesystem::file_time_type const & file_time)
{
    auto system_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        file_time - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    return system_time;
}

std::filesystem::file_time_type tm_to_file_time(std::tm const & timeinfo)
{
    std::time_t time_t_val = std::mktime(const_cast<std::tm *>(&timeinfo));
    auto        time_point = std::chrono::system_clock::from_time_t(time_t_val);
    return std::filesystem::file_time_type::clock::now() + (time_point - std::chrono::system_clock::now());
}

std::chrono::system_clock::time_point tm_to_time_point(std::tm const & timeinfo)
{
    std::time_t time_t_val = std::mktime(const_cast<std::tm *>(&timeinfo));
    return std::chrono::system_clock::from_time_t(time_t_val);
}

std::string FileSystem::GetTempDir()
{
    return fs::temp_directory_path().generic_string();
}

std::string FileSystem::GetCurrentDir()
{
    return fs::current_path().generic_string();
}

// https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
template<typename T = std::mt19937>
T random_generator()
{
    auto constexpr seed_bytes = sizeof(typename T::result_type) * T::state_size;
    auto constexpr seed_len   = seed_bytes / sizeof(std::seed_seq::result_type);

    auto seed = std::array<std::seed_seq::result_type, seed_len>();
    auto dev  = std::random_device();
    std::generate_n(begin(seed), seed_len, std::ref(dev));
    auto seed_seq = std::seed_seq(begin(seed), end(seed));

    return T{seed_seq};
}

std::string generate_random_alphanumeric_string(std::size_t len)
{
    static constexpr auto chars = "0123456789"
                                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                  "abcdefghijklmnopqrstuvwxyz";

    thread_local auto rng    = random_generator<>();
    auto              dist   = std::uniform_int_distribution{{}, std::strlen(chars) - 1};
    auto              result = std::string(len, '\0');

    std::generate_n(begin(result), len, [&]() { return chars[dist(rng)]; });

    return result;
}

std::string FileSystem::GetTempFileName()
{
    return generate_random_alphanumeric_string(16) + ".tmp";
}

FileSystem::FileSystem(std::string root_dir)
{
    assert(!root_dir.empty());

    std::swap(m_data_dir, root_dir);
    std::list<std::string> zip_file_list;

    fs::path p(m_data_dir);

    if(fs::exists(p) && fs::is_directory(p))
    {
        for(auto it = fs::recursive_directory_iterator(p); it != fs::recursive_directory_iterator(); it++)
        {
            fs::path const & lp = (*it).path();
            if(fs::is_regular_file(lp))
            {
                if(lp.has_extension() && lp.extension() == fs::path(".zip"))
                {
                    zip_file_list.push_back(lp.generic_string());
                }
                else
                {
                    std::string tmp_fname = lp.generic_string();
                    tmp_fname.erase(0, m_data_dir.length() + 1);

                    file_data fd;
                    fd.fname = tmp_fname;

                    m_files.emplace_back(std::move(fd));
                }
            }
        }
    }

    if(!zip_file_list.empty())
        std::for_each(zip_file_list.begin(), zip_file_list.end(),
                      [this](std::string const & fname) { this->addZippedDir(fname); });
}

void FileSystem::addZippedDir(std::string const & fname)
{
    std::ifstream ifs;
    ifs.open(fname, std::ifstream::in | std::ifstream::binary);
    if(!ifs.is_open())
    {
        std::stringstream ss;
        ss << "FileSystem::AddZippedDir File: " << fname << " - not found";
        std::cout << ss.str() << std::endl;
        return;
    }

    // get length of file:
    ifs.seekg(0, std::ifstream::end);
    size_t const file_size   = static_cast<size_t>(ifs.tellg());
    size_t       EOCD_offset = 0;

    for(size_t offset = file_size - sizeof(EOCD); offset != 0; --offset)
    {
        uint32_t signature = 0;

        ifs.seekg(offset, std::ifstream::beg);
        ifs.read(reinterpret_cast<char *>(&signature), sizeof(signature));

        if(0x06054b50 == signature)
        {
            EOCD_offset = static_cast<size_t>(ifs.tellg());
            break;
        }
    }

    if(EOCD_offset == 0)
    {
        std::stringstream ss;
        ss << "FileSystem::AddZippedDir File: " << fname << " - not found EOCD_offset signature";
        std::cout << ss.str() << std::endl;
        return;
    }

    EOCD eocd{};
    ifs.seekg(EOCD_offset, std::ifstream::beg);
    ifs.read(reinterpret_cast<char *>(&eocd), sizeof(eocd));

    ifs.seekg(eocd.central_directory_offset, std::ifstream::beg);
    CentralDirectoryFileHeader cdfh{};

    for(uint16_t i = 0; i < eocd.number_central_directory_record; ++i)
    {
        file_data zfile;
        zfile.is_zip = true;

        ifs.read(reinterpret_cast<char *>(&cdfh), sizeof(cdfh));

        if(0x02014b50 != cdfh.signature)
        {
            std::stringstream ss;
            ss << "FileSystem::AddZippedDir File: " << fname
               << " - not found CentralDirectoryFileHeader signature";
            std::cout << ss.str() << std::endl;
            return;
        }

        if(cdfh.general_purpose_bit_flag & 0x1)   // encrypted
        {
            std::stringstream ss;
            ss << "FileSystem::AddZippedDir File: " << fname << " - encrypted";
            std::cout << ss.str() << std::endl;
            return;
        }
        if(cdfh.general_purpose_bit_flag & 0x8)   // DataDescr Struct
        {
            std::stringstream ss;
            ss << "FileSystem::AddZippedDir File: " << fname << " - DataDescr Struct";
            std::cout << ss.str() << std::endl;
            return;
        }

        if(cdfh.filename_length)
        {
            std::unique_ptr<char[]> filename = std::make_unique<char[]>(cdfh.filename_length + 1);
            ifs.read(reinterpret_cast<char *>(filename.get()), cdfh.filename_length);

            filename[cdfh.filename_length] = 0;

            zfile.zip_data.fname = std::string(filename.get());
        }

        if(cdfh.compression_method != 0 && cdfh.compression_method != Z_DEFLATED)
        {
            continue;
        }

        zfile.zip_data.compressed        = (Z_DEFLATED == cdfh.compression_method);
        zfile.zip_data.compressed_size   = cdfh.compressed_size;
        zfile.zip_data.uncompressed_size = cdfh.uncompressed_size;
        zfile.zip_data.lfh_offset        = cdfh.local_file_header_offset;

        if(zfile.zip_data.uncompressed_size != 0 || zfile.zip_data.compressed != 0)
        {
            zfile.fname = fname;
            m_files.emplace_back(std::move(zfile));
        }

        if(cdfh.extra_field_length)
        {
            ifs.seekg(cdfh.extra_field_length, std::ifstream::cur);
        }
        if(cdfh.file_comment_length)
        {
            ifs.seekg(cdfh.file_comment_length, std::ifstream::cur);
        }
    }
}

bool FileSystem::isExist(std::string const & fname) const
{
    assert(!fname.empty());

    for(auto & fl: m_files)
    {
        if(fl.is_zip)
        {
            if(fname == fl.zip_data.fname)
                return true;
        }
        else
        {
            if(fname == fl.fname)
                return true;
        }
    }

    return false;
}

std::optional<InFile> FileSystem::getFile(std::string const & fname) const
{
    assert(!fname.empty());

    auto res = std::find_if(m_files.begin(), m_files.end(), [&fname](file_data const & f) -> bool {
        if(f.is_zip)
        {
            if(fname == f.zip_data.fname)
                return true;
        }
        else
        {
            if(fname == f.fname)
                return true;
        }

        return false;
    });

    if(res != m_files.end())
    {
        if((*res).is_zip)
            return loadZipFile(*res);
        else
            return loadRegularFile(*res);
    }
    else
    {
        std::stringstream ss;
        ss << "FileSystem::GetFile File: " << fname << " - not found";
        std::cout << ss.str() << std::endl;
        return {};
    }
}

bool FileSystem::writeFile(BaseFile const & file, std::string path)
{
    std::string filename = file.getName();

    if(!path.empty())
        filename = path + '/' + filename;

    std::ofstream ofs(std::string(m_data_dir + '/' + filename), std::ios::binary);
    if(!ofs.is_open())
    {
        std::stringstream ss;
        ss << "FileSystem::WriteFile File: " << filename << " - not writed";
        std::cout << ss.str() << std::endl;
        return false;
    }
    ofs.write(reinterpret_cast<char *>(const_cast<int8_t *>(file.getData())),
              static_cast<std::streamsize>(file.getFileSize()));
    ofs.close();

    if(!isExist(filename))
    {
        file_data fd;
        fd.fname = filename;

        m_files.emplace_back(std::move(fd));
    }

    return true;
}

// http://blog2k.ru/archives/3397
bool FileSystem::createZIP(std::vector<BaseFile const *> filelist, std::string const & zipname)
{
    // http://stackoverflow.com/questions/922360/why-cant-i-make-a-vector-of-references
    assert(!filelist.empty());
    assert(!zipname.empty());

    struct FileInfo
    {
        uint32_t compressed_size;
        uint32_t uncompressed_size;
        uint16_t compression_method;
        uint32_t crc32;
        uint32_t offset;
    };

    // Buffer for compressed data
    std::vector<uint8_t> data_buffer;
    // Information for creation Central directory file header
    std::vector<FileInfo> file_info_list;

    std::ofstream ofs(std::string(m_data_dir + '/' + zipname), std::ofstream::binary | std::ofstream::trunc);
    if(!ofs.is_open())
    {
        return false;
    }

    uint16_t time = 0;
    uint16_t date = 0;

    for(size_t i = 0; i < filelist.size(); i++)
    {
        assert(filelist[i] != nullptr);

        std::string       fname = filelist[i]->getName();
        std::vector<char> buf;
        buf.resize(filelist[i]->getFileSize());
        buf.assign(filelist[i]->getData(), filelist[i]->getData() + filelist[i]->getFileSize());

        LocalFileHeader lfh{};
        memset(&lfh, 0, sizeof(lfh));

        lfh.uncompressed_size = static_cast<uint32_t>(buf.size());

        lfh.crc32 = static_cast<uint32_t>(
            crc32(0, reinterpret_cast<unsigned char const *>(buf.data()), lfh.uncompressed_size));

        // Let's allocate memory for compressed data
        data_buffer.resize(lfh.uncompressed_size);

        // Structure for data compression
        z_stream zStream;
        memset(&zStream, 0, sizeof(zStream));
        deflateInit2(&zStream, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);

        // Compressing data
        zStream.avail_in  = lfh.uncompressed_size;
        zStream.next_in   = reinterpret_cast<unsigned char *>(buf.data());
        zStream.avail_out = lfh.uncompressed_size;
        zStream.next_out  = data_buffer.data();
        deflate(&zStream, Z_FINISH);

        // Compressed data size
        char *   data_ptr = nullptr;
        uint32_t size     = 0;
        if(zStream.total_out >= lfh.uncompressed_size)
        {
            lfh.compressed_size    = lfh.uncompressed_size;
            lfh.compression_method = 0;

            data_ptr = buf.data();
            size     = lfh.uncompressed_size;
        }
        else
        {
            lfh.compressed_size    = static_cast<uint32_t>(zStream.total_out);
            lfh.compression_method = Z_DEFLATED;

            data_ptr = reinterpret_cast<char *>(data_buffer.data());
            size     = lfh.compressed_size;
        }

        deflateEnd(&zStream);

        lfh.filename_length = static_cast<uint16_t>(fname.size());

        // Save the offset to the record Local File Header внутри архива
        uint32_t const lfh_offset = static_cast<uint32_t>(ofs.tellp());

        GetDosTime(To_time_t(filelist[i]->timeStamp()), time, date);

        // Write down the signature Local File Header
        lfh.signature         = 0x04034b50;
        lfh.modification_date = date;
        lfh.modification_time = time;
        // Write down Local File Header
        ofs.write(reinterpret_cast<char *>(&lfh), sizeof(lfh));
        // Write down filename
        ofs.write(fname.c_str(), static_cast<std::streamsize>(fname.size()));
        // Write down data
        ofs.write(data_ptr, static_cast<std::streamsize>(size));

        // Let's save all the data for Central directory file header
        FileInfo file_info{};
        file_info.compressed_size    = lfh.compressed_size;
        file_info.uncompressed_size  = lfh.uncompressed_size;
        file_info.compression_method = lfh.compression_method;
        file_info.crc32              = lfh.crc32;
        file_info.offset             = lfh_offset;
        file_info_list.push_back(file_info);
    }

    // Offset of the first record for EOCD
    uint32_t const first_offset_CDFH = static_cast<uint32_t>(ofs.tellp());

    for(uint32_t i = 0; i < filelist.size(); ++i)
    {
        std::string const &        filename  = filelist[i]->getName();
        FileInfo const &           file_info = file_info_list[i];
        CentralDirectoryFileHeader cdfh{};
        memset(&cdfh, 0, sizeof(cdfh));

        GetDosTime(To_time_t(filelist[i]->timeStamp()), time, date);

        cdfh.compressed_size          = file_info.compressed_size;
        cdfh.uncompressed_size        = file_info.uncompressed_size;
        cdfh.compression_method       = file_info.compression_method;
        cdfh.crc32                    = file_info.crc32;
        cdfh.local_file_header_offset = file_info.offset;
        cdfh.filename_length          = static_cast<uint16_t>(filename.size());
        cdfh.signature                = 0x02014b50;
        cdfh.modification_date        = date;
        cdfh.modification_time        = time;

        ofs.write(reinterpret_cast<char *>(&cdfh), sizeof(cdfh));

        ofs.write(filename.c_str(), cdfh.filename_length);
    }

    // Let's calculate the data size for the next step
    uint32_t const last_offset_CDFH = static_cast<uint32_t>(ofs.tellp());

    EOCD eocd{};
    memset(&eocd, 0, sizeof(eocd));
    eocd.central_directory_offset        = first_offset_CDFH;
    eocd.number_central_directory_record = static_cast<uint16_t>(filelist.size());
    eocd.total_central_directory_record  = static_cast<uint16_t>(filelist.size());
    eocd.size_of_central_directory       = last_offset_CDFH - first_offset_CDFH;

    // Write a signature
    uint32_t signature = 0x06054b50;
    ofs.write(reinterpret_cast<char *>(&signature), sizeof(signature));

    // Write EOCD
    ofs.write(reinterpret_cast<char *>(&eocd), sizeof(eocd));

    ofs.close();
    return true;
}

bool FileSystem::addFileToZIP(BaseFile const * file, std::string const & zipname)
{
    assert(!zipname.empty());
    assert(file != nullptr);

    std::fstream ofs(m_data_dir + '/' + zipname,
                     std::fstream::binary | std::fstream::ate | std::fstream::out | std::fstream::in);
    if(!ofs.is_open())
    {
        return false;
    }

    size_t const file_size   = static_cast<size_t>(ofs.tellg());
    size_t       EOCD_offset = 0;

    for(size_t offset = file_size - sizeof(EOCD); offset != 0; --offset)
    {
        uint32_t signature = 0;

        ofs.seekg(offset, std::fstream::beg);
        ofs.read(reinterpret_cast<char *>(&signature), sizeof(signature));

        if(0x06054b50 == signature)
        {
            EOCD_offset = static_cast<size_t>(ofs.tellg());
            break;
        }
    }

    if(EOCD_offset == 0)
        return false;

    EOCD eocd{};
    ofs.seekg(EOCD_offset, std::fstream::beg);
    ofs.read(reinterpret_cast<char *>(&eocd), sizeof(eocd));

    auto central_directory_data = std::make_unique<char[]>(eocd.size_of_central_directory);
    ofs.seekg(eocd.central_directory_offset, std::fstream::beg);
    ofs.read(central_directory_data.get(), static_cast<std::streamsize>(eocd.size_of_central_directory));
    ofs.seekg(eocd.central_directory_offset, std::fstream::beg);

    std::vector<std::string> zip_fnames;

    for(uint16_t i = 0; i < eocd.number_central_directory_record; ++i)
    {
        CentralDirectoryFileHeader cdfh{};

        ofs.read(reinterpret_cast<char *>(&cdfh), sizeof(cdfh));

        if(0x02014b50 != cdfh.signature)
        {
            return false;
        }

        if(cdfh.filename_length)
        {
            auto filename = std::make_unique<char[]>(cdfh.filename_length + 1);
            ofs.read(static_cast<char *>(filename.get()), cdfh.filename_length);

            filename[cdfh.filename_length] = 0;

            zip_fnames.push_back(std::string(filename.get()));
        }
    }

    ofs.seekg(eocd.central_directory_offset, std::fstream::beg);

    auto file_it = std::find_if(zip_fnames.begin(), zip_fnames.end(),
                                [&file](std::string const & zn) -> bool { return file->getName() == zn; });

    if(file_it != zip_fnames.end())
        return false;   // If there is a file with this name

    uint16_t time = 0;
    uint16_t date = 0;
    GetDosTime(To_time_t(file->timeStamp()), time, date);

    LocalFileHeader lfh{};
    memset(&lfh, 0, sizeof(lfh));
    // Buffer for compressed data
    std::vector<uint8_t> data_buffer;

    lfh.uncompressed_size = static_cast<uint32_t>(file->getFileSize());

    lfh.crc32 = static_cast<uint32_t>(
        crc32(0, reinterpret_cast<unsigned char const *>(file->getData()), lfh.uncompressed_size));

    // Let's allocate memory for compressed data
    data_buffer.resize(lfh.uncompressed_size);

    // Structure for data compression
    z_stream z_stream;
    memset(&z_stream, 0, sizeof(z_stream));
    deflateInit2(&z_stream, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);

    // Compressing data
    z_stream.avail_in  = lfh.uncompressed_size;
    z_stream.next_in   = reinterpret_cast<unsigned char *>(const_cast<int8_t *>(file->getData()));
    z_stream.avail_out = lfh.uncompressed_size;
    z_stream.next_out  = data_buffer.data();
    deflate(&z_stream, Z_FINISH);

    // Compressed data size
    char *   data_ptr = nullptr;
    uint32_t size     = 0;
    if(z_stream.total_out >= lfh.uncompressed_size)
    {
        lfh.compressed_size    = lfh.uncompressed_size;
        lfh.compression_method = 0;

        data_ptr = reinterpret_cast<char *>(const_cast<int8_t *>(file->getData()));
        size     = lfh.uncompressed_size;
    }
    else
    {
        lfh.compressed_size    = static_cast<uint32_t>(z_stream.total_out);
        lfh.compression_method = Z_DEFLATED;

        data_ptr = reinterpret_cast<char *>(data_buffer.data());
        size     = lfh.compressed_size;
    }

    // Cleaning
    deflateEnd(&z_stream);

    lfh.filename_length = static_cast<uint16_t>(file->getName().size());

    // Save the offset to the record to Local File Header
    uint32_t const lfh_offset = static_cast<uint32_t>(ofs.tellp());

    // Write a signature Local File Header
    lfh.signature         = 0x04034b50;
    lfh.modification_date = date;
    lfh.modification_time = time;
    // Write Local File Header
    ofs.write(reinterpret_cast<char *>(&lfh), sizeof(lfh));
    // Write filename
    ofs.write(file->getName().c_str(), static_cast<std::streamsize>(file->getName().size()));
    // Write data
    ofs.write(data_ptr, static_cast<std::streamsize>(size));

    uint32_t const first_offset_CDFH = static_cast<uint32_t>(ofs.tellp());

    ofs.write(central_directory_data.get(), static_cast<std::streamsize>(eocd.size_of_central_directory));

    CentralDirectoryFileHeader cdfh{};
    memset(&cdfh, 0, sizeof(cdfh));

    cdfh.compressed_size          = lfh.compressed_size;
    cdfh.uncompressed_size        = lfh.uncompressed_size;
    cdfh.compression_method       = lfh.compression_method;
    cdfh.crc32                    = lfh.crc32;
    cdfh.local_file_header_offset = lfh_offset;
    cdfh.filename_length          = lfh.filename_length;
    cdfh.signature                = 0x02014b50;
    cdfh.modification_date        = date;
    cdfh.modification_time        = time;

    ofs.write(reinterpret_cast<char *>(&cdfh), sizeof(cdfh));

    ofs.write(file->getName().c_str(), cdfh.filename_length);

    uint32_t const last_offset_CDFH = static_cast<uint32_t>(ofs.tellp());

    eocd.central_directory_offset        = first_offset_CDFH;
    eocd.number_central_directory_record = static_cast<uint16_t>(eocd.number_central_directory_record + 1);
    eocd.total_central_directory_record  = static_cast<uint16_t>(eocd.total_central_directory_record + 1);
    eocd.size_of_central_directory       = last_offset_CDFH - first_offset_CDFH;

    // Write a signature
    uint32_t const signature = 0x06054b50;
    ofs.write(reinterpret_cast<char const *>(&signature), sizeof(signature));

    // Write a EOCD
    ofs.write(reinterpret_cast<char *>(&eocd), sizeof(eocd));

    ofs.close();
    return true;
}

InFile FileSystem::loadRegularFile(file_data const & f) const
{
    std::ifstream ifs(m_data_dir + '/' + f.fname, std::ios::binary);
    if(!ifs.is_open())
    {
        std::stringstream ss;
        ss << "FileSystem::LoadRegularFile File: " << f.fname << " - not found";
        std::cout << ss.str() << std::endl;
        throw std::runtime_error("Unable to load file");
    }

    ifs.seekg(0, std::ios_base::end);
    size_t file_size = static_cast<size_t>(ifs.tellg());
    ifs.seekg(0, std::ios_base::beg);

    auto data = std::make_unique<int8_t[]>(file_size);

    ifs.read(reinterpret_cast<char *>(const_cast<int8_t *>(data.get())),
             static_cast<std::streamsize>(file_size));

    bool success = !ifs.fail() && file_size == static_cast<size_t>(ifs.gcount());
    if(!success)
    {
        std::stringstream ss;
        ss << "FileSystem::LoadRegularFile File: " << f.fname << " - not found";
        std::cout << ss.str() << std::endl;
        throw std::runtime_error("Unable to load file");
    }

    ifs.close();

    std::chrono::system_clock::time_point ftime =
        file_time_to_time_point(fs::last_write_time(m_data_dir + '/' + f.fname));

    return {f.fname, ftime, file_size, std::move(data)};
}

// http://blog2k.ru/archives/3392
InFile FileSystem::loadZipFile(file_data const & zf) const
{
    std::unique_ptr<int8_t[]> data;

    std::ifstream ifs(zf.fname, std::ifstream::binary);
    if(!ifs.is_open())
    {
        std::stringstream ss;
        ss << "FileSystem::LoadZipFile File: " << zf.fname << " - not found";
        std::cout << ss.str() << std::endl;
        throw std::runtime_error("Unable to load file");
    }

    ifs.seekg(zf.zip_data.lfh_offset, std::ifstream::beg);
    LocalFileHeader lfh{};
    ifs.read(reinterpret_cast<char *>(&lfh), sizeof(lfh));

    if(0x04034b50 != lfh.signature)
    {
        std::stringstream ss;
        ss << "FileSystem::LoadZipFile File: " << zf.fname << " - doesn't have zip file signature";
        std::cout << ss.str() << std::endl;
        throw std::runtime_error("Unable to load file");
    }

    ifs.seekg(lfh.filename_length, std::ifstream::cur);
    ifs.seekg(lfh.extra_field_length, std::ifstream::cur);

    auto read_buffer = std::make_unique<int8_t[]>(lfh.compressed_size);
    ifs.read(reinterpret_cast<char *>(read_buffer.get()), static_cast<std::streamsize>(lfh.compressed_size));

    if(!zf.zip_data.compressed)
    {
        data = std::move(read_buffer);
    }
    else
    {
        data = std::make_unique<int8_t[]>(lfh.uncompressed_size);

        z_stream zs;
        std::memset(&zs, 0, sizeof(zs));
        inflateInit2(&zs, -MAX_WBITS);

        zs.avail_in  = lfh.compressed_size;
        zs.next_in   = reinterpret_cast<unsigned char *>(read_buffer.get());
        zs.avail_out = lfh.uncompressed_size;
        zs.next_out  = reinterpret_cast<unsigned char *>(data.get());

        inflate(&zs, Z_FINISH);

        inflateEnd(&zs);
    }
    ifs.close();

    struct tm timeinfo;
    std::memset(&timeinfo, 0, sizeof(timeinfo));
    timeinfo.tm_year = ((lfh.modification_date & 0xFE00) >> 9) + 1980;
    timeinfo.tm_mon  = (lfh.modification_date & 0x01E0) >> 5;
    timeinfo.tm_mday = lfh.modification_date & 0x001F;

    timeinfo.tm_hour = (lfh.modification_time & 0xF800) >> 11;
    timeinfo.tm_min  = (lfh.modification_time & 0x07E0) >> 5;
    timeinfo.tm_sec  = (lfh.modification_time & 0x001f) * 2;

    auto   t        = tm_to_time_point(timeinfo);
    size_t unc_size = zf.zip_data.compressed ? lfh.uncompressed_size : lfh.compressed_size;

    return {zf.fname, t, unc_size, std::move(data)};
}

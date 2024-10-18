#include "file_system.h"
// #include "../core/exception.h"
// #include "../log/log.h"
#include <random>
#include <array>
#include <algorithm>
// #include <boost/filesystem.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <zlib.h>

#include "zip.h"

// https://medium.com/@sshambir/%D0%BF%D1%80%D0%B8%D0%B2%D0%B5%D1%82-std-filesystem-4c7ed50d5634
namespace evnt
{
namespace fs = std::filesystem;

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
    return generate_random_alphanumeric_string(10) + ".tmp";
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
        Log::Log(Log::error,
                 Log::cstr_log("FileSystem::AddZippedDir File: \"%s\" - not found", fname.c_str()));
        return;
    }

    // get length of file:
    ifs.seekg(0, std::ifstream::end);
    size_t const fileSize    = static_cast<size_t>(ifs.tellg());
    size_t       EOCD_offset = 0;

    for(size_t offset = fileSize - sizeof(EOCD); offset != 0; --offset)
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
        Log::Log(Log::error,
                 Log::cstr_log("FileSystem::AddZippedDir File: \"%s\" - not found EOCD_offset signature",
                               fname.c_str()));
        return;
    }

    EOCD eocd{};
    ifs.seekg(EOCD_offset, std::ifstream::beg);
    ifs.read(reinterpret_cast<char *>(&eocd), sizeof(eocd));

    ifs.seekg(eocd.centralDirectoryOffset, std::ifstream::beg);
    CentralDirectoryFileHeader cdfh{};

    for(uint16_t i = 0; i < eocd.numberCentralDirectoryRecord; ++i)
    {
        file_data zfile;
        zfile.is_zip = true;

        ifs.read(reinterpret_cast<char *>(&cdfh), sizeof(cdfh));

        if(0x02014b50 != cdfh.signature)
        {
            Log::Log(
                Log::error,
                Log::cstr_log(
                    "FileSystem::AddZippedDir File: \"%s\" - not found CentralDirectoryFileHeader signature",
                    fname.c_str()));
            return;
        }

        if(cdfh.generalPurposeBitFlag & 0x1)   // encrypted
        {
            Log::Log(Log::error,
                     Log::cstr_log("FileSystem::AddZippedDir File: \"%s\" - encrypted", fname.c_str()));
            return;
        }
        if(cdfh.generalPurposeBitFlag & 0x8)   // DataDescr Struct
        {
            Log::Log(Log::error, Log::cstr_log("FileSystem::AddZippedDir File: \"%s\" - DataDescr Struct",
                                               fname.c_str()));
            return;
        }

        if(cdfh.filenameLength)
        {
            std::unique_ptr<char[]> filename = std::make_unique<char[]>(cdfh.filenameLength + 1);
            ifs.read(reinterpret_cast<char *>(filename.get()), cdfh.filenameLength);

            filename[cdfh.filenameLength] = 0;

            zfile.zip_data.fname = std::string(filename.get());
        }

        if(cdfh.compressionMethod != 0 && cdfh.compressionMethod != Z_DEFLATED)
        {
            continue;
        }

        zfile.zip_data.compressed       = (Z_DEFLATED == cdfh.compressionMethod);
        zfile.zip_data.compressedSize   = cdfh.compressedSize;
        zfile.zip_data.uncompressedSize = cdfh.uncompressedSize;
        zfile.zip_data.lfhOffset        = cdfh.localFileHeaderOffset;

        if(zfile.zip_data.uncompressedSize != 0 || zfile.zip_data.compressed != 0)
        {
            zfile.fname = fname;
            m_files.emplace_back(std::move(zfile));
        }

        if(cdfh.extraFieldLength)
        {
            ifs.seekg(cdfh.extraFieldLength, std::ifstream::cur);
        }
        if(cdfh.fileCommentLength)
        {
            ifs.seekg(cdfh.fileCommentLength, std::ifstream::cur);
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

InFile FileSystem::getFile(std::string const & fname) const
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
        Log::Log(Log::warning, Log::cstr_log("FileSystem::GetFile File: \"%s\" - not found", fname.c_str()));
        EV_EXCEPT("File not found");
    }
}

bool FileSystem::writeFile(std::string const & path, BaseFile const * file)
{
    std::string filename = file->getName();

    if(!path.empty())
        filename = path + '/' + filename;

    std::ofstream ofs(std::string(m_data_dir + '/' + filename), std::ios::binary);
    if(!ofs.is_open())
    {
        Log::Log(Log::error,
                 Log::cstr_log("FileSystem::WriteFile File: \"%s\" - not writed", filename.c_str()));
        return false;
    }
    ofs.write(reinterpret_cast<char *>(const_cast<int8_t *>(file->getData())),
              static_cast<std::streamsize>(file->getFileSize()));
    ofs.close();

    if(!isExist(filename))
    {
        file_data fd;
        fd.fname = filename;

        m_files.emplace_back(std::move(fd));
    }

    return true;
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

// http://blog2k.ru/archives/3397
bool FileSystem::createZIP(std::vector<BaseFile const *> filelist, std::string const & zipname)
{
    // http://stackoverflow.com/questions/922360/why-cant-i-make-a-vector-of-references
    assert(!filelist.empty());
    assert(!zipname.empty());

    struct FileInfo
    {
        uint32_t compressedSize;
        uint32_t uncompressedSize;
        uint16_t compressionMethod;
        uint32_t crc32;
        uint32_t offset;
    };

    // Буфер для сжатых данных
    std::vector<uint8_t> dataBuffer;
    // Информация для создания Central directory file header
    std::vector<FileInfo> fileInfoList;

    std::ofstream ofs(std::string(m_data_dir + '/' + zipname), std::ofstream::binary | std::ofstream::trunc);
    if(!ofs.is_open())
    {
        return false;
    }

    uint16_t time = 0;
    uint16_t date = 0;

    for(size_t i = 0; i < filelist.size(); i++)
    {
        std::string       fname = filelist[i]->getName();
        std::vector<char> buf;
        buf.resize(filelist[i]->getFileSize());
        buf.assign(filelist[i]->getData(), filelist[i]->getData() + filelist[i]->getFileSize());

        LocalFileHeader lfh{};
        memset(&lfh, 0, sizeof(lfh));

        lfh.uncompressedSize = static_cast<uint32_t>(buf.size());

        lfh.crc32 = static_cast<uint32_t>(
            crc32(0, reinterpret_cast<unsigned char const *>(buf.data()), lfh.uncompressedSize));

        // Выделим память для сжатых данных
        dataBuffer.resize(lfh.uncompressedSize);

        // Структура для сжатия данных
        z_stream zStream;
        memset(&zStream, 0, sizeof(zStream));
        deflateInit2(&zStream, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);

        // Сжимаем данные
        zStream.avail_in  = lfh.uncompressedSize;
        zStream.next_in   = reinterpret_cast<unsigned char *>(buf.data());
        zStream.avail_out = lfh.uncompressedSize;
        zStream.next_out  = dataBuffer.data();
        deflate(&zStream, Z_FINISH);

        // Размер сжатых данных
        char *   data_ptr = nullptr;
        uint32_t size     = 0;
        if(zStream.total_out >= lfh.uncompressedSize)
        {
            lfh.compressedSize    = lfh.uncompressedSize;
            lfh.compressionMethod = 0;

            data_ptr = buf.data();
            size     = lfh.uncompressedSize;
        }
        else
        {
            lfh.compressedSize    = static_cast<uint32_t>(zStream.total_out);
            lfh.compressionMethod = Z_DEFLATED;

            data_ptr = reinterpret_cast<char *>(dataBuffer.data());
            size     = lfh.compressedSize;
        }

        // Очистка
        deflateEnd(&zStream);

        lfh.filenameLength = static_cast<uint16_t>(fname.size());

        // Сохраним смещение к записи Local File Header внутри архива
        uint32_t const lfhOffset = static_cast<uint32_t>(ofs.tellp());

        GetDosTime(filelist[i]->timeStamp(), time, date);

        // Запишем сигнатуру Local File Header
        lfh.signature        = 0x04034b50;
        lfh.modificationDate = date;
        lfh.modificationTime = time;
        // Запишем Local File Header
        ofs.write(reinterpret_cast<char *>(&lfh), sizeof(lfh));
        // Запишем имя файла
        ofs.write(fname.c_str(), static_cast<std::streamsize>(fname.size()));
        // Запишем данные
        ofs.write(data_ptr, static_cast<std::streamsize>(size));

        // Сохраним все данные для Central directory file header
        FileInfo fileInfo{};
        fileInfo.compressedSize    = lfh.compressedSize;
        fileInfo.uncompressedSize  = lfh.uncompressedSize;
        fileInfo.compressionMethod = lfh.compressionMethod;
        fileInfo.crc32             = lfh.crc32;
        fileInfo.offset            = lfhOffset;
        fileInfoList.push_back(fileInfo);
    }

    // Смещение первой записи для EOCD
    uint32_t const firstOffsetCDFH = static_cast<uint32_t>(ofs.tellp());

    for(uint32_t i = 0; i < filelist.size(); ++i)
    {
        std::string const &        filename = filelist[i]->getName();
        FileInfo const &           fileInfo = fileInfoList[i];
        CentralDirectoryFileHeader cdfh{};
        memset(&cdfh, 0, sizeof(cdfh));

        GetDosTime(filelist[i]->timeStamp(), time, date);

        cdfh.compressedSize        = fileInfo.compressedSize;
        cdfh.uncompressedSize      = fileInfo.uncompressedSize;
        cdfh.compressionMethod     = fileInfo.compressionMethod;
        cdfh.crc32                 = fileInfo.crc32;
        cdfh.localFileHeaderOffset = fileInfo.offset;
        cdfh.filenameLength        = static_cast<uint16_t>(filename.size());
        cdfh.signature             = 0x02014b50;
        cdfh.modificationDate      = date;
        cdfh.modificationTime      = time;

        // Запишем структуру
        ofs.write(reinterpret_cast<char *>(&cdfh), sizeof(cdfh));

        // Имя файла
        ofs.write(filename.c_str(), cdfh.filenameLength);
    }

    // Посчитаем размер данных для следующего шага
    uint32_t const lastOffsetCDFH = static_cast<uint32_t>(ofs.tellp());

    EOCD eocd{};
    memset(&eocd, 0, sizeof(eocd));
    eocd.centralDirectoryOffset       = firstOffsetCDFH;
    eocd.numberCentralDirectoryRecord = static_cast<uint16_t>(filelist.size());
    eocd.totalCentralDirectoryRecord  = static_cast<uint16_t>(filelist.size());
    eocd.sizeOfCentralDirectory       = lastOffsetCDFH - firstOffsetCDFH;

    // Пишем сигнатуру
    uint32_t signature = 0x06054b50;
    ofs.write(reinterpret_cast<char *>(&signature), sizeof(signature));

    // Пишем EOCD
    ofs.write(reinterpret_cast<char *>(&eocd), sizeof(eocd));

    ofs.close();
    return true;
}

bool FileSystem::addFileToZIP(BaseFile const * file, std::string const & zipname)
{
    assert(!zipname.empty());

    std::fstream ofs(m_data_dir + '/' + zipname,
                     std::fstream::binary | std::fstream::ate | std::fstream::out | std::fstream::in);
    if(!ofs.is_open())
    {
        return false;
    }

    size_t const fileSize     = static_cast<size_t>(ofs.tellg());
    size_t       EOCD_offset_ = 0;

    for(size_t offset = fileSize - sizeof(EOCD); offset != 0; --offset)
    {
        uint32_t signature = 0;

        ofs.seekg(offset, std::fstream::beg);
        ofs.read(reinterpret_cast<char *>(&signature), sizeof(signature));

        if(0x06054b50 == signature)
        {
            EOCD_offset_ = static_cast<size_t>(ofs.tellg());
            break;
        }
    }

    if(EOCD_offset_ == 0)
        return false;

    EOCD eocd{};
    ofs.seekg(EOCD_offset_, std::fstream::beg);
    ofs.read(reinterpret_cast<char *>(&eocd), sizeof(eocd));

    auto centralDirectoryData = std::make_unique<char[]>(eocd.sizeOfCentralDirectory);
    ofs.seekg(eocd.centralDirectoryOffset, std::fstream::beg);
    ofs.read(centralDirectoryData.get(), static_cast<std::streamsize>(eocd.sizeOfCentralDirectory));
    ofs.seekg(eocd.centralDirectoryOffset, std::fstream::beg);

    std::vector<std::string> zip_fnames;

    for(uint16_t i = 0; i < eocd.numberCentralDirectoryRecord; ++i)
    {
        CentralDirectoryFileHeader cdfh{};

        ofs.read(reinterpret_cast<char *>(&cdfh), sizeof(cdfh));

        if(0x02014b50 != cdfh.signature)
        {
            return false;
        }

        if(cdfh.filenameLength)
        {
            auto filename = std::make_unique<char[]>(cdfh.filenameLength + 1);
            ofs.read(static_cast<char *>(filename.get()), cdfh.filenameLength);

            filename[cdfh.filenameLength] = 0;

            zip_fnames.push_back(std::string(filename.get()));
        }
    }

    ofs.seekg(eocd.centralDirectoryOffset, std::fstream::beg);

    auto file_it = std::find_if(zip_fnames.begin(), zip_fnames.end(),
                                [&file](std::string const & zn) -> bool { return file->getName() == zn; });

    if(file_it != zip_fnames.end())
        return false;   // Если есть файл с таким именем

    uint16_t time = 0;
    uint16_t date = 0;
    GetDosTime(file->timeStamp(), time, date);

    LocalFileHeader lfh{};
    memset(&lfh, 0, sizeof(lfh));
    // Буфер для сжатых данных
    std::vector<uint8_t> dataBuffer;

    lfh.uncompressedSize = static_cast<uint32_t>(file->getFileSize());

    lfh.crc32 = static_cast<uint32_t>(
        crc32(0, reinterpret_cast<unsigned char const *>(file->getData()), lfh.uncompressedSize));

    // Выделим память для сжатых данных
    dataBuffer.resize(lfh.uncompressedSize);

    // Структура для сжатия данных
    z_stream zStream;
    memset(&zStream, 0, sizeof(zStream));
    deflateInit2(&zStream, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);

    // Сжимаем данные
    zStream.avail_in  = lfh.uncompressedSize;
    zStream.next_in   = reinterpret_cast<unsigned char *>(const_cast<int8_t *>(file->getData()));
    zStream.avail_out = lfh.uncompressedSize;
    zStream.next_out  = dataBuffer.data();
    deflate(&zStream, Z_FINISH);

    // Размер сжатых данных
    char *   data_ptr = nullptr;
    uint32_t size     = 0;
    if(zStream.total_out >= lfh.uncompressedSize)
    {
        lfh.compressedSize    = lfh.uncompressedSize;
        lfh.compressionMethod = 0;

        data_ptr = reinterpret_cast<char *>(const_cast<int8_t *>(file->getData()));
        size     = lfh.uncompressedSize;
    }
    else
    {
        lfh.compressedSize    = static_cast<uint32_t>(zStream.total_out);
        lfh.compressionMethod = Z_DEFLATED;

        data_ptr = reinterpret_cast<char *>(dataBuffer.data());
        size     = lfh.compressedSize;
    }

    // Очистка
    deflateEnd(&zStream);

    lfh.filenameLength = static_cast<uint16_t>(file->getName().size());

    // Сохраним смещение к записи Local File Header внутри архива
    uint32_t const lfhOffset = static_cast<uint32_t>(ofs.tellp());

    // Запишем сигнатуру Local File Header
    lfh.signature        = 0x04034b50;
    lfh.modificationDate = date;
    lfh.modificationTime = time;
    // Запишем Local File Header
    ofs.write(reinterpret_cast<char *>(&lfh), sizeof(lfh));
    // Запишем имя файла
    ofs.write(file->getName().c_str(), static_cast<std::streamsize>(file->getName().size()));
    // Запишем данные
    ofs.write(data_ptr, static_cast<std::streamsize>(size));

    uint32_t const firstOffsetCDFH = static_cast<uint32_t>(ofs.tellp());

    ofs.write(centralDirectoryData.get(), static_cast<std::streamsize>(eocd.sizeOfCentralDirectory));

    CentralDirectoryFileHeader cdfh{};
    memset(&cdfh, 0, sizeof(cdfh));

    cdfh.compressedSize        = lfh.compressedSize;
    cdfh.uncompressedSize      = lfh.uncompressedSize;
    cdfh.compressionMethod     = lfh.compressionMethod;
    cdfh.crc32                 = lfh.crc32;
    cdfh.localFileHeaderOffset = lfhOffset;
    cdfh.filenameLength        = lfh.filenameLength;
    cdfh.signature             = 0x02014b50;
    cdfh.modificationDate      = date;
    cdfh.modificationTime      = time;

    // Запишем структуру
    ofs.write(reinterpret_cast<char *>(&cdfh), sizeof(cdfh));

    // Имя файла
    ofs.write(file->getName().c_str(), cdfh.filenameLength);

    uint32_t const lastOffsetCDFH = static_cast<uint32_t>(ofs.tellp());

    eocd.centralDirectoryOffset       = firstOffsetCDFH;
    eocd.numberCentralDirectoryRecord = static_cast<uint16_t>(eocd.numberCentralDirectoryRecord + 1);
    eocd.totalCentralDirectoryRecord  = static_cast<uint16_t>(eocd.totalCentralDirectoryRecord + 1);
    eocd.sizeOfCentralDirectory       = lastOffsetCDFH - firstOffsetCDFH;

    // Пишем сигнатуру
    uint32_t const signature = 0x06054b50;
    ofs.write(reinterpret_cast<char const *>(&signature), sizeof(signature));

    // Пишем EOCD
    ofs.write(reinterpret_cast<char *>(&eocd), sizeof(eocd));

    ofs.close();
    return true;
}

// https://stackoverflow.com/questions/61030383/how-to-convert-stdfilesystemfile-time-type-to-time-t
template<typename TP>
std::time_t to_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
    return system_clock::to_time_t(sctp);
}

InFile FileSystem::loadRegularFile(file_data const & f) const
{
    std::ifstream ifs(m_data_dir + '/' + f.fname, std::ios::binary);
    if(!ifs.is_open())
    {
        Log::Log(Log::error,
                 Log::cstr_log("FileSystem::LoadRegularFile: \"%s\" - not found", f.fname.c_str()));
        EV_EXCEPT("Unable to load file");
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
        Log::Log(Log::error,
                 Log::cstr_log("FileSystem::LoadRegularFile: \"%s\" - not found", f.fname.c_str()));
        EV_EXCEPT("Unable to load file");
    }

    ifs.close();

    auto        ftime = fs::last_write_time(m_data_dir + '/' + f.fname);
    std::time_t time  = to_time_t(ftime);

    return {f.fname, time, file_size, std::move(data)};
}

// http://blog2k.ru/archives/3392
InFile FileSystem::loadZipFile(file_data const & zf) const
{
    std::unique_ptr<int8_t[]> data;

    std::ifstream ifs(zf.fname, std::ifstream::binary);
    if(!ifs.is_open())
    {
        Log::Log(Log::error, Log::cstr_log("FileSystem::LoadZipFile: \"%s\" - not found", zf.fname.c_str()));
        EV_EXCEPT("Unable to load file");
    }

    ifs.seekg(zf.zip_data.lfhOffset, std::ifstream::beg);
    LocalFileHeader lfh{};
    ifs.read(reinterpret_cast<char *>(&lfh), sizeof(lfh));

    if(0x04034b50 != lfh.signature)
    {
        Log::Log(Log::error,
                 Log::cstr_log("FileSystem::LoadZipFile: \"%s\" - doesn't have zip file signature",
                               zf.fname.c_str()));
        EV_EXCEPT("Unable to load file");
    }

    ifs.seekg(lfh.filenameLength, std::ifstream::cur);
    ifs.seekg(lfh.extraFieldLength, std::ifstream::cur);

    auto readBuffer = std::make_unique<int8_t[]>(lfh.compressedSize);
    ifs.read(reinterpret_cast<char *>(readBuffer.get()), static_cast<std::streamsize>(lfh.compressedSize));

    if(!zf.zip_data.compressed)
    {
        data = std::move(readBuffer);
    }
    else
    {
        data = std::make_unique<int8_t[]>(lfh.uncompressedSize);

        z_stream zs;
        std::memset(&zs, 0, sizeof(zs));
        inflateInit2(&zs, -MAX_WBITS);

        zs.avail_in  = lfh.compressedSize;
        zs.next_in   = reinterpret_cast<unsigned char *>(readBuffer.get());
        zs.avail_out = lfh.uncompressedSize;
        zs.next_out  = reinterpret_cast<unsigned char *>(data.get());

        inflate(&zs, Z_FINISH);

        inflateEnd(&zs);
    }
    ifs.close();

    struct tm timeinfo;
    std::memset(&timeinfo, 0, sizeof(timeinfo));
    timeinfo.tm_year = ((lfh.modificationDate & 0xFE00) >> 9) + 1980;
    timeinfo.tm_mon  = (lfh.modificationDate & 0x01E0) >> 5;
    timeinfo.tm_mday = lfh.modificationDate & 0x001F;

    timeinfo.tm_hour = (lfh.modificationTime & 0xF800) >> 11;
    timeinfo.tm_min  = (lfh.modificationTime & 0x07E0) >> 5;
    timeinfo.tm_sec  = (lfh.modificationTime & 0x001f) * 2;

    std::time_t t        = std::mktime(&timeinfo);
    size_t      unc_size = zf.zip_data.compressed ? lfh.uncompressedSize : lfh.compressedSize;

    return {zf.fname, t, unc_size, std::move(data)};
}
}   // namespace evnt

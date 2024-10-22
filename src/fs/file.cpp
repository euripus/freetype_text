#include "file.h"
#include "file_system.h"
#include <filesystem>

std::string BaseFile::getNameExt() const
{
    return std::filesystem::path(m_name).extension().string();
}

OutFile::OutFile()
{
    m_name            = FileSystem::GetTempFileName();
    m_last_write_time = std::chrono::system_clock::now();
}

OutFile::OutFile(std::string name)
{
    m_name            = std::move(name);
    m_last_write_time = std::chrono::system_clock::now();
}

OutFile::OutFile(std::string name, char const * data, size_t length)
    : OutFile(std::move(name))
{
    m_data.write(reinterpret_cast<int8_t const *>(data), length);
}

OutFile::OutFile(InFile const & infile)
    : OutFile{infile.getName(), reinterpret_cast<char const *>(infile.getData()), infile.getFileSize()}
{}

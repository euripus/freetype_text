#ifndef FILE_H
#define FILE_H

#include "memory_stream.h"
#include <chrono>

class BaseFile
{
public:
    BaseFile()          = default;
    virtual ~BaseFile() = default;

    virtual int8_t const * getData() const     = 0;
    virtual size_t         getFileSize() const = 0;

    std::chrono::system_clock::time_point timeStamp() const { return m_last_write_time; }
    std::string                           getName() const { return m_name; }
    std::string                           getNameExt() const;
    bool                                  empty() const { return getFileSize() > 0; }

protected:
    std::chrono::system_clock::time_point m_last_write_time;
    std::string                           m_name;
};

class InFile;

class OutFile : public BaseFile
{
public:
    OutFile();   // file for writing with random name
    OutFile(std::string name);
    OutFile(std::string name, char const * data, size_t length);
    explicit OutFile(InFile const & infile);
    ~OutFile() override = default;

    int8_t const * getData() const override { return m_data.getBufferPtr(); }
    size_t         getFileSize() const override { return m_data.getLength(); }

    void writeTimeNow()   // change write time of file
    {
        m_last_write_time = std::chrono::system_clock::now();
    }

    OutputMemoryStream &       getStream() { return m_data; }
    OutputMemoryStream const & getStream() const { return m_data; }

private:
    OutputMemoryStream m_data;
};

class InFile : public BaseFile
{
public:
    InFile(size_t f_size)
        : m_data(f_size)
    {
        m_name            = FileSystem::GetTempFileName();
        m_last_write_time = std::chrono::system_clock::now();
    }

    InFile(std::string name, std::chrono::system_clock::time_point timestamp, size_t f_size,
           std::unique_ptr<int8_t[]> data)
        : m_data(std::move(data), f_size)
    {
        std::swap(m_name, name);
        m_last_write_time = timestamp;
    }

    explicit InFile(OutFile const & outfile)
    {
        m_name            = outfile.getName();
        m_last_write_time = outfile.timeStamp();
        m_data            = InputMemoryStream(outfile.getStream());
    }

    ~InFile() override = default;

    int8_t const * getData() const override { return m_data.getPtr(); }
    size_t         getFileSize() const override { return m_data.getCapacity(); }

    InputMemoryStream & getStream() { return m_data; }

private:
    InputMemoryStream m_data;
};
#endif   // FILE_H

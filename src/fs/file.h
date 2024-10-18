#ifndef FILE_H
#define FILE_H

#include "memory_stream.h"
#include <ctime>

namespace evnt
{
class BaseFile
{
public:
    BaseFile()          = default;
    virtual ~BaseFile() = default;

    virtual int8_t const * getData() const     = 0;
    virtual size_t         getFileSize() const = 0;

    std::time_t timeStamp() const { return m_last_write_time; }
    std::string getName() const { return m_name; }
    std::string getNameExt() const;

protected:
    std::time_t m_last_write_time = 0;
    std::string m_name;
};

class InFile;

class OutFile : public BaseFile
{
public:
    OutFile();   // file for writing with random name
    OutFile(std::string name);
    OutFile(std::string name, char const * data, size_t length);
    OutFile(InFile const & infile);
    ~OutFile() override = default;

    int8_t const * getData() const override { return m_data.getBufferPtr(); }
    size_t         getFileSize() const override { return m_data.getLength(); }

    void                 write(char const * buffer, size_t len);   // change write time of file
    OutputMemoryStream & getStream() { return m_data; }

private:
    OutputMemoryStream m_data;
};

class InFile : public BaseFile
{
public:
    InFile(std::string name, std::time_t timestamp, size_t f_size, std::unique_ptr<int8_t[]> data)
        : m_data(std::move(data), f_size)
    {
        std::swap(m_name, name);
        m_last_write_time = timestamp;
    }
    InFile(OutFile const & outfile)
    {
        m_name            = outfile.getName();
        m_last_write_time = outfile.timeStamp();

        std::unique_ptr<int8_t[]> new_buf = std::make_unique<int8_t[]>(outfile.getFileSize());
        std::memcpy(new_buf.get(), outfile.getData(), outfile.getFileSize());
        m_data = InputMemoryStream(std::move(new_buf), outfile.getFileSize());
    }
    ~InFile() override = default;

    int8_t const * getData() const override { return m_data.getPtr(); }
    size_t         getFileSize() const override { return m_data.getCapacity(); }

    InputMemoryStream & getStream() { return m_data; }

private:
    InputMemoryStream m_data;
};
}   // namespace evnt
#endif   // FILE_H

#ifndef MEMORYSTREAM_H
#define MEMORYSTREAM_H

#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

class InputMemoryStream;

class OutputMemoryStream
{
public:
    OutputMemoryStream()  = default;
    ~OutputMemoryStream() = default;

    OutputMemoryStream(OutputMemoryStream const & other) { m_buffer = other.m_buffer; }
    OutputMemoryStream & operator=(OutputMemoryStream const & other)
    {
        // check for self-assignment
        if(&other == this)
            return *this;

        m_buffer = other.m_buffer;

        return *this;
    }

    OutputMemoryStream(OutputMemoryStream && other)
        : OutputMemoryStream()
    {
        std::swap(*this, other);
    }

    OutputMemoryStream & operator=(OutputMemoryStream && other)
    {
        // check for self-assignment
        if(&other == this)
            return *this;

        std::swap(*this, other);

        return *this;
    }

    friend void swap(OutputMemoryStream & left, OutputMemoryStream & right)
    {
        using std::swap;

        swap(left.m_buffer, right.m_buffer);
    }

    int8_t const * getBufferPtr() const { return m_buffer.data(); }
    size_t         getLength() const { return m_buffer.size(); }

    void write(int8_t const * data, size_t byte_count);

    template<typename T>
    void write(T & data)
    {
        // https://stackoverflow.com/questions/48225673/why-is-stdis-pod-deprecated-in-c20
        static_assert(std::is_standard_layout_v<T>, "Generic Write only supports primitive data types");

        write(reinterpret_cast<int8_t const *>(&data), sizeof(data));
    }

    void write(InputMemoryStream const & stream);

    void clear() { m_buffer.clear(); }

private:
    std::vector<int8_t> m_buffer;
};

class InputMemoryStream
{
public:
    InputMemoryStream(std::unique_ptr<int8_t[]> data, size_t byte_count)
        : mup_data{std::move(data)},
          m_head{0},
          m_capacity{byte_count}
    {}
    explicit InputMemoryStream(size_t byte_count)
        : mup_data{std::make_unique<int8_t[]>(byte_count)},
          m_head{0},
          m_capacity{byte_count}
    {
        std::memset(mup_data.get(), 0, byte_count);
    }
    InputMemoryStream()  = default;
    ~InputMemoryStream() = default;

    InputMemoryStream(InputMemoryStream const & other)
        : m_head{other.m_head},
          m_capacity{other.m_capacity},
          m_eof{other.m_eof}
    {
        mup_data = std::make_unique<int8_t[]>(m_capacity);
        std::memcpy(mup_data.get(), other.mup_data.get(), m_capacity);
    }

    explicit InputMemoryStream(OutputMemoryStream const & stream)
        : m_head{0},
          m_capacity{stream.getLength()}
    {
        std::unique_ptr<int8_t[]> new_buf = std::make_unique<int8_t[]>(m_capacity);
        std::memcpy(new_buf.get(), stream.getBufferPtr(), stream.getLength());
        std::swap(mup_data, new_buf);
    }

    InputMemoryStream & operator=(InputMemoryStream const & other)
    {
        // check for self-assignment
        if(&other == this)
            return *this;

        m_head     = other.m_head;
        m_capacity = other.m_capacity;
        m_eof      = other.m_eof;
        mup_data   = std::make_unique<int8_t[]>(m_capacity);
        std::memcpy(mup_data.get(), other.mup_data.get(), m_capacity);

        return *this;
    }

    InputMemoryStream(InputMemoryStream && other)
        : InputMemoryStream()
    {
        std::swap(*this, other);
    }

    InputMemoryStream & operator=(InputMemoryStream && other)
    {
        // check for self-assignment
        if(&other == this)
            return *this;

        std::swap(*this, other);

        return *this;
    }

    friend void swap(InputMemoryStream & left, InputMemoryStream & right)
    {
        using std::swap;

        swap(left.mup_data, right.mup_data);
        swap(left.m_head, right.m_head);
        swap(left.m_capacity, right.m_capacity);
        swap(left.m_eof, right.m_eof);
    }

    bool read(void * out_data, size_t byte_count) const;

    template<typename T>
    bool read(T & out_data) const
    {
        static_assert(std::is_standard_layout_v<T>, "Generic Read only supports primitive data types");
        return read(reinterpret_cast<void *>(&out_data), sizeof(out_data));
    }

    size_t         getRemainingDataSize() const { return m_capacity - m_head; }
    size_t         getCapacity() const { return m_capacity; }
    int8_t const * getCurPosPtr() const { return mup_data.get() + m_head; }
    int8_t const * getPtr() const { return mup_data.get(); }
    explicit       operator bool() const { return !m_eof; }

    void resetHead()
    {
        m_head = 0;
        m_eof  = false;
    }

    void setCapacity(uint32_t new_capacity) { m_capacity = new_capacity; }

private:
    std::unique_ptr<int8_t[]> mup_data;
    mutable size_t            m_head;
    size_t                    m_capacity;
    mutable bool              m_eof = false;
};

InputMemoryStream const & GetLine(InputMemoryStream const & input_stream, std::string & out_str);

#endif   // MEMORYSTREAM_H

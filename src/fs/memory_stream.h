#ifndef MEMORYSTREAM_H
#define MEMORYSTREAM_H

#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace evnt
{
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

    void write(int8_t const * inData, size_t inByteCount);

    template<typename T>
    void write(T & inData)
    {
        // https://stackoverflow.com/questions/48225673/why-is-stdis-pod-deprecated-in-c20
        static_assert(std::is_standard_layout_v<T>, "Generic Write only supports primitive data types");

        write(reinterpret_cast<int8_t const *>(&inData), sizeof(inData));
    }

    void write(std::string const & inString)
    {
        size_t elementCount = inString.size();
        write(elementCount);
        write(reinterpret_cast<int8_t const *>(inString.data()), elementCount * sizeof(char));
    }

    void write(InputMemoryStream const & inStream);

    void clear() { m_buffer.clear(); }

private:
    std::vector<int8_t> m_buffer;
};

class InputMemoryStream
{
public:
    InputMemoryStream(std::unique_ptr<int8_t[]> inData, size_t inByteCount)
        : mup_data{std::move(inData)},
          m_head{0},
          m_capacity{inByteCount}
    {}
    InputMemoryStream(size_t inByteCount)
        : mup_data{std::make_unique<int8_t[]>(inByteCount)},
          m_head{0},
          m_capacity{inByteCount}
    {}
    InputMemoryStream()  = default;
    ~InputMemoryStream() = default;

    InputMemoryStream(InputMemoryStream const & other)
        : m_head{other.m_head},
          m_capacity{other.m_capacity}
    {
        mup_data = std::make_unique<int8_t[]>(m_capacity);
        std::memcpy(mup_data.get(), other.mup_data.get(), m_capacity);
    }
    InputMemoryStream & operator=(InputMemoryStream const & other)
    {
        // check for self-assignment
        if(&other == this)
            return *this;

        m_head     = other.m_head;
        m_capacity = other.m_capacity;
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
    }

    void read(void * outData, size_t inByteCount) const;

    template<typename T>
    void read(T & outData) const
    {
        static_assert(std::is_standard_layout_v<T>, "Generic Read only supports primitive data types");
        read(reinterpret_cast<void *>(&outData), sizeof(outData));
    }

    void read(std::string & inString) const
    {
        inString.clear();
        uint32_t elementCount;
        read(&elementCount, sizeof(uint32_t));
        inString.resize(elementCount);
        read(inString.data(), elementCount * sizeof(char));
    }

    size_t         getRemainingDataSize() const { return m_capacity - m_head; }
    size_t         getCapacity() const { return m_capacity; }
    int8_t const * getCurPosPtr() const { return mup_data.get() + m_head; }
    int8_t const * getPtr() const { return mup_data.get(); }

    void resetHead() { m_head = 0; }
    void setCapacity(uint32_t newCapacity) { m_capacity = newCapacity; }

    static InputMemoryStream ConvertToInputMemoryStream(OutputMemoryStream const & inStream);

private:
    std::unique_ptr<int8_t[]> mup_data;
    mutable size_t            m_head;
    size_t                    m_capacity;
};
}   // namespace evnt

#endif   // MEMORYSTREAM_H

#include "memory_stream.h"

#include <cstring>
#include <stdexcept>

namespace evnt
{
void OutputMemoryStream::write(int8_t const * inData, size_t inByteCount)
{
    m_buffer.reserve(m_buffer.size() + inByteCount);
    m_buffer.insert(std::end(m_buffer), inData, inData + inByteCount);
}

void OutputMemoryStream::write(InputMemoryStream const & inStream)
{
    m_buffer.reserve(m_buffer.size() + inStream.getCapacity());
    m_buffer.insert(std::end(m_buffer), inStream.getPtr(), inStream.getPtr() + inStream.getCapacity());
}

void InputMemoryStream::read(void * outData, size_t inByteCount) const
{
    size_t resultHead = m_head + inByteCount;
    if(resultHead > m_capacity)
    {
        throw std::range_error("InputMemoryStream::Read - no data to read!");
    }

    std::memcpy(outData, mup_data.get() + m_head, inByteCount);

    m_head = resultHead;
}

InputMemoryStream InputMemoryStream::ConvertToInputMemoryStream(OutputMemoryStream const & inStream)
{
    std::unique_ptr<int8_t[]> new_buf = std::make_unique<int8_t[]>(inStream.getLength());
    std::memcpy(new_buf.get(), inStream.getBufferPtr(), inStream.getLength());
    InputMemoryStream temp(std::move(new_buf), inStream.getLength());

    return temp;
}
}   // namespace evnt

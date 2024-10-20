#include "memory_stream.h"
#include <cstring>

namespace evnt
{
void OutputMemoryStream::write(int8_t const * data, size_t byte_count)
{
    m_buffer.reserve(m_buffer.size() + byte_count);
    m_buffer.insert(std::end(m_buffer), data, data + byte_count);
}

void OutputMemoryStream::write(InputMemoryStream const & stream)
{
    m_buffer.reserve(m_buffer.size() + stream.getCapacity());
    m_buffer.insert(std::end(m_buffer), stream.getPtr(), stream.getPtr() + stream.getCapacity());
}

bool InputMemoryStream::read(void * out_data, size_t byte_count) const
{
    size_t result_head = m_head + byte_count;
    if(result_head > m_capacity)
    {
        if(static_cast<int>(m_capacity - m_head) > 0)
            std::memcpy(out_data, mup_data.get() + m_head, m_capacity - m_head);
        m_head = m_capacity;
        m_eof  = true;
        return false;
    }

    std::memcpy(out_data, mup_data.get() + m_head, byte_count);

    m_head = result_head;
    return true;
}

InputMemoryStream & GetLine(InputMemoryStream & input_stream, std::string & out_str)
{
    char ch;
    out_str.clear();
    while(input_stream.read(ch) && ch != '\n')
        out_str.push_back(ch);
    return input_stream;
}

}   // namespace evnt

#include "basic_types.h"

VertexBuffer & ColorMap::GetOrCreate(ColoredTextBuffers & buffer, glm::vec4 const & color)
{
    auto [it, inserted] = buffer.try_emplace(color, VertexBuffer::pos_tex, 1);

    return it->second;
}

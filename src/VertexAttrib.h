#ifndef VERTEXATTRIB_H
#define VERTEXATTRIB_H

#include <cstdint>
#include <string>

struct VertexAttrib
{
    std::string   name;
    std::uint32_t location;
    size_t        size;     // num of components
    size_t        stride;   // vertex size
    size_t        pointer;

    VertexAttrib()
        : location(0),
          size(0),
          stride(0),
          pointer(0)
    {}

    static VertexAttrib VertexAttribParse(char const * format);
    void                VertexAttribEnable() const;
    void                VertexAttribDisable() const;
};

#endif   // VERTEXATTRIB_H

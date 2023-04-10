#include "VertexAttrib.h"
#include <GL/glew.h>
#include <cassert>
#include <cstring>
#include <stdio.h>

VertexAttrib VertexAttrib::VertexAttribParse(char const * format)
{
    assert(format);

    VertexAttrib attr;

    char const * p = std::strchr(format, ':');
    if(p != NULL)
    {
        attr.name = std::string(format, p - format);
        if(*(++p) == '\0')
        {
            fprintf(stderr, "No size specified for '%s' attribute\n", attr.name.c_str());
        }
        attr.size = *p - '0';
    }
    else
    {
        fprintf(stderr, "Vertex attribute format not understood ('%s')\n", format);
    }

    return attr;
}

void VertexAttrib::VertexAttribEnable() const
{
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, size, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(pointer));
}

void VertexAttrib::VertexAttribDisable() const
{
    glDisableVertexAttribArray(location);
}

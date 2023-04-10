#ifndef ATLASTEX_H
#define ATLASTEX_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

class AtlasTex
{
public:
    AtlasTex() = default;

    void Create(unsigned int width, unsigned int height);
    void Delete();
    void Clear();

    glm::ivec4 GetRegion(unsigned int width, unsigned int height);
    void       SetRegion(glm::ivec4 reg, unsigned char const * data, int stride);   // z - width, w - height

    unsigned int GetWidth() const { return _width; }
    unsigned int GetHeight() const { return _height; }

    void UploadTexture();
    void DeleteTexture();
    void BindTexture();

    void WriteAtlasToTGA(std::string const & name);

protected:
    int  AtlasFit(unsigned int index, unsigned int width, unsigned int height);
    void AtlasMerge();

private:
    unsigned int               _width  = 0;
    unsigned int               _height = 0;
    std::vector<unsigned char> _data;

    unsigned int            _used = 0;
    std::vector<glm::ivec3> _nodes;
};

#endif   // ATLASTEX_H

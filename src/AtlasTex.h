#ifndef ATLASTEX_H
#define ATLASTEX_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

class AtlasTex
{
public:
    AtlasTex();
    virtual ~AtlasTex();

    void Create(unsigned int width, unsigned int height, unsigned int depth);
    void Delete();
    void Clear();

    glm::ivec4 GetRegion(unsigned int width, unsigned int height);
    void       SetRegion(glm::ivec4 reg, const unsigned char * data, int stride);   // z - width, w - height

    unsigned int GetWidth() const { return _width; }
    unsigned int GetHeight() const { return _height; }
    unsigned int GetDepth() const { return _depth; }

    void UploadTexture();
    void DeleteTexture();
    void BindTexture();

    void WriteAtlasToTGA(const std::string & name);

protected:
    int  AtlasFit(unsigned int index, unsigned int width, unsigned int height);
    void AtlasMerge();

private:
    unsigned int    _width;
    unsigned int    _height;
    unsigned int    _depth;
    unsigned char * _data;

    unsigned int            _used;
    std::vector<glm::ivec3> _nodes;
};

#endif   // ATLASTEX_H

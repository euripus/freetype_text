#ifndef ATLASTEX_H
#define ATLASTEX_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

class AtlasTex
{
public:
    AtlasTex(unsigned int size = 64);

    void Clear();

    glm::ivec4 GetRegion(unsigned int width, unsigned int height);
    void       SetRegion(glm::ivec4 reg, unsigned char const * data, int stride);   // z - width, w - height

    unsigned int GetSize() const { return _size; }

    void UploadTexture();
    void DeleteTexture();
    void BindTexture();

    void WriteAtlasToTGA(std::string const & name);

protected:
    int  AtlasFit(unsigned int index, unsigned int width, unsigned int height);
    void AtlasMerge();

private:
	unsigned int               _size = 0;
    std::vector<unsigned char> _data;

    unsigned int            _used = 0;
    std::vector<glm::ivec3> _nodes;
};

#endif   // ATLASTEX_H

#ifndef ATLASTEX_H
#define ATLASTEX_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

class AtlasTex
{
public:
    AtlasTex(unsigned int size = 64);

    void clear();

    glm::ivec4 getRegion(unsigned int width, unsigned int height);
    void       setRegion(glm::ivec4 reg, unsigned char const * data, int stride);   // z - width, w - height

    unsigned int getSize() const { return m_size; }

    void UploadTexture();
    void DeleteTexture();
    void BindTexture();

    void WriteAtlasToTGA(std::string const & name);

protected:
    int  atlasFit(unsigned int index, unsigned int width, unsigned int height);
    void atlasMerge();

private:
    unsigned int               m_size = 0;
    std::vector<unsigned char> m_data;
    std::vector<glm::ivec3>    m_nodes;
};

#endif   // ATLASTEX_H

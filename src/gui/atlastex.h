#ifndef ATLASTEX_H
#define ATLASTEX_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

class AtlasTex
{
public:
    AtlasTex(unsigned int size = 512);

    void clear();

    glm::ivec4 getRegion(unsigned int width, unsigned int height);
    void       setRegionTL(glm::ivec4 reg, unsigned char const * data,
                           int stride);   // z - width, w - height, top-left region
    void       setRegionBL(glm::ivec4 reg, unsigned char const * data,
                           int stride);   // z - width, w - height, bottom-left region

    unsigned int          getSize() const { return m_size; }
    unsigned char const * getData() const { return m_data.data(); }

    void writeAtlasToTGA(std::string const & name);

    void UploadTexture();
    void DeleteTexture();
    void BindTexture();
private:
    int  atlasFit(unsigned int index, unsigned int width, unsigned int height);
    void atlasMerge();

    unsigned int               m_size = 0;
    std::vector<unsigned char> m_data;
    std::vector<glm::ivec3>    m_nodes;
};

#endif   // ATLASTEX_H

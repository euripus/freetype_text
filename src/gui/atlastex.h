#ifndef ATLASTEX_H
#define ATLASTEX_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

class AtlasTex
{
public:
    AtlasTex(uint32_t size = 64);

    void clear();

    glm::ivec4 getRegion(uint32_t width, uint32_t height);
    void       setRegionTL(glm::ivec4 reg, unsigned char const * data, int32_t stride,
                           int32_t bytes_ppx = 3);   // z - width, w - height, top-left region
    void       setRegionBL(glm::ivec4 reg, unsigned char const * data, int32_t stride,
                           int32_t bytes_ppx = 3);   // z - width, w - height, bottom-left region

    uint32_t              getSize() const { return m_size; }
    unsigned char const * getData() const { return m_data.data(); }

    void writeAtlasToTGA(std::string const & name);

    void UploadTexture();
    void DeleteTexture();
    void BindTexture();

private:
    int32_t atlasFit(uint32_t index, uint32_t width, uint32_t height);
    void    atlasMerge();

    uint32_t                   m_size = 0;
    std::vector<unsigned char> m_data;
    std::vector<glm::ivec3>    m_nodes;
	
	uint32_t m_atlas_tex_id = 0;
};

#endif   // ATLASTEX_H

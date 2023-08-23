#include "fontmanager.h"
#include <stdexcept>

TexFont & FontManager::addFont(FontDataDesc const & desc)
{
    std::string hash_string = desc.filename + ' ' + std::to_string(static_cast<uint32_t>(desc.pt_size));
    std::size_t hash_val    = std::hash<std::string>{}(hash_string);

    if(auto search = m_fonts.find(hash_val); search != m_fonts.end())
        return *search->second.get();   // font already loaded

    m_fonts[hash_val] = std::make_unique<TexFont>(*this, desc.filename, desc.pt_size, desc.hinting,
                                                  desc.kerning, desc.outline_thickness, desc.outline_type);

    return *m_fonts[hash_val].get();
}

TexFont & FontManager::getFont(std::string name, uint32_t size)
{
    std::string hash_string = name + ' ' + std::to_string(size);
    std::size_t hash_val    = std::hash<std::string>{}(hash_string);

    if(auto search = m_fonts.find(hash_val); search != m_fonts.end())
        return *search->second.get();
    else
    {
        std::string error_msg = "Font not found: " + hash_string;
        throw std::runtime_error(error_msg);
    }
}

void FontManager::resizeAtlas()
{
    AtlasTex new_atlas(m_atlas.getSize() * 2);
    m_atlas = std::move(new_atlas);

    for(auto & fnt : m_fonts)
    {
        fnt.second->reloadGlyphs();
    }
}

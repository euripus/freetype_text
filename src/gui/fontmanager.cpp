#include "fontmanager.h"

TexFont & FontManager::addFont(FontDataDesc const & desc)
{
    std::string hash_string = desc.font_id + ' ' + std::to_string(static_cast<uint32_t>(desc.pt_size));
    std::size_t hash_val    = std::hash<std::string>{}(hash_string);

    if(auto search = m_fonts.find(hash_val); search != m_fonts.end())
        return *search->second.get();   // font already loaded

    if(auto file = m_file_sys.getFile(desc.filename); file)
    {
        m_fonts[hash_val] =
            std::make_unique<TexFont>(*this, file->getData(), file->getFileSize(), desc.pt_size, desc.hinting,
                                      desc.kerning, desc.outline_thickness, desc.outline_type);
    }
    else
    {
        std::stringstream ss;
        ss << "FontManager::addFont File: " << desc.filename << " - not found";
        throw std::rutime_error(ss.str());
    }

    return *m_fonts[hash_val].get();
}

TexFont * FontManager::getFont(std::string name, uint32_t size)
{
    std::string hash_string = name + ' ' + std::to_string(size);
    std::size_t hash_val    = std::hash<std::string>{}(hash_string);

    if(auto search = m_fonts.find(hash_val); search != m_fonts.end())
        return search->second.get();
    else
    {
        return nullptr;
    }
}

void FontManager::resizeAtlas()
{
    AtlasTex new_atlas(m_atlas.getSize() * 2);
    m_atlas = std::move(new_atlas);

    for(auto & fnt: m_fonts)
    {
        fnt.second->reloadGlyphs();
    }
}

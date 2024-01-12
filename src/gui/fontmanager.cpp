#include "fontmanager.h"
#include <fstream>
#include <boost/json.hpp>
#include <stdexcept>

Glyph::OutlineType FontManager::GetOutlineTypeFromString(std::string_view str_outline)
{
    if(str_outline == "NONE")
        return Glyph::OutlineType::NONE;
    else if(str_outline == "LINE")
        return Glyph::OutlineType::LINE;
    else if(str_outline == "INNER")
        return Glyph::OutlineType::INNER;
    else if(str_outline == "OUTER")
        return Glyph::OutlineType::OUTER;

    return Glyph::OutlineType::NONE;
}

void FontManager::parseFontsRes(std::string const & file_name)
{
    boost::json::value jv;

    try
    {
        std::ifstream ifile(file_name, std::ios::in);
        std::string   file_data;

        if(ifile.is_open())
        {
            std::string tp;
            while(std::getline(ifile, tp))
            {
                file_data += tp;
            }
        }
        else
        {
            std::string err = "File: " + file_name + " - not found!";
            throw std::runtime_error(err);
        }

        jv = boost::json::parse(file_data);
    }
    catch(std::exception const & e)
    {
        throw std::runtime_error(e.what());
    }

    assert(!jv.is_null());

    auto const & obj          = jv.get_object();
    auto const   fonts_set_it = obj.find(sid_fonts);
    if(fonts_set_it != obj.end())
    {
        auto const & arr = fonts_set_it->value().as_array();
        if(!arr.empty())
        {
            for(auto const & font_entry: arr)
            {
                auto const & font_obj = font_entry.as_object();
                FontDataDesc desc;
                std::string  glyphs;

                for(auto const & kvp: font_obj)
                {
                    if(kvp.key() == sid_file_name)
                    {
                        desc.filename = kvp.value().as_string();
                    }
                    else if(kvp.key() == sid_font_id)
                    {
                        desc.font_id = kvp.value().as_string();
                    }
                    else if(kvp.key() == sid_hinting)
                    {
                        desc.hinting = kvp.value().as_bool();
                    }
                    else if(kvp.key() == sid_kerning)
                    {
                        desc.kerning = kvp.value().as_bool();
                    }
                    else if(kvp.key() == sid_outline_thickness)
                    {
                        desc.outline_thickness = kvp.value().as_double();
                    }
                    else if(kvp.key() == sid_outline_type)
                    {
                        desc.outline_type = GetOutlineTypeFromString(kvp.value().as_string());
                    }
                    else if(kvp.key() == sid_font_size)
                    {
                        desc.pt_size = kvp.value().as_int64();
                    }
                    else if(kvp.key() == sid_glyphs)
                    {
                        glyphs = kvp.value().as_string();
                    }
                    else
                    {
                        std::string error =
                            "Unknown parameter: " + std::string(kvp.key()) + " in file: " + file_name;
                        throw std::runtime_error(error);
                    }
                }

                auto & fnt = addFont(desc);
                fnt.cacheGlyphs(glyphs.c_str());
            }
        }
    }
}

TexFont & FontManager::addFont(FontDataDesc const & desc)
{
    std::string hash_string = desc.font_id + ' ' + std::to_string(static_cast<uint32_t>(desc.pt_size));
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

    for(auto & fnt: m_fonts)
    {
        fnt.second->reloadGlyphs();
    }
}

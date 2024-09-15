#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include "texfont.h"
#include "atlastex.h"
#include <map>
#include <memory>

struct FontDataDesc
{
    std::string        filename;
    std::string        font_id;
    float              pt_size           = 24.0f;
    bool               hinting           = true;
    bool               kerning           = true;
    float              outline_thickness = 0.0f;
    Glyph::OutlineType outline_type      = Glyph::OutlineType::NONE;
};

class FontManager
{
public:
    // json keys
    static constexpr char const * sid_fonts             = "fonts";
    static constexpr char const * sid_file_name         = "file_name";
    static constexpr char const * sid_font_id           = "font_id";
    static constexpr char const * sid_hinting           = "hinting";
    static constexpr char const * sid_kerning           = "kerning";
    static constexpr char const * sid_outline_thickness = "outline_thickness";
    static constexpr char const * sid_outline_type      = "outline_type";
    static constexpr char const * sid_font_size         = "font_size";
    static constexpr char const * sid_glyphs            = "glyphs";

    static Glyph::OutlineType GetOutlineTypeFromString(std::string_view str_outline);

public:
    void parseFontsRes(std::string const & file_name);

    TexFont & addFont(FontDataDesc const & desc);
    TexFont * getFont(std::string name, uint32_t size);

    AtlasTex & getAtlas() { return m_atlas; }
    void       resizeAtlas();

private:
    using font_map = std::map<std::size_t, std::unique_ptr<TexFont>>;

    AtlasTex m_atlas;   // one tex atlas for all loaded fonts
    font_map m_fonts;
};

#endif

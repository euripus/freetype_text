#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include "texfont.h"
#include "atlastex.h"
#include <map>
#include <memory>

struct FontDataDesc
{
    std::string        filename;
    float              pt_size           = 24.0f;
    bool               hinting           = true;
    bool               kerning           = true;
    float              outline_thickness = 0.0f;
    Glyph::OutlineType outline_type      = Glyph::OutlineType::NONE;
};

class FontManager
{
public:
    TexFont & addFont(FontDataDesc const & desc);
    TexFont & getFont(std::string name, uint32_t size);

    AtlasTex & getAtlas() { return m_atlas; }
    void       resizeAtlas();

private:
    using font_map = std::map<std::size_t, std::unique_ptr<TexFont>>;

    AtlasTex m_atlas;   // one tex atlas for all loaded fonts
    font_map m_fonts;
};

#endif
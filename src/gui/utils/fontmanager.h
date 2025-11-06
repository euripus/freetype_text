#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include "texfont.h"
#include "atlastex.h"
#include "../uiconfigloader.h"
#include "../../fs/file_system.h"
#include <map>
#include <memory>

class FontManager
{
public:
    FontManager(FileSystem & fsys)
        : m_file_sys(fsys)
    {}
    TexFont & addFont(FontDataDesc const & desc);
    TexFont * getFont(std::string name, uint32_t size);

    AtlasTex & getAtlas() { return m_atlas; }
    void       resizeAtlas();

private:
    using font_map = std::map<std::size_t, std::unique_ptr<TexFont>>;

    FileSystem & m_file_sys;
    AtlasTex     m_atlas;   // one tex atlas for all loaded fonts
    font_map     m_fonts;
};

#endif

#ifndef GUI_H
#define GUI_H

#include <glm/glm.hpp>
#include "../input/input.h"
#include "../render/vertex_buffer.h"
#include "utils/fontmanager.h"
#include "uiimagemanager.h"
#include "packer.h"
#include "uiwindow.h"

class RendererBase;
struct Texture;

struct RefToDrawData
{
    VertexBuffer &                                    background;
    ColorMap::ColoredTextBuffers &                    text;
    std::vector<std::pair<VertexBuffer, Texture *>> & textured_quads;
};

class UI
{
public:
    UI(FileSystem & fsys);

    void       update(float time);
    void       resize(int32_t w, int32_t h) { m_screen_size = glm::ivec2{w, h}; }
    glm::ivec2 getScreenSize() const { return m_screen_size; }

    bool init(RendererBase & render);
    void draw(RendererBase & render);
    void terminate(RendererBase & render);

    UIWindow * loadWindow(InFile & file_json, int32_t layer = 0,
                          std::string const & image_group = std::string());

    void fitWidgets(UIWindow * win_ptr) const;

    AtlasTex & getUIImageAtlas() { return m_ui_image_atlas.getAtlas(); }
    AtlasTex & getFontImageAtlas() { return m_fonts.getAtlas(); }

    glm::vec4 const & getFontColor() const { return m_font_color; }

    // private
    void clearAndFillBuffers(RefToDrawData draw_data) const;

    Input *      m_input = nullptr;
    FileSystem & m_fsys;

    glm::ivec2              m_screen_size = {};
    UIImageGroupManager     m_ui_image_atlas;
    FontManager             m_fonts;
    TexFont *               m_default_font = nullptr;
    glm::vec4               m_font_color   = ColorMap::black;
    std::unique_ptr<Packer> m_packer;
    std::string             m_current_gui_set = {"default"};

    mutable VertexBuffer                 m_win_buf;
    mutable ColorMap::ColoredTextBuffers m_colored_text_buffers =
        ColorMap::ColoredTextBuffers{ColorMap::EpsilonLessVec4(0.001f)};
    mutable std::vector<std::pair<VertexBuffer, Texture *>> m_external_textured_regions;

    std::vector<std::unique_ptr<UIWindow>> m_windows;
    std::vector<std::vector<UIWindow *>>   m_layers;
};

#endif

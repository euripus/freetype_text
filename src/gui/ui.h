#ifndef GUI_H
#define GUI_H

#include <glm/glm.hpp>
#include "../input/input.h"
#include "../render/vertex_buffer.h"
#include "utils/fontmanager.h"
#include "uiimagemanager.h"
#include "packer.h"
#include "uiwindow.h"

namespace ColorMap
{
// Standart colors
constexpr glm::vec4 black   = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
constexpr glm::vec4 silver  = glm::vec4(.75f, .75f, .75f, 1.0f);
constexpr glm::vec4 gray    = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
constexpr glm::vec4 white   = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
constexpr glm::vec4 maroon  = glm::vec4(0.5f, 0.0f, 0.0f, 1.0f);
constexpr glm::vec4 red     = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
constexpr glm::vec4 purple  = glm::vec4(0.5f, 0.0f, 0.5f, 1.0f);
constexpr glm::vec4 fuchsia = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
constexpr glm::vec4 green   = glm::vec4(0.0f, 0.5f, 0.0f, 1.0f);
constexpr glm::vec4 lime    = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
constexpr glm::vec4 olive   = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
constexpr glm::vec4 yellow  = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
constexpr glm::vec4 navy    = glm::vec4(0.0f, 0.0f, 0.5f, 1.0f);
constexpr glm::vec4 blue    = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
constexpr glm::vec4 teal    = glm::vec4(0.0f, 0.5f, 0.5f, 1.0f);
constexpr glm::vec4 aqua    = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);

using ColorBuffers = std::map<glm::vec4, VertexBuffer>;
}   // namespace ColorMap

class RendererBase;

class UI
{
public:
    UI(FileSystem & fsys);

    void       update(float time);
    void       clearAndFillBuffers(VertexBuffer & background, VertexBuffer & text) const;
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

    Input *      m_input = nullptr;
    FileSystem & m_fsys;

    glm::ivec2              m_screen_size = {};
    UIImageGroupManager     m_ui_image_atlas;
    FontManager             m_fonts;
    TexFont *               m_default_font = nullptr;
    glm::vec4               m_font_color   = ColorMap::black;
    std::unique_ptr<Packer> m_packer;
    std::string             m_current_gui_set = {"default"};

    mutable VertexBuffer m_win_buf;
    mutable VertexBuffer m_text_win_buf;

    std::vector<std::unique_ptr<UIWindow>> m_windows;
    std::vector<std::vector<UIWindow *>>   m_layers;
};

#endif

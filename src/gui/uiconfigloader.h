#ifndef UICONFIGLOADER_H
#define UICONFIGLOADER_H

#include <glm/glm.hpp>
#include <memory>
#include "basic_types.h"
#include "texfont.h"
#include "../fs/file_system.h"

class UIImageManager;
class UIWindow;
class UI;
class Widget;

struct FontDataDesc
{
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

    std::string        filename;
    std::string        font_id;
    float              pt_size           = 24.0f;
    bool               hinting           = true;
    bool               kerning           = true;
    float              outline_thickness = 0.0f;
    Glyph::OutlineType outline_type      = Glyph::OutlineType::NONE;

    static Glyph::OutlineType GetOutlineTypeFromString(std::string_view str_outline);
    static void               ParseFontsRes(FontManager & fmgr, InFile & file_json);
};

struct WidgetDesc
{
    // json keys
    static constexpr char const * sid_minimal_size     = "minimal_size";
    static constexpr char const * sid_maximal_size     = "maximal_size";
    static constexpr char const * sid_type             = "type";
    static constexpr char const * sid_visible          = "visible";
    static constexpr char const * sid_region_name      = "region_name";
    static constexpr char const * sid_id_name          = "id_name";
    static constexpr char const * sid_stretch          = "stretch";
    static constexpr char const * sid_align_horizontal = "align_horizontal";
    static constexpr char const * sid_align_vertical   = "align_vertical";
    static constexpr char const * sid_font             = "font";
    static constexpr char const * sid_font_size        = "font_size";
    static constexpr char const * sid_static_text      = "static_text";
    static constexpr char const * sid_text_horizontal  = "text_horizontal";
    static constexpr char const * sid_children         = "children";

    // constants
    static constexpr float MaxWidgetSize = std::numeric_limits<int>::max();

    static ElementType GetElementTypeFromString(std::string_view name);
    static Align       GetAlignFromString(std::string_view name);

    static std::unique_ptr<Widget> GetWidgetFromDesc(WidgetDesc const & desc, UIWindow & owner);

    glm::vec2   min_size    = {};
    glm::vec2   max_size    = {MaxWidgetSize, MaxWidgetSize};
    ElementType type        = ElementType::Unknown;
    float       stretch     = 0.f;
    bool        visible     = true;
    std::string region_name = {};
    std::string id_name     = {};
    Align       horizontal  = Align::left;
    Align       vertical    = Align::top;
    std::string font_name   = {};
    float       size        = 0.0f;
    std::string static_text = {};
    Align       text_hor    = Align::left;
};

struct WindowDesc
{
    // json keys
    static constexpr char const * sid_window_caption = "window_caption";
    static constexpr char const * sid_window_size    = "window_size";
    static constexpr char const * sid_window_spacing = "window_spacing";
    static constexpr char const * sid_widgets        = "widgets";

    static void LoadWindow(UIWindow & win, InFile & file_json);
};

struct UIImageManagerDesc
{
    // json keys
    static constexpr char const * sid_gui_set        = "gui_sets";
    static constexpr char const * sid_set_name       = "set_name";
    static constexpr char const * sid_images         = "images";
    static constexpr char const * sid_texture        = "texture";
    static constexpr char const * sid_9slice_margins = "9slice_margins";

    static void ParseUIRes(UIImageManager & mgr, InFile & file_json, FileSystem & fsys);
};

struct UIDesc
{
    static constexpr char const * sid_gui_set          = "current_gui_set";
    static constexpr char const * sid_defult_font      = "defult_font";
    static constexpr char const * sid_defult_font_size = "defult_font_size";

    static void ParseDefaultUISetID(UI & ui, InFile & file_json);
};

#endif

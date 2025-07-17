#include "uiconfigloader.h"
#include "ui.h"
#include "fontmanager.h"
#include "widget.h"
#include "window.h"
#include "text_box.h"
#include "button.h"
#include <boost/json.hpp>

Glyph::OutlineType FontDataDesc::GetOutlineTypeFromString(std::string_view str_outline)
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

void FontDataDesc::ParseFontsRes(FontManager & fmgr, InFile & file_json)
{
    boost::json::value jv;

    try
    {
        std::string file_data;

        std::string line;
        while(GetLine(file_json.getStream(), line))
        {
            file_data += line;
        }
        file_json.getStream().resetHead();

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
            for(auto const & font_entry : arr)
            {
                auto const & font_obj = font_entry.as_object();
                FontDataDesc desc;
                std::string  glyphs;

                for(auto const & kvp : font_obj)
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
                        std::string error = "Unknown parameter: " + std::string(kvp.key())
                                            + " in file: " + file_json.getName();
                        throw std::runtime_error(error);
                    }
                }

                auto & fnt = fmgr.addFont(desc);
                fnt.cacheGlyphs(glyphs.c_str());
            }
        }
    }
}

ElementType WidgetDesc::GetElementTypeFromString(std::string_view name)
{
    ElementType type = ElementType::Unknown;

    if(name == "TextBox")
        type = ElementType::TextBox;
    else if(name == "ImageBox")
        type = ElementType::ImageBox;
    else if(name == "Button")
        type = ElementType::Button;
    else if(name == "CheckBox")
        type = ElementType::CheckBox;
    // else if(name == "RadioButton")
    // type = ElementType::RadioButton;
    else if(name == "Slider")
        type = ElementType::Slider;
    else if(name == "ProgressBar")
        type = ElementType::ProgressBar;
    else if(name == "InputBox")
        type = ElementType::InputBox;
    else if(name == "ScrollView")
        type = ElementType::ScrollView;
    else if(name == "VerticalLayoutee")
        type = ElementType::VerticalLayoutee;
    else if(name == "HorizontalLayoutee")
        type = ElementType::HorizontalLayoutee;

    return type;
}

Align WidgetDesc::GetAlignFromString(std::string_view name)
{
    Align align = Align::left;

    if(name == "left")
        align = Align::left;
    else if(name == "center")
        align = Align::center;
    else if(name == "right")
        align = Align::right;
    else if(name == "top")
        align = Align::top;
    else if(name == "bottom")
        align = Align::bottom;

    return align;
}

static std::unique_ptr<Widget> GetWidgetFromJson(boost::json::object const & obj, UIWindow & owner)
{
    assert(!obj.empty());

    WidgetDesc desc;
    for(auto const & kvp : obj)
    {
        if(kvp.key() == WidgetDesc::sid_minimal_size)
        {
            std::vector<int32_t> vec;
            vec = boost::json::value_to<std::vector<int32_t>>(kvp.value());

            desc.min_size.x = static_cast<float>(vec[0]);
            desc.min_size.y = static_cast<float>(vec[1]);
        }
        else if(kvp.key() == WidgetDesc::sid_maximal_size)
        {
            std::vector<int32_t> vec;
            vec = boost::json::value_to<std::vector<int32_t>>(kvp.value());

            desc.max_size.x = static_cast<float>(vec[0]);
            desc.max_size.y = static_cast<float>(vec[1]);
        }
        else if(kvp.key() == WidgetDesc::sid_type)
        {
            desc.type = WidgetDesc::GetElementTypeFromString(kvp.value().as_string());
        }
        else if(kvp.key() == WidgetDesc::sid_stretch)
        {
            desc.stretch = static_cast<float>(kvp.value().as_int64());
        }
        else if(kvp.key() == WidgetDesc::sid_visible)
        {
            desc.visible = kvp.value().as_bool();
        }
        else if(kvp.key() == WidgetDesc::sid_region_name)
        {
            desc.region_name = kvp.value().as_string();
        }
        else if(kvp.key() == WidgetDesc::sid_id_name)
        {
            desc.id_name = kvp.value().as_string();
        }
        else if(kvp.key() == WidgetDesc::sid_align_horizontal)
        {
            desc.horizontal = WidgetDesc::GetAlignFromString(kvp.value().as_string());
        }
        else if(kvp.key() == WidgetDesc::sid_align_vertical)
        {
            desc.vertical = WidgetDesc::GetAlignFromString(kvp.value().as_string());
        }
        else if(kvp.key() == WidgetDesc::sid_font)
        {
            desc.font_name = kvp.value().as_string();
        }
        else if(kvp.key() == WidgetDesc::sid_font_size)
        {
            desc.size = static_cast<float>(kvp.value().as_int64());
        }
        else if(kvp.key() == WidgetDesc::sid_static_text)
        {
            desc.static_text = kvp.value().as_string();
        }
        else if(kvp.key() == WidgetDesc::sid_text_horizontal)
        {
            desc.text_hor = WidgetDesc::GetAlignFromString(kvp.value().as_string());
        }
    }

    auto widg_ptr = WidgetDesc::GetWidgetFromDesc(desc, owner);

    if(auto const children_it = obj.find(WidgetDesc::sid_children); children_it != obj.end())
    {
        auto const & arr = children_it->value().as_array();
        if(!arr.empty())
        {
            for(auto const & child_entry : arr)
            {
                auto const & widget_obj = child_entry.as_object();
                if(!widget_obj.empty())
                {
                    widg_ptr->addWidget(GetWidgetFromJson(widget_obj, owner));
                }
            }
        }
    }

    return widg_ptr;
}

std::unique_ptr<Widget> WidgetDesc::GetWidgetFromDesc(WidgetDesc const & desc, UIWindow & owner)
{
    std::unique_ptr<Widget> result;

    switch(desc.type)
    {
        case ElementType::TextBox:
        {
            result = std::make_unique<TextBox>(desc, owner);

            break;
        }
        // case ElementType::ImageBox:
        // {
        //     // result = std::make_unique<ImageBox>(std::string(), owner);
        //     break;
        // }
        case ElementType::Button:
        {
            result = std::make_unique<Button>(desc, owner);

            break;
        }
        case ElementType::VerticalLayoutee:
        case ElementType::HorizontalLayoutee:
        case ElementType::Unknown:
        case ElementType::ImageBox:
        case ElementType::Empty:
        {
            result = std::make_unique<Widget>(desc, owner);
            break;
        }
    }

    return result;
}

void WindowDesc::LoadWindow(UIWindow & win, InFile & file_json)
{
    boost::json::value jv;

    try
    {
        std::string file_data;

        std::string line;
        while(GetLine(file_json.getStream(), line))
        {
            file_data += line;
        }
        file_json.getStream().resetHead();

        jv = boost::json::parse(file_data);
    }
    catch(std::exception const & e)
    {
        throw std::runtime_error(e.what());
    }

    assert(!jv.is_null());

    if(auto const & win_obj = jv.get_object(); !win_obj.empty())
    {
        for(auto const & kvp : win_obj)
        {
            if(kvp.key() == sid_window_size)
            {
                std::vector<int32_t> vec;
                vec = boost::json::value_to<std::vector<int32_t>>(kvp.value());

                win.m_rect.m_size.x = static_cast<float>(vec[0]);
                win.m_rect.m_size.y = static_cast<float>(vec[1]);
            }
            else if(kvp.key() == sid_window_caption)
            {
                win.m_caption = kvp.value().as_string();
            }
            else if(kvp.key() == sid_window_spacing)
            {
                win.m_spacing = static_cast<float>(kvp.value().as_int64());
            }
            else if(kvp.key() == sid_widgets)
            {
                auto const & arr = kvp.value().as_array();
                if(!arr.empty())
                {
                    auto const & root_entry = arr[0];
                    win.m_root              = GetWidgetFromJson(root_entry.as_object(), win);

                    auto const & background_entry = arr[1];
                    win.m_background              = GetWidgetFromJson(background_entry.as_object(), win);
                }
            }
        }
    }
}

void parseImages(boost::json::value const & jv, UIImageGroup & group, FileSystem & fsys)
{
    auto const & arr = jv.get_array();
    if(!arr.empty())
    {
        for(auto const & kvp : arr)
        {
            std::string          path;
            std::string          name;
            std::vector<int32_t> margins;

            auto const it = kvp.get_object().begin();
            name          = it->key();

            for(auto const & kvp2 : it->value().as_object())
            {
                if(kvp2.key() == UIImageManagerDesc::sid_texture)
                    path = kvp2.value().as_string();
                else if(kvp2.key() == UIImageManagerDesc::sid_9slice_margins)
                {
                    margins = boost::json::value_to<std::vector<int32_t>>(kvp2.value());
                }
            }

            tex::ImageData image;
            if(auto file = fsys.getFile(path); file)
            {
                if(!tex::ReadTGA(*file, image))
                    continue;
            }
            else
                continue;

            if(group.addImage(name, path, image, margins[0], margins[1], margins[2], margins[3]) == -1)
            {
                // texture atlas is full
                // let's try again
                group.getOwner().resizeAtlas();
                if(group.addImage(name, path, image, margins[0], margins[1], margins[2], margins[3]) == -1)
                    throw std::runtime_error("Texture atlas is full");
            }
        }
    }
}

void UIImageManagerDesc::ParseUIRes(UIImageGroupManager & mgr, InFile & file_json, FileSystem & fsys)
{
    boost::json::value jv;

    try
    {
        std::string file_data;

        std::string line;
        while(GetLine(file_json.getStream(), line))
        {
            file_data += line;
        }
        file_json.getStream().resetHead();

        jv = boost::json::parse(file_data);
    }
    catch(std::exception const & e)
    {
        throw std::runtime_error(e.what());
    }

    assert(!jv.is_null());

    auto const & obj        = jv.get_object();
    auto const   gui_set_it = obj.find(sid_gui_set);
    if(gui_set_it != obj.end())
    {
        auto const & arr = gui_set_it->value().as_array();
        if(!arr.empty())
        {
            for(auto const & set_val : arr)
            {
                std::string gr_name;

                auto const & array_obj = set_val.as_object();
                for(auto const & kvp : array_obj)
                {
                    if(kvp.key() == sid_set_name)
                    {
                        gr_name = kvp.value().as_string();
                    }
                    else if(kvp.key() == sid_images)
                    {
                        mgr.m_groups[gr_name] = std::make_unique<UIImageGroup>(mgr, fsys);
                        parseImages(kvp.value(), *mgr.m_groups[gr_name], fsys);
                    }
                }
            }
        }
    }
    else
    {
        std::string err = "In file " + file_json.getName() + " not found " + sid_gui_set;
        throw std::runtime_error(err);
    }
}

void UIDesc::ParseDefaultUISetID(UI & ui, InFile & file_json)
{
    boost::json::value jv;

    try
    {
        std::string file_data;

        std::string line;
        while(GetLine(file_json.getStream(), line))
        {
            file_data += line;
        }
        file_json.getStream().resetHead();

        jv = boost::json::parse(file_data);
    }
    catch(std::exception const & e)
    {
        throw std::runtime_error(e.what());
    }

    assert(!jv.is_null());

    auto const & obj        = jv.get_object();
    auto const   gui_set_it = obj.find(sid_gui_set);
    if(gui_set_it != obj.end())
    {
        ui.m_current_gui_set = gui_set_it->value().as_string();
    }

    auto const default_font_name = obj.find(sid_defult_font);
    auto const default_font_size = obj.find(sid_defult_font_size);
    if(default_font_name != obj.end() && default_font_size != obj.end())
    {
        std::string name{default_font_name->value().as_string()};
        int32_t     size = default_font_size->value().as_int64();

        ui.m_default_font = ui.m_fonts.getFont(name, size);
    }

    if(ui.m_default_font == nullptr)
        throw std::runtime_error("Default UI font not found");
}

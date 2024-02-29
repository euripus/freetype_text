#include "window.h"
#include "ui.h"
#include "text_box.h"
#include "button.h"
#include <fstream>
#include <boost/json.hpp>

UIWindow::UIWindow(UI & owner, std::string const & image_group)
    : m_owner(owner)
{
    m_images = &m_owner.m_ui_image_atlas.getImageGroup(image_group);
}

void UIWindow::draw(VertexBuffer & background, VertexBuffer & text) const
{
    if(!m_visible)
        return;

    if(m_background)
        m_background->draw(background, text);

    if(m_root)
        m_root->draw(background, text);
}

void UIWindow::update(float time, bool check_cursor)
{
    if(m_child_resized)
    {
        m_owner.fitWidgets(this);
        move(m_pos);
    }

    if(!m_visible)
        return;

    if(m_root)
        m_root->update(time, check_cursor);
    if(m_background)
        m_background->update(time, check_cursor);

    if(!m_callbacks.empty())
    {
        for(auto & fn: m_callbacks)
            fn();

        m_callbacks.clear();
    }
}

void UIWindow::move(glm::vec2 const & new_origin)
{
    m_pos = new_origin;

    if(m_root)
        m_root->move(m_pos);

    if(m_background)
    {
        m_background->setRect(getRect());
        m_background->move(m_pos);
    }
}

void UIWindow::addCallBack(std::function<void(void)> fn)
{
    if(fn)
        m_callbacks.push_back(fn);
}

Widget * UIWindow::getWidgetFromID(std::string const & id_name) const
{
    if(auto ptr = m_root->getWidgetFromIDName(id_name); ptr != nullptr)
        return ptr;

    if(auto ptr = m_background->getWidgetFromIDName(id_name); ptr != nullptr)
        return ptr;

    return nullptr;
}

void UIWindow::loadWindowFromDesc(std::string const & file_name)
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

    if(auto const & win_obj = jv.get_object(); !win_obj.empty())
    {
        auto const & val = *win_obj.begin();

        m_caption        = std::string(val.key());
        auto const & arr = val.value().as_array();
        if(!arr.empty())
        {
            auto const & root_entry = arr[0];
            m_root                  = GetWidgetFromJson(root_entry.as_object(), *this);

            auto const & background_entry = arr[1];
            m_background                  = GetWidgetFromJson(background_entry.as_object(), *this);
        }
    }

    childResized();
}

std::unique_ptr<Widget> UIWindow::GetWidgetFromDesc(WidgetDesc const & desc, UIWindow & owner)
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
        {
            result = std::make_unique<Widget>(desc, owner);
            break;
        }
    }

    return result;
}

std::unique_ptr<Widget> UIWindow::GetWidgetFromJson(boost::json::object const & obj, UIWindow & owner)
{
    assert(!obj.empty());

    WidgetDesc desc;
    for(auto const & kvp: obj)
    {
        if(kvp.key() == WidgetDesc::sid_size)
        {
            std::vector<int32_t> vec;
            vec = boost::json::value_to<std::vector<int32_t>>(kvp.value());

            desc.size_hint.x = static_cast<float>(vec[0]);
            desc.size_hint.y = static_cast<float>(vec[1]);
        }
        else if(kvp.key() == WidgetDesc::sid_type)
        {
            desc.type = WidgetDesc::GetElementTypeFromString(kvp.value().as_string());
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
        else if(kvp.key() == WidgetDesc::sid_size_policy)
        {
            desc.scale = WidgetDesc::GetSizePolicyFromString(kvp.value().as_string());
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

    auto widg_ptr = GetWidgetFromDesc(desc, owner);

    if(auto const children_it = obj.find(WidgetDesc::sid_children); children_it != obj.end())
    {
        auto const & arr = children_it->value().as_array();
        if(!arr.empty())
        {
            for(auto const & child_entry: arr)
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

#include "window.h"
#include "ui.h"
#include <fstream>
#include <boost/json.hpp>

UIWindow::UIWindow(UI & owner, std::string const & image_group) : m_owner(owner)
{
    m_images = &m_owner.m_ui_image_atlas.getImageGroup(image_group);
}

void UIWindow::draw() {}

void UIWindow::update(float time, bool check_cursor)
{
    if(!m_visible)
        return;

    if(m_root)
        m_root->update(time, check_cursor);
    if(m_background)
        m_background->update(time, check_cursor);
}

void UIWindow::move(glm::vec2 const & new_origin)
{
	m_pos = new_origin;

    m_root->move(new_origin);

    m_background->setRect(m_root->getRect());
    m_background->move(new_origin);
}

void UIWindow::childResized()
{
    m_owner.fitWidgets(this);
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
            m_root                  = Widget::GetWidgetFromDesc(root_entry.as_object(), *this);

            auto const & background_entry = arr[1];
            m_background                  = Widget::GetWidgetFromDesc(background_entry.as_object(), *this);
        }
    }
}

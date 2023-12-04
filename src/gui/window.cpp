#include "window.h"
#include "ui.h"
#include <fstream>
#include <boost/json.hpp>

struct WidgetDesc
{
    glm::vec2   size_hint   = {};
    ElementType type        = ElementType::Unknown;
    bool        visible     = true;
    std::string region_name = {};
    std::string id_name     = {};
    SizePolicy  scale       = SizePolicy::scale;
    Align       horizontal  = Align::left;
    Align       vertical    = Align::top;
    std::string font_name   = {};
    float       size        = 0.0f;
};

WidgetDesc ParseEntry(boost::json::object const & obj)
{
    WidgetDesc desc;
    return desc;
}

UIWindow::UIWindow(UI & owner, std::string caption, std::string_view image_group) :
    m_caption(std::move(caption)), m_owner(owner)
{
    m_images = &m_owner.m_ui_image_atlas.getImageGroup(std::string(image_group));
}

void UIWindow::update(float time, bool check_cursor) {}

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
}

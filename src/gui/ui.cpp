#include "ui.h"
#include <fstream>
#include <boost/json.hpp>

UI::UI(Input const & inp)
    : m_input(inp)
{
    m_packer = std::make_unique<ChainsPacker>();
}

void UI::update(float time)
{
    for(auto & ptr: m_windows)
    {
        ptr->update(time, true);
    }
}

void UI::draw(VertexBuffer & background, VertexBuffer & text) const
{
    for(auto const & ptr: m_windows)
    {
        ptr->draw(background, text);
    }
}

UIWindow * UI::loadWindow(std::string const & widgets_filename, int32_t layer,
                          std::string const & image_group)
{
    std::unique_ptr<UIWindow> win;
    if(image_group.empty())
        win = std::make_unique<UIWindow>(*this, m_current_gui_set);
    else
        win = std::make_unique<UIWindow>(*this, image_group);
    win->loadWindowFromDesc(widgets_filename);

    m_windows.push_back(std::move(win));

    auto * win_ptr = m_windows.back().get();
    fitWidgets(win_ptr);

    if(layer + 1 > static_cast<int32_t>(m_layers.size()))
        m_layers.resize(layer + 1);
    // fit windows on layer
    m_layers[layer].push_back(win_ptr);

    return win_ptr;
}

void UI::parseDefaultUISetID(std::string const & file_name)
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

    auto const & obj        = jv.get_object();
    auto const   gui_set_it = obj.find(sid_gui_set);
    if(gui_set_it != obj.end())
    {
        m_current_gui_set = gui_set_it->value().as_string();
    }

    auto const default_font_name = obj.find(sid_defult_font);
    auto const default_font_size = obj.find(sid_defult_font_size);
    if(default_font_name != obj.end() && default_font_size != obj.end())
    {
        std::string name = default_font_name->value().as_string();
        int32_t     size = default_font_size->value().as_int64();

        m_default_font = m_fonts.getFont(name, size);
    }

    if(m_default_font == nullptr)
        throw std::runtime_error("Default UI font not found");
}

void UI::parseUIResources(std::string const & file_name)
{
    m_ui_image_atlas.parseUIRes(file_name);
    m_fonts.parseFontsRes(file_name);
    parseDefaultUISetID(file_name);
}

void UI::fitWidgets(UIWindow * win_ptr) const
{
    if(win_ptr == nullptr)
        return;

    m_packer->setSpacing(win_ptr->getSpacing());
    m_packer->fitWidgets(win_ptr);
}

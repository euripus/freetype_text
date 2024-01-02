#include "ui.h"

void UI::update(float time)
{
    for(auto & ptr : m_windows)
    {
        ptr->update(time, true);
    }
}

UIWindow * UI::loadWindow(std::string const & widgets_filename, int32_t layer,
                          std::string const & image_group)
{
    auto win = std::make_unique<UIWindow>(*this, image_group);
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

void UI::parseUIResources(std::string const & file_name)
{
    m_ui_image_atlas.parseUIRes(file_name);
    m_fonts.parseFontsRes(file_name);
}

void UI::fitWidgets(UIWindow * win_ptr) const
{
    m_packer.fitWidgets(win_ptr);
    win_ptr->adjustSize();
}

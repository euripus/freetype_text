#include "ui.h"

void UI::update(float time) {}

UIWindow * UI::loadWindow(std::string const & widgets_filename, std::string const & caption,
                          std::string const & image_roup)
{
    auto win = std::make_unique<UIWindow>(*this, caption, image_roup);
    win->loadWindowFromDesc(widgets_filename);

    m_windows.push_back(std::move(win));
    return m_windows.back().get();
}

void UI::loadUIImageGroup(std::string const & file_name)
{
    m_ui_image_atlas.parseUIRes(file_name);
}

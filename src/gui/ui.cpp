#include "ui.h"
#include "uiconfigloader.h"

UI::UI(Input const & inp, FileSystem & fsys)
    : m_input(inp),
      m_fonts(fsys)
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

UIWindow * UI::loadWindow(InFile const & file_json, int32_t layer, std::string const & image_group)
{
    std::unique_ptr<UIWindow> win;
    if(image_group.empty())
        win = std::make_unique<UIWindow>(*this, m_current_gui_set);
    else
        win = std::make_unique<UIWindow>(*this, image_group);

    WindowDesc::LoadWindow(*win.get(), file_json);

    m_windows.push_back(std::move(win));

    auto * win_ptr = m_windows.back().get();
    fitWidgets(win_ptr);

    if(layer + 1 > static_cast<int32_t>(m_layers.size()))
        m_layers.resize(layer + 1);
    // fit windows on layer
    m_layers[layer].push_back(win_ptr);

    return win_ptr;
}

void UI::parseUIResources(InFile const & file_json)
{
    UIImageManagerDesc::ParseUIRes(m_ui_image_atlas, file_json);
    FontDataDesc::ParseFontsRes(m_fonts, file_json);
    UIDesc::ParseDefaultUISetID(*this, file_json);
}

void UI::fitWidgets(UIWindow * win_ptr) const
{
    if(win_ptr == nullptr)
        return;

    m_packer->setSpacing(win_ptr->getSpacing());
    m_packer->fitWidgets(win_ptr);
}

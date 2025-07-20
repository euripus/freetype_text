#include "uiwindow.h"
#include "ui.h"

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
    if(m_size_updated)
    {
        m_owner.fitWidgets(this);
        move(m_pos);

        m_size_updated = false;
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

Widget * UIWindow::getRootWidget() const
{
    if(m_root)
        return m_root.get();
    else
        return nullptr;
}

Widget * UIWindow::getBackgroundWidget() const
{
    if(m_background)
        return m_background.get();
    else
        return nullptr;
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

#include "packer.h"
#include "window.h"
#include "chain.h"
#include "basic_types.h"

template<typename T>
T const & GetRef(std::unique_ptr<T> const & ptr)
{
    if(ptr)
    {
        return *ptr.get();
    }

    throw std::runtime_error("Error dereferencing null unique_ptr!");
}

template<typename T>
T & GetRef(std::unique_ptr<T> & ptr)
{
    if(ptr)
    {
        return *ptr.get();
    }

    throw std::runtime_error("Error dereferencing null unique_ptr!");
}

void ChainsPacker::fitWidgets(UIWindow * win, float width, float height) const
{
    if(win == nullptr || win->getRootWidget() == nullptr)
        return;

    MemPool        pool;
    Widget &       root       = *win->getRootWidget();
    StringLayout * top_string = nullptr;

    if(root.getType() == ElementType::VerticalLayoutee || root.getType() == ElementType::HorizontalLayoutee)
    {
        if(root.getType() == ElementType::VerticalLayoutee)
        {
            top_string = pool.createNewLayout(Direction::Up, m_border);
            addNewString(root, top_string, pool);
        }
        else
        {
            top_string = pool.createNewLayout(Direction::LeftToRight, m_border);
            addNewString(root, top_string, pool);
        }
    }
    else
    {
        win->setSize(width, height);
        root.setSize(width, height);
    }

    if(top_string != nullptr)
    {
        auto new_size = top_string->resizeAll(width, height);
        win->setSize(new_size.x, new_size.y);
    }

    auto w_rect = win->getRect();
    if(auto backround = win->getBackgroundWidget(); backround != nullptr)
    {
        backround->setSize(w_rect.width(), w_rect.height());
    }
}

void ChainsPacker::addNewString(Widget & string_node, StringLayout * parent, MemPool & pool) const
{
    for(auto & ch: string_node.getChildren())
    {
        auto & w = GetRef(ch);
        switch(w.getType())
        {
            case ElementType::VerticalLayoutee:
            {
                auto * nested_string = pool.createNewLayout(Direction::Up, m_border);
                parent->addString(nested_string);
                addNewString(w, nested_string, pool);

                break;
            }
            case ElementType::HorizontalLayoutee:
            {
                auto * nested_string = pool.createNewLayout(Direction::LeftToRight, m_border);
                parent->addString(nested_string);
                addNewString(w, nested_string, pool);

                break;
            }
            default:
            {
                addWidgetInLayout(w, parent);
                break;
            }
        }
    }
}

void ChainsPacker::addWidgetInLayout(Widget & node, StringLayout * layout) const
{
    Align align = Align::center;
    if(layout->direction() == Direction::LeftToRight || layout->direction() == Direction::RightToLeft)
    {
        align = node.getHorizontalAlign();
    }
    else
    {
        align = node.getVerticalAlign();
    }

    layout->addWidget(&node, node.getStretch(), align);
}

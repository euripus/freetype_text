#include "packer.h"
#include "window.h"
#include "chain.h"
#include "basic_types.h"
#include <stdexcept>

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

static glm::vec2 GetWidgetsSize(Widget const & root, float border)
{
    glm::vec2 result{0.0f};

    switch(root.getType())
    {
        case ElementType::VerticalLayoutee:
        {
            for(auto const & ch: root.getChildren())
            {
                auto const child_size = GetWidgetsSize(GetRef(ch), border);

                result.x  = std::max(result.x, child_size.x);
                result.y += child_size.y;
            }

            if(root.getNumChildren() > 1)
            {
                result.y += border * (root.getNumChildren() - 1);
            }

            break;
        }
        case ElementType::HorizontalLayoutee:
        {
            for(auto const & ch: root.getChildren())
            {
                auto const child_size = GetWidgetsSize(GetRef(ch), border);

                result.x += child_size.x;
                result.y  = std::max(result.y, child_size.y);
            }

            if(root.getNumChildren() > 1)
            {
                result.x += border * (root.getNumChildren() - 1);
            }

            break;
        }
        default:
        {
            result = root.getSize();

            break;
        }
    }

    return result;
}

void ChainsPacker::fitWidgets(UIWindow * win) const
{
    if(win == nullptr || win->getRootWidget() == nullptr)
        return;

    MemPool        pool;
    Widget &       root       = *win->getRootWidget();
    StringLayout * top_string = nullptr;
    auto           size       = glm::max(GetWidgetsSize(root, m_border), win->size());

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
        win->setSize(size.x, size.y);
        root.setSize(size.x, size.y);
    }

    if(top_string != nullptr)
    {
        auto new_size = top_string->resizeAll(size.x, size.y);
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

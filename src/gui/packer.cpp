#include "packer.h"
#include "basic_types.h"
#include <algorithm>

void Packer::fitWidgets(Widget * root) {}

Packer::FinalList Packer::getListFromTree(Widget * root)
{
    Packer::FinalList list;

    if(root != nullptr)
        addSubTree(list, root);

    std::reverse(std::begin(list), std::end(list));

    return list;
}

void Packer::addSubTree(FinalList & ls, Widget * root)
{
    ls.push_back({});
    auto & ch_list = ls.back();

    if(root->m_type == ElementType::VerticalLayoutee || root->m_type == ElementType::HorizontalLayoutee)
    {
        for(auto const & ch : root->m_children)
        {
            if(auto * cur_ch = ch.get(); cur_ch->m_type == ElementType::VerticalLayoutee
                                         || cur_ch->m_type == ElementType::HorizontalLayoutee)
            {
                if(cur_ch->m_type == ElementType::HorizontalLayoutee)
                    addSubTree(ls, cur_ch);
                else
                {
                    for(auto const & ch2 : cur_ch->m_children)
                    {
                        addSubTree(ls, ch2.get());
                    }
                }
            }
            else
            {
                ch_list.push_back(cur_ch);
            }
        }
    }
    else
    {
        ch_list.push_back(root);
    }
}

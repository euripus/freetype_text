#include "packer.h"

void Packer::fitWidgets(Widget * root)
{}

FinalList Packer::getListFromTree(Widget * root)
{
	FinalList list;	
	
	if(root != nullptr)
		addSubTree(list, root);
	
	std::reverse(std.begin(list), std::end(list));
	
	return list;
}

void Packer::addSubTree(FinalList & ls, Widget * root)
{
	ls.push_back({});
	auto & ch_list = ls.back();
	
	if(root->m_state == VerticalLayoutee || root->m_state == HorizontalLayoutee)
	{
		for(auto const & ch : root->m_children)
		{
			if(auto const * cur_ch = ch.get(); cur_ch->m_state != VerticalLayoutee || cur_ch->m_state != HorizontalLayoutee)
			{
				ch_list.push_back(cur_ch);
			}
			else
			{
				if(cur_ch->m_state == HorizontalLayoutee)
					addSubTree(list, cur_ch);
				else
				{
					for(auto const & ch2 : cur_ch->m_children)
					{
						addSubTree(list, ch2.get());
					}
				}
			}
		}
	}
	else
	{
		ch_list.push_back(root);
	}
}
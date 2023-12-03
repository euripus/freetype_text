#include "window.h"

struct WidgetDesc
{
	glm::vec2   size_hint ={};
	ElementType type = ElementType::Unknown;
	bool        visible    = true;
	std::string region_name ={};
	std::string id_name ={};
	SizePolicy  scale      = SizePolicy::scale;
	Align       horizontal = Align::left;
    Align       vertical   = Align::top;
	std::string font_name ={};
	float       size = 0.0f;
};

WidgetDesc ParseEntry(boost::json::object const & obj)
{
	WidgetDesc desc;
	return desc;
}

void UIWindow::update(float time) {}

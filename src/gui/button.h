#ifndef BUTTON_H
#define BUTTON_H

#include "widget.h"

class Button : public Widget
{
public:
	enum class ButtonState
	{
		clicked,
		unclicked,
		disabled
	};

	Button(WidgetDesc const & desc, UIWindow & owner);

    // Widget interface
    void update(float time, bool check_cursor) override;

    void setCallback(std::function<void(void)> click_callback);

private:
    void subClassDraw(VertexBuffer & background, VertexBuffer & text) const override;

	RegionDataOfUITexture const * getRegionFromState(ButtonState state) const;

protected:
    std::string               m_caption;
    std::function<void(void)> m_click_callback;
    ButtonState               m_state = ButtonState::unclicked;
};

#endif

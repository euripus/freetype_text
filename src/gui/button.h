#ifndef BUTTON_H
#define BUTTON_H

#include "widget.h"

class Button : public Widget
{
public:
    Button(std::string name, UIWindow & owner);

    // Widget interface
    void update(float time, bool check_cursor) override;
    void draw() override;
    void move(glm::vec2 const & new_origin) override;

    void setCallback(std::function<void(void)> click_callback);

protected:
    std::string               m_caption;
    std::function<void(void)> m_click_callback;
    bool                      m_enabled = true;
};

#endif

#include "text_box.h"

TextBox::TextBox(std::string const & text, UIWindow & owner) : Widget(owner), m_text(text) {}

void TextBox::update(float time, bool check_cursor) {}

void TextBox::draw() {}

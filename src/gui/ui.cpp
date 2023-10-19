#include "ui.h"

void UI::update(float time) {}

UIWindow & UI::loadWindow(std::string_view file_name) {}

ElementType UI::GetElementTypeFromString(std::string_view name) {}

SizePolicy UI::GetSizePolicyFromString(std::string_view name) {}

Align UI::GetAlignFromString(std::string_view name) {}

std::unique_ptr<Widget> UI::CreateWidget(ElementType type) {}

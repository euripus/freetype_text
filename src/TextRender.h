#ifndef TEXTRENDER_H
#define TEXTRENDER_H

#include "TexFont.h"
#include "VertexBuffer.h"

void AddText(VertexBuffer & vb, TexFont & tf, wchar_t * text, glm::vec2 & pen);

#endif   // TEXTRENDER_H

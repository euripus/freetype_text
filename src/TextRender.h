#ifndef TEXTRENDER_H
#define TEXTRENDER_H

#include "TexFont.h"
#include "VertexBuffer.h"

void AddText(VertexBuffer & vb, TexFont & tf, char const * text, glm::vec2 & pos);

#endif   // TEXTRENDER_H

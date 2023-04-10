#ifndef __TGA_H__
#define __TGA_H__

#include <GL/glew.h>   // Header for OpenGL32 library
#include <stdio.h>     // Standard I/O header

typedef struct
{
    GLubyte * imageData;   // Image Data (Up To 32 Bits)
    GLuint    bpp;         // Image Color Depth In Bits Per Pixel
    GLuint    width;       // Image Width
    GLuint    height;      // Image Height
    GLuint    texID;       // Texture ID Used To Select A Texture
    GLuint    type;        // Image Type (GL_RGB, GL_RGBA)
} Texture;

bool LoadTGA(Texture * texture, char * filename);
void WriteUncompressedTGA(char const * fname, unsigned int depth, unsigned int width, unsigned int height,
                          unsigned char const * data);

#endif

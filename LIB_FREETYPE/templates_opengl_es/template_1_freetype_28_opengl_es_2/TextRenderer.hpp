#pragma once

#include "opengl_includes.hpp"
#include <string>

class TextRenderer
{
public:
    TextRenderer();
    ~TextRenderer();

    void Load(std::string font, GLuint fontSize);
    void RenderText();

private:
    GLfloat verticles_table[20]= {
        0.5f,  0.5f, 0.0f,    1.0f,0.0f,
        0.5f, -0.5f, 0.0f,    1.0f,1.0f,
        -0.5f, -0.5f, 0.0f,   0.0f,1.0f,
        -0.5f, 0.5f, 0.0f,    0.0f,0.0f
    };
    GLuint vbo;
    GLint position_location;
    GLint texCoord_attrib_location;
    GLint textureUnitLocation;

    GLuint TextureID;
};

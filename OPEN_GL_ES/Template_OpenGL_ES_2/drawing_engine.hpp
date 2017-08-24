#pragma once

#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>


GLuint DE_initShader();

typedef struct{
    GLuint vbo_id;
    GLuint texture_id;
} DE_Rectangle;



void DE_initRectangle(DE_Rectangle * rectangle, const char * textureFilename, GLfloat width, GLfloat height);
void DE_drawRectangle(DE_Rectangle * rectangle);

void DE_deleteRectangle(DE_Rectangle * rectangle);




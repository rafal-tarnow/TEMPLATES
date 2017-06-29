#pragma once

#include <GL/gl.h>

GLuint GenerateTexture(const char * filename);

void DeleteTexture(GLuint *texture_id);

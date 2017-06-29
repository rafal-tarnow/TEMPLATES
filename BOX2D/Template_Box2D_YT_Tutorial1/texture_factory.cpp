#include "texture_factory.hpp"
#include <SOIL/SOIL.h>
#include <GL/gl.h>

GLuint GenerateTexture(const char * filename) {

    GLuint texture;

    int width, height;
    unsigned char * image = SOIL_load_image(filename, &width, &height, 0, SOIL_LOAD_RGBA);

    // Create Texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);   // 2d texture (x and y size)

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture

    // 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image,
    // border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.
    glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    return texture;
};

void DeleteTexture(GLuint * texture_id){
    glDeleteTextures(1, texture_id);
}

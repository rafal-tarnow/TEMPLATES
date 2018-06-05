//
//  SpriteRenderer.hpp
//  OpenGLBreakout
//
//  Created by 梅宇宸 on 16/12/23.
//  Copyright © 2016年 梅宇宸. All rights reserved.
//

#ifndef SpriteRenderer_hpp
#define SpriteRenderer_hpp

#include "opengl_includes.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.hpp"

class Texture2D;

class SpriteRenderer
{
public:
    // Constructor (inits shaders/shapes)
    SpriteRenderer (Shader& shader);
    // Destructor
    ~SpriteRenderer ();
    // Renders a defined quad textured with given sprite
    void DrawSprite (Texture2D& texture, glm::vec2 position, glm::vec2 size = glm::vec2(10, 10), GLfloat rotate = 0.0f, glm::vec3 color = glm::vec3(1.0f));
private:
    // Render state
    Shader shader;
    GLuint VBO;
    // Initializes and configures the quad's buffer and vertex attributes
    void initRenderData ();
};
#endif /* SpriteRenderer_hpp */

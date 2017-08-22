#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

#include "texture_manager.hpp"

static const GLuint WIDTH = 800;
static const GLuint HEIGHT = 600;
void InitGLFWWindow();
GLint compileShaders(const char *vertex_shader_source, const char *fragment_shader_source);
void initOpenGLViewport();
GLuint prepareVBO(const GLfloat * data, GLsizeiptr size);




static const GLchar* vertex_shader_source =
        "#version 100                           \n"
        "attribute vec3 position;               \n"
        "attribute vec2 texCoord;               \n"
        "varying vec2 v_TexCoordinate;          \n"
        "void main() {                          \n"
        "   gl_Position = vec4(position, 1.0);  \n"
        "   v_TexCoordinate = texCoord;          \n"
        "}                                      \n";


static const GLchar* fragment_shader_source =
        "#version 100                               \n"
        "precision mediump float;                   \n"
        "varying vec2 v_TexCoordinate;              \n"
        "uniform sampler2D ourTexture;              \n"
        "void main() {                              \n"
        "   gl_FragColor = texture2D(ourTexture,v_TexCoordinate);\n"
        "}                                          \n";


//static const GLfloat quad_vertices[] = {
//    0.5f,  0.5f, 0.0f,
//    0.5f, -0.5f, 0.0f,
//    -0.5f, -0.5f, 0.0f,
//    -0.5f, 0.5f, 0.0f,
//};

static const GLfloat quad_vertices[] = {
    0.5f,  0.5f,    1.0f,1.0f,
    0.5f, 0.0f,     1.0f,0.0f,
    0.0f, 0.0f,     0.0f,0.0f,
    0.0f, 0.5f,     0.0f,1.0f,
};

static const GLfloat triangle_vertices[] = {
    0.0f,  0.5f, 0.0f,      0.5f, 1.0f,
    0.5f, -0.5f, 0.0f,      1.0f, 0.0f,
    -0.5f, -0.5f, 0.0f,     0.0f, 0.0f
};



GLFWwindow* window;
GLuint shader_program;
GLuint triangle_vbo_id;
GLuint rectangle_vbo_id;
GLint position_location;
GLint texCoord_attrib_location;
GLuint texture_id;

void renderFunction(){
    glClear(GL_COLOR_BUFFER_BIT );

    glUseProgram(shader_program);


//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, texture_id);
//        glUniform1i(texCoord_attrib_location, 0);
//        glBindBuffer(GL_ARRAY_BUFFER, rectangle_vbo_id);
//        {

//            glVertexAttribPointer(position_location, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
//            glEnableVertexAttribArray(position_location);
//            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//             //glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
//        }
//        glBindBuffer(GL_ARRAY_BUFFER, 0);


    glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo_id);


    glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(position_location);

    glVertexAttribPointer(texCoord_attrib_location, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(texCoord_attrib_location);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glUniform1i(texCoord_attrib_location, 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glUseProgram(0);

}


int main(void) {

    InitGLFWWindow();
    initOpenGLViewport();



    shader_program = compileShaders(vertex_shader_source, fragment_shader_source);

    position_location = glGetAttribLocation(shader_program, "position");
    texCoord_attrib_location = glGetAttribLocation(shader_program,"texCoord");

    //rectangle
    rectangle_vbo_id = prepareVBO(quad_vertices, sizeof(quad_vertices));
    triangle_vbo_id = prepareVBO(triangle_vertices, sizeof(triangle_vertices));

    texture_id = TextureManager::getTextureId("./data/png/bg.png");
    //texture_id = TextureManager::getTextureId("./data/png/coin_2.png");



    //MAIN LOOP
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        {
            renderFunction();
        }
        glfwSwapBuffers(window);
    }

    glDeleteBuffers(1, &triangle_vbo_id);
    glfwTerminate();
    return EXIT_SUCCESS;
}







GLuint prepareVBO(const GLfloat * data, GLsizeiptr size){
    GLuint vbo;

    glGenBuffers(1,&vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vbo;
}


void InitGLFWWindow(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(WIDTH, HEIGHT, __FILE__, NULL, NULL);
    glfwMakeContextCurrent(window);

    printf("GL_VERSION  : %s\n", glGetString(GL_VERSION) );
    printf("GL_RENDERER : %s\n", glGetString(GL_RENDERER) );
}

void initOpenGLViewport(){
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, WIDTH, HEIGHT);
}

GLint compileShaders(const char *vertex_shader_source, const char *fragment_shader_source) {
    enum Consts {INFOLOG_LEN = 512};
    GLchar infoLog[INFOLOG_LEN];
    GLint fragment_shader;
    GLint shader_program;
    GLint success;
    GLint vertex_shader;

    /* Vertex shader */
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    /* Fragment shader */
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }

    /* Link shaders */
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return shader_program;
}

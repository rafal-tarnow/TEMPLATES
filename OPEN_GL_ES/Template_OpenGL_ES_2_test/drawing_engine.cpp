#include "drawing_engine.hpp"
#include "texture_manager.hpp"

static GLint compileShaders(const char *vertex_shader_source, const char *fragment_shader_source);

static const GLchar* vertex_shader_source =
        "#version 100                           \n"
        "attribute vec3 position;               \n"
        "attribute vec2 texCoord;               \n"
        "varying vec2 v_TexCoordinate;          \n"
        "                                       \n"
        "uniform mat4 model;                    \n"
        "uniform mat4 view;                     \n"
        "uniform mat4 projection;               \n"
        "                                       \n"
        "void main() {                          \n"
        "   gl_Position =  projection * view * model * vec4(position, 1.0);  \n"
        "   v_TexCoordinate = texCoord;          \n"
        "}                                      \n";


static const GLchar* fragment_shader_source =
        "#version 100                                               \n"
        "precision mediump float;                                   \n"
        "varying vec2 v_TexCoordinate;                              \n"
        "uniform sampler2D textureUnit;                             \n"
        "//out vec4 color;                                          \n"
        "void main() {                                              \n"
        "   vec4 color = texture2D(textureUnit,v_TexCoordinate);    \n"
        "//   if (color.a < 0.9)                                    \n"
        "//      discard;                                           \n"
        "   gl_FragColor = color;                                   \n"
        "}                                                          \n";

static GLfloat rectangle_vertices[] = {
    1.0f,  1.0f, 0.0f,      1.0f, 1.0f,
    1.0f, -1.0f, 0.0f,      1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,     0.0f, 0.0f,
    -1.0f, 1.0f, 0.0f,      0.0f, 1.0f
};

static GLuint prepareVBO(const GLfloat * data, GLsizeiptr size){
    GLuint vbo;

    glGenBuffers(1,&vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vbo;
}

static GLint position_location;
static GLint texCoord_attrib_location;
static GLint textureUnitLocation;
static GLuint shader_program;

GLuint DE_initShader(){
    shader_program = compileShaders(vertex_shader_source, fragment_shader_source);

    position_location = glGetAttribLocation(shader_program, "position");
    texCoord_attrib_location = glGetAttribLocation(shader_program,"texCoord");
    textureUnitLocation = glGetUniformLocation (shader_program, "textureUnit" );

    return shader_program;
}


void DE_initRectangle(DE_Rectangle * rectangle, const char * textureFilename, GLfloat width, GLfloat height){
    width = width/2.0f;
    height = height/2.0f;

    //TOP RIGHT VERTICES
    rectangle_vertices[0] = width;
    rectangle_vertices[1] = height;
    //BOTTOM RIGHT VERTICES
    rectangle_vertices[5] = width;
    rectangle_vertices[6] = -height;
    //BOTTOM LEFT VERTICES
    rectangle_vertices[10] = -width;
    rectangle_vertices[11] = -height;
    //
    rectangle_vertices[15] = -width;
    rectangle_vertices[16] = height;

    rectangle->texture_id = TextureManager::getTextureId(textureFilename);
    rectangle->vbo_id = prepareVBO(rectangle_vertices, sizeof(rectangle_vertices));
}


void DE_drawRectangle(DE_Rectangle * rectangle){

    glBindBuffer(GL_ARRAY_BUFFER, rectangle->vbo_id);

    glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(position_location);

    glVertexAttribPointer(texCoord_attrib_location, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(texCoord_attrib_location);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rectangle->texture_id);
    glUniform1i(textureUnitLocation, GL_TEXTURE0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DE_deleteRectangle(DE_Rectangle * rectangle){
    glDeleteBuffers(1, &(rectangle->vbo_id));
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

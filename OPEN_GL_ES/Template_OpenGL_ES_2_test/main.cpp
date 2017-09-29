#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <stack>

using namespace std;

#include "drawing_engine.hpp"

static const GLuint WIDTH = 800;
static const GLuint HEIGHT = 600;
void InitGLFWWindow();

void initOpenGL();



DE_Rectangle rectangle;
DE_Rectangle coinRectangle;
GLFWwindow* window;
static GLuint shader_prog;

GLint modelLocation;
GLint viewLocation;
GLint projectionLocation;

glm::mat4 projectionMat;
glm::mat4 viewMat;
glm::mat4 modelMat;


void reshape(GLFWwindow * window, int width, int height){
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, width, height);

    cout << "reshape width = " << width << " height = " << height << endl;
    projectionMat  = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, 100.0f, -100.0f);
}


void renderFunction(){
    //cout << "renderFunction()" << endl;

    glClear(GL_COLOR_BUFFER_BIT );

    glUseProgram(shader_prog);
    {
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(modelMat));
        DE_drawRectangle(&rectangle);

        //glm::mat4 objectPositionMat = glm::translate(modelMat, glm::vec3(2.0f, 0.0f, 0.0f));
        //glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(objectPositionMat));
        DE_drawRectangle(&coinRectangle);

    }
    glUseProgram(0);

}


int main(void) {

    InitGLFWWindow();



    shader_prog = DE_initShader();

    modelLocation = glGetUniformLocation(shader_prog, "model");
    viewLocation = glGetUniformLocation(shader_prog, "view");
    projectionLocation = glGetUniformLocation(shader_prog, "projection");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(shader_prog);
    {
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMat));
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMat));
    }
    glUseProgram(0);

    DE_initRectangle(&coinRectangle,"./data/png/coin_2.png",0.5, 0.5);
    DE_initRectangle(&rectangle,"./data/png/bg.png" , 0.8, 0.8);



    //MAIN LOOP
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        {
            renderFunction();
        }
        glfwSwapBuffers(window);
    }

    DE_deleteRectangle(&rectangle);
    DE_deleteRectangle(&coinRectangle);

    glfwTerminate();
    return EXIT_SUCCESS;
}




void InitGLFWWindow(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(WIDTH, HEIGHT, __FILE__, NULL, NULL);


    glfwMakeContextCurrent(window);

    glfwSetWindowSizeCallback(window,reshape);
    reshape(window,WIDTH,HEIGHT);

    printf("GL_VERSION  : %s\n", glGetString(GL_VERSION) );
    printf("GL_RENDERER : %s\n", glGetString(GL_RENDERER) );
}





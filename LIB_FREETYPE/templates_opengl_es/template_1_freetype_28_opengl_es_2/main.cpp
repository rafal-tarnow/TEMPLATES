#include <iostream>
using namespace std;

#include "opengl_includes.hpp"
#include "TextRenderer.hpp"

static const GLuint WIDTH = 800;
static const GLuint HEIGHT = 600;
GLFWwindow* window;
void InitGLFWWindow();

TextRenderer * textRenderer;

void init(){
    textRenderer = new TextRenderer();
    textRenderer->Load("./data/font/arial.ttf", 10);
}

void reshape(GLFWwindow * window, int width, int height){
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, width, height);
}

void renderFunction(){
    //cout << "renderFunction()" << endl;
    glClear(GL_COLOR_BUFFER_BIT );
    textRenderer->RenderText();
}


int main()
{
    InitGLFWWindow();

    init();

    //MAIN LOOP
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        {
            renderFunction();
        }
        glfwSwapBuffers(window);
    }

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



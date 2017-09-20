#include <iostream>
using namespace std;

#include <cstdint>

#include "opengl_includes.hpp"
#include <library_opengles_2/TextRenderer/TextRenderer_v1.hpp>

static const GLuint WIDTH = 640;
static const GLuint HEIGHT = 480;
GLFWwindow* window;
void InitGLFWWindow();

TextRenderer_v1 * textRenderer;

void init(){
    textRenderer = new TextRenderer_v1(WIDTH,HEIGHT);
    //textRenderer->Load("./data/font/arial.ttf", 375);
    textRenderer->Load("/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf", 35);
}

void reshape(GLFWwindow * window, int width, int height){
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glViewport(0, 0, width, height);
}

void renderFunction(){
    //cout << "renderFunction()" << endl;
    glClear(GL_COLOR_BUFFER_BIT);
    textRenderer->RenderText("W");
}


int main()
{

    //wchar_t znaki[10] = {"Ą"};
   // znaki[7] = 'Ą';

    //char *wsk;
    //wsk = (char*)(&(znaki[0]));
    //for(unsigned int i = 0; i < 10; i++){
    //    cout << " " << (unsigned int)(wsk[i]) ;
   // }
   // cout << endl;

   // cout << znaki << endl;

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



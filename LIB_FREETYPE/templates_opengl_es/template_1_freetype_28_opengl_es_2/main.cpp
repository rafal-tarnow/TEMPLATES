#include <iostream>
using namespace std;

#include <cstdint>
#include <iomanip>
#include <sstream>

#include "opengl_includes.hpp"
#include <library_opengles_2/TextRenderer/TextRenderer_v2.hpp>
#include <library_opengles_2/TextRenderer/TextRenderer_v1.hpp>

static const GLuint WIDTH = 640*2;
static const GLuint HEIGHT = 480;
GLFWwindow* window;
void InitGLFWWindow();
void renderFunction();
double calculateFPS();
void initFPS();

TextRenderer_v2 * textRenderer_v2 = nullptr;
TextRenderer_v1 * textRenderer_v1 = nullptr;

void init(){
    textRenderer_v1 = new TextRenderer_v1(WIDTH,HEIGHT);
    //textRenderer_v1 = new TextRenderer_v1(WIDTH,HEIGHT);
    const GLuint fontSize = 110;
    //textRenderer_v2->Load("./data/font/arial.ttf", fontSize);
    //textRenderer_v2->Load("/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf", fontSize);
    //textRenderer_v2->Load("./data/font/dahot_Garfield_www_myfontfree_com.ttf", fontSize);
    textRenderer_v1->Load("./data/font/design_graffiti_agentorange_www_myfontfree_com.ttf", fontSize);

    //textRenderer_v1->Load("./data/font/arial.ttf", fontSize);
    //textRenderer_v1->Load("/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf", fontSize);
    //textRenderer_v1->Load("./data/font/dahot_Garfield_www_myfontfree_com.ttf", fontSize);
    //textRenderer_v1->Load("./data/font/design_graffiti_agentorange_www_myfontfree_com.ttf", fontSize);
}

void reshape(GLFWwindow * window, int width, int height){
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, width, height);

    //textRenderer_v1->onVievportResize(width, height);
    textRenderer_v1->onVievportResize(width, height);

}

void renderFunction(){
    //cout << "renderFunction()" << endl;
    glClear(GL_COLOR_BUFFER_BIT);



//    textRenderer_v1->RenderText("!", 80.0f, 440.0f);
//    textRenderer_v1->RenderText("Text = # dupa", 80.0f, 340.0f);
//    //textRenderer_v1->RenderText("}", 80.0f, 240.0f);
//    textRenderer_v1->RenderText(ss.str(), 80.0f, 140.0f);

    //textRenderer_v2->RenderText("!", 80.0f, 440.0f);
    //textRenderer_v2->RenderText("Text = # dupa", 80.0f, 340.0f);
    //textRenderer_v2->RenderText("}", 80.0f, 240.0f);
    textRenderer_v1->RenderText("AZCDEFGHIJKLMNOPQRSTUWXYZABCDEFGHIJKLMNOPQRSTUWXYZABCDEFGHIJKLMNOPQRSTUWXYZ", 80.0f, 140.0f);

}


int main()
{

    InitGLFWWindow();


    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);


    while (!glfwWindowShouldClose(window)) {
        cout << "FPS = "  << calculateFPS() << endl;


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

    init();
    reshape(window,WIDTH,HEIGHT);

    printf("GL_VERSION  : %s\n", glGetString(GL_VERSION) );
    printf("GL_RENDERER : %s\n", glGetString(GL_RENDERER) );

    initFPS();
}



#define SIZE 2200
static double tablica[SIZE];
static int index = 0;
static double suma = 0.0;

void initFPS(){
    for(int i = 0; i < SIZE; i++){
        tablica[i] = 0.0;
    }
}




double calculateFPS(){
    static double current_time = 0;
    static double previous_time = 0;
    static double delta_time = 0;
    static double fps = 0;


    current_time = glfwGetTime();
    delta_time = current_time - previous_time;
    previous_time = current_time;

    fps = 1.0/delta_time;


    double tmp = tablica[index];

    tablica[index] = fps;
    index++;

    if(index == SIZE)
        index = 0;

    suma = suma + fps - tmp;
    fps  = suma/SIZE;


    return fps;
}


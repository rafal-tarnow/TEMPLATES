#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <sstream>
std::stringstream msg;

#include "FreeCameraWrapper/FreeCameraWrapper.hpp"

using namespace std;

//rozmiar okna
const int WIDTH  = 1280;
const int HEIGHT = 960;

//zmienne zwišzane z upływem czasu
float last_time=0, current_time =0;
float delta_time = 0.0f;

FreeCameraWrapper * freeCameraWrapper;

void OnMouseMove(int x, int y){
    //freeCameraWrapper->systemInput_OnMouseMove(x,y);
    glutPostRedisplay();
}

void OnMouseButtonEvent(int button, int s, int x, int y){
    cout << "GLUT OnMouseDown(int button, int s, int x, int y)" << endl;
    //freeCameraWrapper->systemInput_OnMouseButtonEvent(button, s, x, y);
}

void OnShutdown() {
    //delete freeCameraWrapper;
}

void OnResize(int w, int h) {
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
    freeCameraWrapper->systemInput_OnResize(w, h);
}

void OnIdle() {
    //freeCameraWrapper->systemInput_OnIdle();
    glutPostRedisplay();
}

void OnKeyDown(unsigned char key, int x, int y)
{
    //freeCameraWrapper->systemInput_OnKeyEvent(key, FreeCameraWrapper::DOWN, x,y);
    glutPostRedisplay();
}

void OnKeyUp(unsigned char key, int x, int y)
{
    //freeCameraWrapper->systemInput_OnKeyEvent(key, FreeCameraWrapper::UP, x, y);
    glutPostRedisplay();
}

void OnRender() {
    //obliczenia zwišzane z czasem
    last_time = current_time;
    current_time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
    delta_time = current_time-last_time;

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    {

        freeCameraWrapper->systemInput_Render(0.01);
    }
    glutSwapBuffers();
}

void createGlutWindow(int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitContextVersion (3, 3);
    glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Kamera swobodna - OpenGL 3.3");
}

void InicjalizacjaGLEW(){
    //inicjalizacja glew
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)	{
        cerr<<"Blad: "<<glewGetErrorString(err)<<endl;
    } else {
        if (GLEW_VERSION_3_3)
        {
            cout<<"Sterownik obsluguje OpenGL 3.3\nSzczegoly:"<<endl;
        }
    }
    err = glGetError(); //w celu ignorowania błędu 1282 INVALID ENUM
    GL_CHECK_ERRORS

            //wyprowadzanie informacji na ekran
            cout<<"\tWersja GLEW "<<glewGetString(GLEW_VERSION)<<endl;
    cout<<"\tProducent: "<<glGetString (GL_VENDOR)<<endl;
    cout<<"\tRenderer: "<<glGetString (GL_RENDERER)<<endl;
    cout<<"\tWersja OpenGL: "<<glGetString (GL_VERSION)<<endl;
    cout<<"\tGLSL: "<<glGetString (GL_SHADING_LANGUAGE_VERSION)<<endl;

    GL_CHECK_ERRORS
}

void setGlutCallback()
{
    glutCloseFunc(OnShutdown);
    glutDisplayFunc(OnRender);
    glutReshapeFunc(OnResize);
    glutMouseFunc(OnMouseButtonEvent);
    glutMotionFunc(OnMouseMove);
    glutKeyboardFunc(OnKeyDown);
    glutKeyboardUpFunc(OnKeyUp);
    glutIdleFunc(OnIdle);
}

int main(int argc, char** argv)
{
    createGlutWindow(argc, argv);
    InicjalizacjaGLEW();

    freeCameraWrapper = new FreeCameraWrapper();

    setGlutCallback();

    glutMainLoop();
    return 0;
}

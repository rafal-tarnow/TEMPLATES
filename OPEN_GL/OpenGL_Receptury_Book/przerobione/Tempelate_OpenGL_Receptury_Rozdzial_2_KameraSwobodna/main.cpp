#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLSLShader.h"



#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")

using namespace std;

//rozmiar okna
const int WIDTH  = 1280;
const int HEIGHT = 960;

//do ustalania dokładnoci obliczeń
const float EPSILON = 0.001f;
const float EPSILON2 = EPSILON*EPSILON;

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=0, rY=0, fov = 45;

#include "FreeCamera.h"

//kody klawiszy
const int VK_W = 0x57;
const int VK_S = 0x53;
const int VK_A = 0x41;
const int VK_D = 0x44;
const int VK_Q = 0x51;
const int VK_Z = 0x5a;

//przyrost czasu
float dt = 0;

//zmienne zwišzane z upływem czasu
float last_time=0, current_time =0;


CFreeCamera cam;

//zmienne zwišzane z wygładzaniem ruchów myszy
const float MOUSE_FILTER_WEIGHT=0.75f;
const int MOUSE_HISTORY_BUFFER_SIZE = 10;
glm::vec2 mouseHistory[MOUSE_HISTORY_BUFFER_SIZE];
float mouseX=0, mouseY=0; //wygładzone wpółrzędne myszy
bool useMouseFiltering = true;

//wyjcie informacyjne
#include <sstream>
std::stringstream msg;

//ID tekstury w formie szachownicy
GLuint checkerTextureID;


//obiekt płaskiej szachownicy
#include "TexturedPlane.h"
CTexturedPlane* checker_plane;

//funkcja wygładzajšca ruchy myszy
void filterMouseMoves(float dx, float dy) {
    for (int i = MOUSE_HISTORY_BUFFER_SIZE - 1; i > 0; --i) {
        mouseHistory[i] = mouseHistory[i - 1];
    }

    // zapisywanie bieżšcego położenia myszy do bufora historii
    mouseHistory[0] = glm::vec2(dx, dy);

    float averageX = 0.0f;
    float averageY = 0.0f;
    float averageTotal = 0.0f;
    float currentWeight = 1.0f;

    // wygładzanie
    for (int i = 0; i < MOUSE_HISTORY_BUFFER_SIZE; ++i)
    {
        glm::vec2 tmp=mouseHistory[i];
        averageX += tmp.x * currentWeight;
        averageY += tmp.y * currentWeight;
        averageTotal += 1.0f * currentWeight;
        currentWeight *= MOUSE_FILTER_WEIGHT;
    }

    mouseX = averageX / averageTotal;
    mouseY = averageY / averageTotal;

}

//obsługa kliknięcia przyciskiem myszy
void OnMouseDown(int button, int s, int x, int y)
{
    if (s == GLUT_DOWN)
    {
        oldX = x;
        oldY = y;
    }

    if(button == GLUT_MIDDLE_BUTTON)
        state = 0;
    else
        state = 1;
}

//obsługa ruchów myszy
void OnMouseMove(int x, int y)
{
    if (state == 0) {
        fov += (y - oldY)/5.0f;
        cam.SetupProjection(fov, cam.GetAspectRatio());
    } else {
        rY -= (y - oldY)/5.0f;
        rX -= (oldX-x)/5.0f;
        if(useMouseFiltering)
            filterMouseMoves(rX, rY);
        else {
            mouseX = rX;
            mouseY = rY;
        }
        cam.Rotate(mouseX,mouseY, 0);
    }
    oldX = x;
    oldY = y;

    glutPostRedisplay();
}

void prepareChessboard(){
    GL_CHECK_ERRORS
            GLubyte szachownica[128][128]={0};
    for(int j=0;j<128;j++) {
        for(int i=0;i<128;i++) {
            szachownica[i][j]=(i<=64 && j<=64 || i>64 && j>64 )?255:0;
        }
    }
    //generowanie obiektu szachownicy
    glGenTextures(1, &checkerTextureID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, checkerTextureID);
    //ustalanie parametrów szachownicy
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    GL_CHECK_ERRORS
            //ustawianie parametrów anizotropii
            GLfloat largest_supported_anisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);
    //ustawianie poziomów mipmapy, podstawowego i maksymalnego
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
    //alokowanie obiektu tekstury
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED, 128, 128, 0, GL_RED, GL_UNSIGNED_BYTE, szachownica);
    //generowanie mipmapy
    glGenerateMipmap(GL_TEXTURE_2D);
    GL_CHECK_ERRORS
            //tworzenie obiektu teksturowanej płaszczyzny
            checker_plane = new CTexturedPlane();
    GL_CHECK_ERRORS
}

void prepareCamera(){
    //przygotowywanie kamery
    //ustalenie położenia i orientacji kamery
    glm::vec3 p = glm::vec3(5);
    cam.SetPosition(p);
    glm::vec3 look =  glm::normalize(p);

    //obracanie kamery we właciwym kierunku
    float yaw = glm::degrees(float(atan2(look.z, look.x)+M_PI));
    float pitch = glm::degrees(asin(look.y));
    rX = yaw;
    rY = pitch;
    if(useMouseFiltering) {
        for (int i = 0; i < MOUSE_HISTORY_BUFFER_SIZE ; ++i) {
            mouseHistory[i] = glm::vec2(rX, rY);
        }
    }
    cam.Rotate(rX,rY,0);
    cout<<"Inicjalizacja powiodla sie"<<endl;
}

void OnInit() {
    prepareChessboard();
    prepareCamera();
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {

    delete checker_plane;
    glDeleteTextures(1, &checkerTextureID);
    cout<<"Shutdown successfull"<<endl;
}

//obsługa zmiany wymiarów okna
void OnResize(int w, int h) {
    //wymiary okna widokowego
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
    //macierz rzutowania kamery
    cam.SetupProjection(45, (GLfloat)w/h);
}

bool Key_w_Pressed = false;
bool Key_s_Pressed = false;
bool Key_a_Pressed = false;
bool Key_d_Pressed = false;
bool Key_q_Pressed = false;
bool Key_e_Pressed = false;

//reakcja na sygnał bezczynnoci procesora
void OnIdle() {

    //obsługa klawiszy W,S,A,D,Q i Z służšcych do poruszania kamerš
    if( Key_w_Pressed) {
        cam.Walk(dt);
    }

    if( Key_s_Pressed) {
        cam.Walk(-dt);
    }

    if(Key_a_Pressed) {
        cam.Strafe(-dt);
    }

    if(Key_d_Pressed) {
        cam.Strafe(dt);
    }

    if(Key_q_Pressed) {
        cam.Lift(dt);
    }

    if(Key_e_Pressed) {
        cam.Lift(-dt);
    }


    glm::vec3 t = cam.GetTranslation();
    if(glm::dot(t,t)>EPSILON2) {
        cam.SetTranslation(t*0.95f);
    }

    //wywołanie funkcji wywietlajšcej
    glutPostRedisplay();
}



//obsługa wcinięcia klawisza spacji włšczajšcego tryb wygładzania ruchów myszy
void OnKeyDown(unsigned char key, int x, int y) {
    cout << "Key Down" << endl;

    switch(key) {
    case ' ':
        useMouseFiltering = !useMouseFiltering;
        break;
    case 'w':
        Key_w_Pressed = true;
        break;
    case 's':
        Key_s_Pressed = true;
        break;
    case 'a':
        Key_a_Pressed = true;
        break;
    case 'd':
        Key_d_Pressed = true;
        break;
    case 'q':
        Key_q_Pressed = true;
        break;
    case 'e':
        Key_e_Pressed = true;
        break;
    }
    glutPostRedisplay();
}

void OnKeyUp(unsigned char key, int x, int y){
    cout << "Key Up" << endl;

    switch(key) {
    case 'w':
        Key_w_Pressed = false;
        break;
    case 's':
        Key_s_Pressed = false;
        break;
    case 'a':
        Key_a_Pressed = false;
        break;
    case 'd':
        Key_d_Pressed = false;
        break;
    case 'q':
        Key_q_Pressed = false;
        break;
    case 'e':
        Key_e_Pressed = false;
        break;
    }
    glutPostRedisplay();
}


//funkcja zwrotna wywietlania
void OnRender() {
    //obliczenia zwišzane z czasem
    last_time = current_time;
    current_time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
    dt = current_time-last_time;

    //czyszczenie buforów koloru i głębi
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    //transformacje kamery
    glm::mat4 MV	= cam.GetViewMatrix();
    glm::mat4 P     = cam.GetProjectionMatrix();
    glm::mat4 MVP	= P*MV;

    //renderowanie płaskiej szachownicy
    checker_plane->Render(glm::value_ptr(MVP));

    //zamiana buforów w celu wywietlenia obrazu
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

void setGlutCallback(){
    //rejestracja funkcji zwrotnych
    glutCloseFunc(OnShutdown);
    glutDisplayFunc(OnRender);
    glutReshapeFunc(OnResize);
    glutMouseFunc(OnMouseDown);
    glutMotionFunc(OnMouseMove);
    glutKeyboardFunc(OnKeyDown);
    glutKeyboardUpFunc(OnKeyUp);
    glutIdleFunc(OnIdle);
}

int main(int argc, char** argv) {
    //inicjalizacja freeglut

    createGlutWindow(argc, argv);

    InicjalizacjaGLEW();

    OnInit();

    setGlutCallback();


    //wywołanie pętli głównej
    glutMainLoop();

    return 0;
}

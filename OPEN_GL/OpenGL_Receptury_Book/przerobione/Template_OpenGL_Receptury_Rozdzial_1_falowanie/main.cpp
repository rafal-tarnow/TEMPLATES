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

//wymiary okna
const int WIDTH  = 1280;
const int HEIGHT = 960;

//referencja shadera
GLSLShader shader;

//ID tablicy wierzchołków i obiektu bufora wierzchołków
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

const int NUM_X = 40; //liczba czworokštów wzdłuż osi X
const int NUM_Z = 40; //liczba czworokštów wzdłuż osi Z

const float SIZE_X = 4; //wymiary płaszczyzny w przestrzeni wiata
const float SIZE_Z = 4;
const float HALF_SIZE_X = SIZE_X/2.0f;
const float HALF_SIZE_Z = SIZE_Z/2.0f;

//szybkoć fali
const float SPEED = 2;

//wierzchołki i indeksy siatki
glm::vec3 vertices[(NUM_X+1)*(NUM_Z+1)];
const int TOTAL_INDICES = NUM_X*NUM_Z*2*3;
GLushort indices[TOTAL_INDICES];

//macierze rzutowania oraz modelu i widoku
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=25, rY=-40, dist = -7;

//bieżšcy czas
float mtime = 0;

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

//obsługa ruchu myszy
void OnMouseMove(int x, int y)
{
    if (state == 0)
        dist *= (1 + (y - oldY)/60.0f);
    else
    {
        rY += (x - oldX)/5.0f;
        rX += (y - oldY)/5.0f;
    }
    oldX = x;
    oldY = y;

    glutPostRedisplay();
}

//Inicjalizacja OpenGL
void OnInit() {
    //ustawienie trybu wielokštów do rysowania linii
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GL_CHECK_ERRORS
    //wczytanie shaderów
    shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/shader.vert");
    shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/shader.frag");
    //kompilacja i konsolidacja programu shaderowego
    shader.CreateAndLinkProgram();
    shader.Use();
        //dodawanie atrybutów i uniformów
        shader.AddAttribute("vVertex");
        shader.AddUniform("MVP");
        shader.AddUniform("mtime");
    shader.UnUse();

    GL_CHECK_ERRORS

    //ustalenie geometrii płaszczyzny
    //ustalenie wierzchołków płaszczyzny
    int count = 0;
    int i=0, j=0;
    for( j=0;j<=NUM_Z;j++) {
        for( i=0;i<=NUM_X;i++) {
            vertices[count++] = glm::vec3( ((float(i)/(NUM_X-1)) *2-1)* HALF_SIZE_X, 0, ((float(j)/(NUM_Z-1))*2-1)*HALF_SIZE_Z);
        }
    }

    //wypełnianie tablicy indeksów płaszczyzny
    GLushort* id=&indices[0];
    for (i = 0; i < NUM_Z; i++) {
        for (j = 0; j < NUM_X; j++) {
            int i0 = i * (NUM_X+1) + j;
            int i1 = i0 + 1;
            int i2 = i0 + (NUM_X+1);
            int i3 = i2 + 1;
            if ((j+i)%2) {
                *id++ = i0; *id++ = i2; *id++ = i1;
                *id++ = i1; *id++ = i2; *id++ = i3;
            } else {
                *id++ = i0; *id++ = i2; *id++ = i3;
                *id++ = i0; *id++ = i3; *id++ = i1;
            }
        }
    }

    GL_CHECK_ERRORS

    //przygotowanie vao i vbo dla płaszczyzny
    glGenVertexArrays(1, &vaoID);
    glGenBuffers(1, &vboVerticesID);
    glGenBuffers(1, &vboIndicesID);

    glBindVertexArray(vaoID);

        glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
        //przekazanie wierzchołków płaszczyzny do obiektu bufora
        glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
        GL_CHECK_ERRORS
        //włšczenie tablicy atrybutów wierzhołka dla położenia
        glEnableVertexAttribArray(shader["vVertex"]);
        glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);
        GL_CHECK_ERRORS
        //pprzekazanie indeksów do bufora tablicy elementów
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
        GL_CHECK_ERRORS

    cout<<"Inicjalizacja powiodla sie"<<endl;
}


//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {
    //likwidacja shadera
    shader.DeleteShaderProgram();

    //Likwidacja vao i vbo
    glDeleteBuffers(1, &vboVerticesID);
    glDeleteBuffers(1, &vboIndicesID);
    glDeleteVertexArrays(1, &vaoID);

    cout<<"Zamknięcie powiodlo sie"<<endl;
}

//obsługa zdarzenia zmiany wymiarów okna
void OnResize(int w, int h) {
    //ustalanie wymiarów okna widokowego
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
    //ustawianie macierzy rzutowania
    P = glm::perspective(45.0f, (GLfloat)w/h, 1.f, 1000.f);
}

//obsługa zdarzenia bezczynnoci
void OnIdle() {
    glutPostRedisplay();
}

//funkcja zwrotna wywietlania
void OnRender() {
    //wyznaczanie upływajšcego czasu
    mtime = glutGet(GLUT_ELAPSED_TIME)/1000.0f * SPEED;

    //czyszczenie buforów koloru i głębi
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    //transformacje kamery
    glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
    glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 MV	= glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 MVP	= P*MV;

    //wišzanie shadera
    shader.Use();
        //przekazywanie uniformów
        glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
        glUniform1f(shader("mtime"), mtime);
            //rysowanie trójkštów siatki
            glDrawElements(GL_TRIANGLES, TOTAL_INDICES, GL_UNSIGNED_SHORT, 0);

    //odwišzywanie shadera
    shader.UnUse();

    //zamiana buforów w celu wywietlenia obrazu
    glutSwapBuffers();
}

int main(int argc, char** argv) {
    //inicjalizacja freeglut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitContextVersion (3, 3);
    glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Falowanie - OpenGL 3.3");

    //inicjalizacja glew
    glewExperimental = GL_TRUE;

    GLenum err = glewInit();
    if (GLEW_OK != err)	{
        cerr<<"Blad: "<<glewGetErrorString(err)<<endl;
    } else {
        if (GLEW_VERSION_3_3)
        {
            cout<<"Sterownik obsługuje OpenGL 3.3\nSzczegóły:"<<endl;
        }
    }
    err = glGetError(); //w celu ignorowania błędu 1282 INVALID ENUM
    GL_CHECK_ERRORS

    //wyprowadzanie informacji na ekran
    cout<<"\tWersja GLEW "<<glewGetString(GLEW_VERSION)<<endl;
    cout<<"\tProducent: "<<glGetString (GL_VENDOR)<<endl;
    cout<<"\tRenderer: "<<glGetString (GL_RENDERER)<<endl;
    cout<<"\tWersja GL: "<<glGetString (GL_VERSION)<<endl;
    cout<<"\tGLSL: "<<glGetString (GL_SHADING_LANGUAGE_VERSION)<<endl;

    GL_CHECK_ERRORS

    //inicjalizacja OpenGL
    OnInit();

    //rejestracja funkcji zwrotnych
    glutCloseFunc(OnShutdown);
    glutDisplayFunc(OnRender);
    glutReshapeFunc(OnResize);
    glutMouseFunc(OnMouseDown);
    glutMotionFunc(OnMouseMove);
    glutIdleFunc(OnIdle);

    //wywołanie głównej pętli
    glutMainLoop();

    return 0;
}

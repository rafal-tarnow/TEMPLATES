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

//wyjciowa struktura wierzchołków dla atrybutów przeplatanych
struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
};

//wierzchołki i indeksy trójkšta
Vertex vertices[3];
GLushort indices[3];

//macierze rzutowania oraz modelu i widoku
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//Inicjalizacja OpenGL
void OnInit() {
    GL_CHECK_ERRORS
    //wczytanie shaderów
    shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/shader.vert");
    shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/shader.frag");
    //kompilacja i konsolidacja programu shaderowego
    shader.CreateAndLinkProgram();
    shader.Use();
        //dodawanie atrybutów i uniformów
        shader.AddAttribute("vVertex");
        shader.AddAttribute("vColor");
        shader.AddUniform("MVP");
    shader.UnUse();

    GL_CHECK_ERRORS

    //ustalenie geometrii trójkšta
    //Ustawienie wierzchołków trójkšta
    vertices[0].color=glm::vec3(1,0,0);
    vertices[1].color=glm::vec3(0,1,0);
    vertices[2].color=glm::vec3(0,0,1);

    vertices[0].position=glm::vec3(-1,-1,0);
    vertices[1].position=glm::vec3(0,1,0);
    vertices[2].position=glm::vec3(1,-1,0);

    //Ustalenie indeksów trójkšta
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;

    GL_CHECK_ERRORS

    //przygotowanie vao i vbo dla trójkšta
    glGenVertexArrays(1, &vaoID);
    glGenBuffers(1, &vboVerticesID);
    glGenBuffers(1, &vboIndicesID);
    GLsizei stride = sizeof(Vertex);

    glBindVertexArray(vaoID);

        glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
        //przekazanie wierzchołków trójkšta do obiektu bufora
        glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
        GL_CHECK_ERRORS
        //włšczenie tablicy atrybutów wierzhołka dla położenia
        glEnableVertexAttribArray(shader["vVertex"]);
        glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,stride,0);
        GL_CHECK_ERRORS
        //włšczenie tablicy atrybutów wierzhołka dla koloru
        glEnableVertexAttribArray(shader["vColor"]);
        glVertexAttribPointer(shader["vColor"], 3, GL_FLOAT, GL_FALSE,stride, (const GLvoid*)offsetof(Vertex, color));
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
    P = glm::ortho(-1,1,-1,1);
}

//funkcja zwrotna wywietlania
void OnRender() {

    //czyszczenie buforów koloru i głębi
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    //wišzanie shadera
    shader.Use();
        //przekazywanie uniformów
        glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV));
            //rysowanie trójkšta
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
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
    glutCreateWindow("Prosty trójkąt - OpenGL 3.3");

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

    //wywołanie głównej pętli
    glutMainLoop();

    return 0;
}

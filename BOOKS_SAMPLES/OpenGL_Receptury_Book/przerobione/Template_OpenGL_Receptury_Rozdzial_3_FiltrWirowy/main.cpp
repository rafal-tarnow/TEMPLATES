#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL/SOIL.h>

#include "GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace std;

//wymiary okna
const int WIDTH  = 512;
const int HEIGHT = 512;

//shader
GLSLShader shader;

//ID tablicy i obiektu bufora wierzcho³ków
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

//ID obrazu tekstury
GLuint textureID;

//wierzcho³ki i indeksy dla pe³noekranowego czworok¹ta
glm::vec2 vertices[4];
GLushort indices[6];

//nazwa pliku z obrazem tekstury
const string filename = "./data/media/Lenna.png";

//moc filtra
float twirl_amount = 0;

//Inicjalizacja OpenGL
void OnInit() {
    GL_CHECK_ERRORS
    //wczytanie shaderów
    shader.LoadFromFile(GL_VERTEX_SHADER, "data/shadery/shader.vert");
    shader.LoadFromFile(GL_FRAGMENT_SHADER, "data/shadery/shader.frag");
    //kompilacja i konsolidacja programu shaderowego
    shader.CreateAndLinkProgram();
    shader.Use();
        //atrybuty i uniformy
        shader.AddAttribute("vVertex");
        shader.AddUniform("textureMap");
        shader.AddUniform("twirl_amount");
        //inicjalizacja sta³ych uniformów
        glUniform1i(shader("textureMap"), 0);
        glUniform1f(shader("twirl_amount"), twirl_amount);
    shader.UnUse();

    GL_CHECK_ERRORS

    //Ustalanie geometrii czworok¹ta
    //ustalanie wierzcho³ków czworok¹ta
    vertices[0] = glm::vec2(0.0,0.0);
    vertices[1] = glm::vec2(1.0,0.0);
    vertices[2] = glm::vec2(1.0,1.0);
    vertices[3] = glm::vec2(0.0,1.0);

    //wype³nianie tablicy indeksów czworok¹ta
    GLushort* id=&indices[0];
    *id++ =0;
    *id++ =1;
    *id++ =2;
    *id++ =0;
    *id++ =2;
    *id++ =3;

    GL_CHECK_ERRORS

    //przygotowanie vao i vbo dla czworok¹ta
    glGenVertexArrays(1, &vaoID);
    glGenBuffers(1, &vboVerticesID);
    glGenBuffers(1, &vboIndicesID);

    glBindVertexArray(vaoID);

        glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
        //przekazanie wierzcho³ków czworok¹ta do obiektu bufora
        glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
        GL_CHECK_ERRORS
        //w³¹czenie tablicy atrybutów wierzho³ka dla po³o¿enia
        glEnableVertexAttribArray(shader["vVertex"]);
        glVertexAttribPointer(shader["vVertex"], 2, GL_FLOAT, GL_FALSE,0,0);
        GL_CHECK_ERRORS
        //przekazanie indeksów czworok¹ta do bufora tablicy elementów
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
        GL_CHECK_ERRORS


    //wczytanie obrazu
    int texture_width = 0, texture_height = 0, channels=0;
    GLubyte* pData = SOIL_load_image(filename.c_str(), &texture_width, &texture_height, &channels, SOIL_LOAD_AUTO);

    //odwracanie obrazu wzglêdem osi Y
    int i,j;
    for( j = 0; j*2 < texture_height; ++j )
    {
        int index1 = j * texture_width * channels;
        int index2 = (texture_height - 1 - j) * texture_width * channels;
        for( i = texture_width * channels; i > 0; --i )
        {
            GLubyte temp = pData[index1];
            pData[index1] = pData[index2];
            pData[index2] = temp;
            ++index1;
            ++index2;
        }
    }

    //przygotowanie tekstury i zwi¹zanie jej jednostk¹ teksturuj¹c¹ o numerze 0
    glGenTextures(1, &textureID);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        //ustawienie parametrów tekstury
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        //alokowanie tekstury
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pData);

    //free SOIL image data
    SOIL_free_image_data(pData);

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

    //usuwanie tekstur
    glDeleteTextures(1, &textureID);
    cout<<"Zamkniecie powiodlo sie"<<endl;
}

//obs³uga zmiany wymiarów okna
void OnResize(int w, int h) {
    //ustalanie wymiarów okna widokowego
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
}

//funkcja wywietlania
void OnRender() {
    //czyszczenie buforów koloru i g³êbi
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    //wi¹zanie shadera
    shader.Use();
        // uniform
        glUniform1f(shader("twirl_amount"), twirl_amount);
            //rysowanie pe³noekranowego czworok¹ta
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    //odwi¹zywanie shadera
    shader.UnUse();

    //zamiana buforów w celu wywietlenia obrazu
    glutSwapBuffers();
}

//obs³uga klawiszy zmieniaj¹cych moc filtra
void OnKey(unsigned char key, int x, int y) {
    switch(key) {
        case '-': twirl_amount-=0.1f; break;
        case '+': twirl_amount+=0.1f; break;
    }
    //call display function
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    //inicjalizacja freeglut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitContextVersion (3, 3);
    glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Filtr wirowy - OpenGL 3.3");

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
    err = glGetError(); //w celu ignorowania b³êdu 1282 INVALID ENUM
    GL_CHECK_ERRORS

    //wyprowadzanie informacji na ekran
    cout<<"\tWersja GLEW "<<glewGetString(GLEW_VERSION)<<endl;
    cout<<"\tProducent: "<<glGetString (GL_VENDOR)<<endl;
    cout<<"\tRenderer: "<<glGetString (GL_RENDERER)<<endl;
    cout<<"\tWersja OpenGL: "<<glGetString (GL_VERSION)<<endl;
    cout<<"\tGLSL: "<<glGetString (GL_SHADING_LANGUAGE_VERSION)<<endl;

    cout<<"wcinij klawisz '-' aby zmniejszyæ moc filtra\n      '+' aby zwiêkszyæ moc filtra\n";
    GL_CHECK_ERRORS

    //inicjalizacja OpenGL
    OnInit();

    //rejestracja funkcji zwrotnych
    glutCloseFunc(OnShutdown);
    glutDisplayFunc(OnRender);
    glutReshapeFunc(OnResize);
    glutKeyboardFunc(OnKey);

    //wywo³anie pêtli g³ównej
    glutMainLoop();

    return 0;
}

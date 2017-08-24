#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <algorithm>

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
const int WIDTH  = 1280;
const int HEIGHT = 960;

//shader
GLSLShader shader;

//ID tablicy wierzcho³ków i obiektu bufora wierzcho³ków
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

//wierzcho³ki i indeksy siatki
glm::vec3 vertices[4];
GLushort indices[6];

//macierze modelu i widoku oraz rzutowania
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=25, rY=-40, dist = -35;

//liczba podzia³ów
int sub_divisions = 4;

//macierz modelu dla ka¿dej instancji
glm::mat4 M[4];

//obs³uga klikniêcia przyciskiem myszy
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

//obs³uga ruchu myszy
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

//Obs³uga wciœniêæ klawiszy w celu zmiany liczby podzia³ów
void OnKey(unsigned char key, int x, int y) {
	switch(key) {
		case ',':	sub_divisions--; break;
		case '.':	sub_divisions++; break;
	}

	sub_divisions = max(1,min(8, sub_divisions));

	//Odœwie¿anie zawartoœci okna 
	glutPostRedisplay();
}

//Inicjalizacja OpenGL
void OnInit() {

	//przygotowanie macierzy modelu dla instancji
	M[0] = glm::translate(glm::mat4(1), glm::vec3(-5,0,-5));
	M[1] = glm::translate(M[0], glm::vec3(10,0,0));
	M[2] = glm::translate(M[1], glm::vec3(0,0,10));
	M[3] = glm::translate(M[2], glm::vec3(-10,0,0));

	GL_CHECK_ERRORS

	//wczytanie shaderów
	shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/shader.vert");
	shader.LoadFromFile(GL_GEOMETRY_SHADER, "shadery/shader.geom");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/shader.frag");
	//tworzenie i konsolidacja programu shaderowego
	shader.CreateAndLinkProgram();
	shader.Use();
		//dodawanie atrybutów i uniformów
		shader.AddAttribute("vVertex");
		shader.AddUniform("PV");
		shader.AddUniform("M");
		shader.AddUniform("sub_divisions");

		//ustawianie wartoœci sta³ych uniformów
		glUniform1i(shader("sub_divisions"), sub_divisions);
		glUniformMatrix4fv(shader("M"), 4, GL_FALSE, glm::value_ptr(M[0]));

	shader.UnUse();

	GL_CHECK_ERRORS

	//Ustalanie geometrii czworok¹ta
	//ustalanie wierzcho³ków czworok¹ta
	vertices[0] = glm::vec3(-5,0,-5);
	vertices[1] = glm::vec3(-5,0,5);
	vertices[2] = glm::vec3(5,0,5);
	vertices[3] = glm::vec3(5,0,-5);

	//ustalanie indeksów czworok¹ta
	GLushort* id=&indices[0];
 	*id++ = 0;
	*id++ = 1;
	*id++ = 2;

	*id++ = 0;
	*id++ = 2;
	*id++ = 3;

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
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);
		GL_CHECK_ERRORS
		//przekazanie indeksów czworok¹ta do bufora tablicy elementów
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS

	//ustawienie trybu renderowania linii
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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

	cout<<"Zamkniecie powiodlo sie"<<endl;
}

//obs³uga zdarzenia zmiany wymiarów okna
void OnResize(int w, int h) {
	//ustalanie wymiarów okna widokowego
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);

	//ustawianie macierzy rzutowania
	P = glm::perspective(45.0f, (GLfloat)w/h, 0.01f, 10000.f);
}

//funkcja wyœwietlania
void OnRender() {
	//czyszczenie buforów koloru i g³êbi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//transformacje kamery 
	glm::mat4 T	 = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx = glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 V	 = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 PV = P*V;

	//wi¹zanie shadera
	shader.Use();
		//ustawianie wartoœci uniformów shadera
		glUniformMatrix4fv(shader("PV"), 1, GL_FALSE, glm::value_ptr(PV));
		glUniform1i(shader("sub_divisions"), sub_divisions);
			//renderowanie instancji 
			glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, 4);
		//odwi¹zywanie shadera
		shader.UnUse();

	//zamiana buforów w celu wyœwietlenia obrazu
	glutSwapBuffers();
}

int main(int argc, char** argv) {
	//inicjalizacja freeglut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Rendering instancyjny - OpenGL 3.3");

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

	GL_CHECK_ERRORS
	
	//inicjalizacja OpenGL
	OnInit();
		
	//rejestracja funkcji zwrotnych
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutKeyboardFunc(OnKey);
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);

	//wywo³anie pêtli g³ównej
	glutMainLoop();

	return 0;
}

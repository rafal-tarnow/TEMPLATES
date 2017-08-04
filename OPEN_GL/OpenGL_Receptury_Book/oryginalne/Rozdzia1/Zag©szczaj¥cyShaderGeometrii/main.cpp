#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL.h>

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

//ID tablicy wierzcho�k�w i obiektu bufora wierzcho�k�w
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

//wierzcho�ki i indeksy siatki
glm::vec3 vertices[4];
GLushort indices[6];

//macierze rzutowania oraz modelu i widoku
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=25, rY=-40, dist = -35;
 
//liczba podzia��w
int sub_divisions = 1;

//obs�uga klikni�cia przyciskiem myszy
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

//obs�uga ruchu myszy
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

//Obs�uga wci�ni�� klawiszy w celu zmiany liczby podzia��w
void OnKey(unsigned char key, int x, int y) {
	switch(key) {
		case ',':	sub_divisions--; break;
		case '.':	sub_divisions++; break;
	}

	sub_divisions = max(1,min(8, sub_divisions));

	glutPostRedisplay();
}

//Inicjalizacja OpenGL
void OnInit() {
	GL_CHECK_ERRORS
	//wczytanie shader�w
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	shader.LoadFromFile(GL_GEOMETRY_SHADER, "shaders/shader.geom");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
	//tworzenie i konsolidacja programu shaderowego
	shader.CreateAndLinkProgram();
	shader.Use();
		//dodawanie atrybut�w i uniform�w
		shader.AddAttribute("vVertex");
		shader.AddUniform("MVP");
		shader.AddUniform("sub_divisions");
		//ustawianie warto�ci sta�ych uniform�w
		glUniform1i(shader("sub_divisions"), sub_divisions);
	shader.UnUse();

	GL_CHECK_ERRORS

	//Ustalanie geometrii czworok�ta
	//ustalanie wierzcho�k�w czworok�ta
	vertices[0] = glm::vec3(-5,0,-5);
	vertices[1] = glm::vec3(-5,0,5);
	vertices[2] = glm::vec3(5,0,5);
	vertices[3] = glm::vec3(5,0,-5);

	//ustalanie indeks�w czworok�ta
	GLushort* id=&indices[0];
 	*id++ = 0;
	*id++ = 1;
	*id++ = 2;

	*id++ = 0;
	*id++ = 2;
	*id++ = 3;

	GL_CHECK_ERRORS

	//przygotowanie vao i vbo dla czworok�ta
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);

	glBindVertexArray(vaoID);

		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		//przekazanie wierzcho�k�w czworok�ta do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//w��czenie tablicy atrybut�w wierzho�ka dla po�o�enia
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);
		GL_CHECK_ERRORS
		//pprzekazanie indeks�w czworok�ta do bufora tablicy element�w
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS

	//ustawienie trybu renderowania linii
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	GL_CHECK_ERRORS

	cout<<"Inicjalizacja powiodla sie"<<endl;
}


//zwalnianie wszystkich alokowanych zasob�w
void OnShutdown() {

	//likwidacja shadera
	shader.DeleteShaderProgram();

	//Likwidacja vao i vbo
	glDeleteBuffers(1, &vboVerticesID);
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID);

	cout<<"Zamkni�cie powiodlo sie"<<endl;
}

//obs�uga zdarzenia zmiany wymiar�w okna
void OnResize(int w, int h) {
	//ustalanie wymiar�w okna widokowego
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);

	//ustawianie macierzy rzutowania
	P = glm::perspective(45.0f, (GLfloat)w/h, 0.01f, 10000.f);
}

//funkcja zwrotna wy�wietlania 
void OnRender() {
	//czyszczenie bufor�w koloru i g��bi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//transformacje kamery
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV	= glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	MV=glm::translate(MV, glm::vec3(-5,0,-5));


	//wi�zanie shadera
	shader.Use();
		//ustawianie warto�ci uniform�w shadera
		glUniform1i(shader("sub_divisions"), sub_divisions);
		glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV));
			//rysowanie pierwszej siatki
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

		MV=glm::translate(MV, glm::vec3(10,0,0));
		glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV));
			//rysowanie drugiej siatki
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

		MV=glm::translate(MV, glm::vec3(0,0,10));
		glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV));
			//rysowanie trzeciej siatki
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

		MV=glm::translate(MV, glm::vec3(-10,0,0));
		glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV));
			//rysowanie czwartej siatki
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	//odwi�zywanie shadera
	shader.UnUse();
	
	//zamiana bufor�w w celu wy�wietlenia obrazu
	glutSwapBuffers();
}

int main(int argc, char** argv) {
	//inicjalizacja freeglut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Zag�szczaj�cy shader geometrii - OpenGL 3.3");

	//inicjalizacja glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)	{
		cerr<<"Error: "<<glewGetErrorString(err)<<endl;
	} else {
		if (GLEW_VERSION_3_3)
		{
			cout<<"Sterownik obs�uguje OpenGL 3.3\nSzczeg�y:"<<endl;
		}
	}
	err = glGetError(); //w celu ignorowania b��du 1282 INVALID ENUM
	GL_CHECK_ERRORS

	//wyprowadzanie informacji na ekran
	cout<<"\tUsing GLEW "<<glewGetString(GLEW_VERSION)<<endl;
	cout<<"\tVendor: "<<glGetString (GL_VENDOR)<<endl;
	cout<<"\tRenderer: "<<glGetString (GL_RENDERER)<<endl;
	cout<<"\tVersion: "<<glGetString (GL_VERSION)<<endl;
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

	//wywo�anie p�tli g��wnej
	glutMainLoop();

	return 0;
}
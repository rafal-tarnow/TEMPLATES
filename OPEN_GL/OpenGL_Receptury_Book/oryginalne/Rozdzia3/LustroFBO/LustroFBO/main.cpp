#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "..\..\src\GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")

using namespace std;

//wymiary okna
const int WIDTH  = 1280;
const int HEIGHT = 960;
 
//siatka
#include "..\..\src\Grid.h"
CGrid* grid;

//jednostkowa kolorowa kostka
#include "..\..\src\UnitColorCube.h"
CUnitColorCube* cube;

//czworok�t
#include "..\..\src\Quad.h"
CQuad* mirror;

//macierze rzutowania oraz modelu i widoku
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=25, rY=-40, dist = -7;

//ID FBO i obiektu bufora renderingu
GLuint fboID, rbID;

//ID tekstury renderowanej pozaekranowo
GLuint renderTextureID;

//macierz obrotu lokalnego
glm::mat4 localR=glm::mat4(1);

//k�t obrotu
float angle = 0;

//inicjalizacja FBO
void InitFBO() {
	//generowanie i wi�zanie ID obiektu bufora ramki
	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);

	//generowanie i wi�zanie ID bufora renderingu
	glGenRenderbuffers(1, &rbID);
	glBindRenderbuffer(GL_RENDERBUFFER, rbID);

	//ustawianie wielko�ci bufora renderingu
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, WIDTH, HEIGHT);

	// pozaekranowe generowanie tekstury
	glGenTextures(1, &renderTextureID);
	glBindTexture(GL_TEXTURE_2D, renderTextureID);

	//ustawianie parametr�w tekstury
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WIDTH, HEIGHT, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	//wi�zanie identyfikatora renderTextureID z przy��czem koloru w FBO
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTextureID, 0);
	//wi�zanie bufora renderingu z przy��czem g��bi w FBO
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER, rbID);

	//sprawdzanie kompletno�ci bufora ramki
	GLuint status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

	if(status==GL_FRAMEBUFFER_COMPLETE) {
		printf("Ustawianie FBO powiodlo sie.");
	} else {
		printf("Blad przy ustawianiu FBO.");
	}

	//odwi�zywanie tekstury i FBO
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

//zwalnianie alokowanych zasob�w FBO
void ShutdownFBO() {
	glDeleteTextures(1, &renderTextureID);
	glDeleteRenderbuffers(1, &rbID);
	glDeleteFramebuffers(1, &fboID);
}

//obs�uga klikni�cia mysz�
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

//obs�uga ruch�w myszy
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

	glEnable(GL_DEPTH_TEST);

	GL_CHECK_ERRORS

	//tworzenie siatki o wymiarach 20x20 w p�aszczy�nie XZ
	grid = new CGrid(20,20);

	//tworzenie kolorowej kostki
	cube = new CUnitColorCube();

	//tworzenie czworok�tnego lustra w po�o�eniu Z=-2 
	mirror = new CQuad(-2);

	//inicjalizacja FBO
	InitFBO();

	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich alokowanych zasob�w
void OnShutdown() {
	ShutdownFBO();

	delete grid;
	delete cube;
	delete mirror;

	cout<<"Zamkniecie powiodlo sie"<<endl;
}

//obs�uga zmiany wymiar�w okna
void OnResize(int w, int h) {
	//ustalanie wymiar�w okna widokowego
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//wyznaczanie macierzy rzutowania
	P = glm::perspective(45.0f, (GLfloat)w/h, 1.f, 1000.f);
}

//obs�uga sygna�u bezczynno�ci procesora
void OnIdle() {
	//zwi�kszanie k�ta i tworzenie macierzy obrotu lokalnego wok� osi Y
	angle += 0.5f; 
	localR=glm::rotate(glm::mat4(1), angle, glm::vec3(0,1,0));

	//wywo�anie wunkcji wy�wietlaj�cej
	glutPostRedisplay();
}

//funkcja zwrotna wy�wietlania
void OnRender() {

	//transformacje kamery
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 Ry	= glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 MV	= Ry;
	glm::mat4 MVP	= P*MV;

	//czyszczenie bufor�w koloru i g��bi 
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//zwyk�e renderowanie sceny	
	//renderowanie siatki
	grid->Render(glm::value_ptr(MVP));
	localR[3][1] = 0.5;
	
	//przesuwanie kostki wzd�u� osi Y do poziomu pod�o�a
	//i renderowanie jej
	cube->Render(glm::value_ptr(P*MV*localR));

	//zapisywanie bie��cej macierzy modelu i widoku
	glm::mat4 oldMV = MV;

	//dostosowanie macierzy widoku do miejsca zajmowanego przez lustro
	//odbijanie wektora kierunku patrzenia wzgl�dem normalnej lustra
	glm::vec3 V = glm::vec3(-MV[2][0], -MV[2][1], -MV[2][2]);
	glm::vec3 R = glm::reflect(V, mirror->normal);

	//umieszczenie kamery w po�o�eniu lustra
	MV = glm::lookAt(mirror->position, mirror->position + R, glm::vec3(0,1,0));

	//poniewa� obraz lustrzany ma zamienione strony, mno�ymy macierz MV przez (-1,1,1)
	MV = glm::scale(MV, glm::vec3(-1,1,1));

	//w��czenie FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);
	//renderowanie do przy��cza koloru 0
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
		//czyszczenie bufor�w koloru i g��bi
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		//pokazanie lustra z jednej tylko strony
		if(glm::dot(V,mirror->normal)<0) {
			grid->Render(glm::value_ptr(P*MV));
			cube->Render(glm::value_ptr(P*MV*localR));
		} 
	
	//odwi�zanie FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	
	//przywracanie domy�lnego bufora tylnego
	glDrawBuffer(GL_BACK_LEFT);

	//przywracanie dawnej macierzy modelu i widoku
	MV = oldMV;

	//wi�zanie bie��cej tekstury z wyj�ciem FBO
	glBindTexture(GL_TEXTURE_2D, renderTextureID);

	//renderowanie lustra
	mirror->Render(glm::value_ptr(P*MV));

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
	glutCreateWindow("Lustro przy u�yciu FBO - OpenGL 3.3");

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
	err = glGetError(); //w celu ignorowania b��du 1282 INVALID ENUM
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
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutIdleFunc(OnIdle);

	//wywo�anie p�tli g��wnej
	glutMainLoop();

	return 0;
}
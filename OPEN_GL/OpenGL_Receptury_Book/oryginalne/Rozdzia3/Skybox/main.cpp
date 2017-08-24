

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "..\src\GLSLShader.h"

#include <SOIL.h>

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace std;

//wymiary okna
const int WIDTH  = 1280;
const int HEIGHT = 960;

//macierze rzutowania oraz modelu i widoku
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=-3, rY=65, dist = -7;

//obiekt skybox 
#include "../src/skybox.h"
CSkybox* skybox;

//ID tekstury skyboksu
GLuint skyboxTextureID;

//nazwy sk³adników tekstury skyboksu
const char* texture_names[6] = {"../media/skybox/ocean/posx.png",
								"../media/skybox/ocean/negx.png",
								"../media/skybox/ocean/posy.png",
								"../media/skybox/ocean/negy.png",
								"../media/skybox/ocean/posz.png",
								"../media/skybox/ocean/negz.png"};
//obs³uga klikniêcia mysz¹
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

//obs³uga ruchów myszy
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

	//w³¹czenie testu g³êbi i obcinanie 
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	GL_CHECK_ERRORS

	//generowanie obiektu skyboksa
	skybox = new CSkybox();

	GL_CHECK_ERRORS
		
	//wczytywanie tekstur skyboksa
	int texture_widths[6];
	int texture_heights[6];
	int channels[6];
	GLubyte* pData[6];

	cout<<"Wczytywanie obrazów nieba: ..."<<endl;
	for(int i=0;i<6;i++) {
		cout<<"\tWczytywanie: "<<texture_names[i]<<" ... ";
		pData[i] = SOIL_load_image(texture_names[i],	&texture_widths[i], &texture_heights[i], &channels[i], SOIL_LOAD_AUTO);
		cout<<"Gotowe."<<endl;
	}

	GL_CHECK_ERRORS

	//generowanie tekstury
    glGenTextures(1, &skyboxTextureID);
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);

	GL_CHECK_ERRORS
	//ustalanie parametrów tekstury
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	GL_CHECK_ERRORS

	//ustalanie formatu obrazów
	GLint format = (channels[0]==4)?GL_RGBA:GL_RGB;
    //za³adowanie 6 obrazów
	for(int i=0;i<6;i++) {
		//alokowanie danych dla tekstury
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, texture_widths[i], texture_heights[i], 0, format, GL_UNSIGNED_BYTE, pData[i]);

		//zwalnianie danych obrazowych 
		SOIL_free_image_data(pData[i]);
	}

	GL_CHECK_ERRORS
	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {
	delete skybox;

	glDeleteTextures(1, &skyboxTextureID);
	cout<<"Shutdown successfull"<<endl;
}

//obs³uga zmiany wymiarów okna
void OnResize(int w, int h) {
	//set the viewport
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//setup the projection matrix
	P = glm::perspective(45.0f, (GLfloat)w/h, 0.1f, 1000.f);
}

//obs³uga sygna³u bezczynnoœci procesora
void OnIdle() {
	glutPostRedisplay();
}

//funkcja zwrotna wyœwietlania
void OnRender() {
	GL_CHECK_ERRORS

	//czyszczenie buforów koloru i g³êbi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//transformacje kamery
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(glm::mat4(1),  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV	= glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 S     = glm::scale(glm::mat4(1),glm::vec3(1000.0));
    glm::mat4 MVP	= P*MV*S;

	//renderowanie obiektu skyboksa
	skybox->Render( glm::value_ptr(MVP));

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
	glutCreateWindow("Skybox - OpenGL 3.3");

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
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutIdleFunc(OnIdle);

	//wywo³anie pêtli g³ównej
	glutMainLoop();

	return 0;
}
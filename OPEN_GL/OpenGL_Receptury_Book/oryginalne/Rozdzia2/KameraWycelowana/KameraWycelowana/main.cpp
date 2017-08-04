#define _USE_MATH_DEFINES
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

//rozmiar okna
const int WIDTH  = 1280;
const int HEIGHT = 960;

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=0, rY=0, dist = 10;

#include "..\..\src\TargetCamera.h"

//kody klawiszy
const int VK_W = 0x57;
const int VK_S = 0x53;
const int VK_A = 0x41;
const int VK_D = 0x44;
const int VK_Q = 0x51;
const int VK_Z = 0x5a;

//przyrost czasu
float dt = 0;

//zmienne zwi¹zane z up³ywem czasu
float last_time=0, current_time =0;

const float MOVE_SPEED = 5; //m/s

//instancja kamery swobodnej
CTargetCamera cam;

//zmienne zwi¹zane z wyg³adzaniem ruchów myszy
const float MOUSE_FILTER_WEIGHT=0.75f;
const int MOUSE_HISTORY_BUFFER_SIZE = 10;

//bufor historii po³o¿eñ myszy
glm::vec2 mouseHistory[MOUSE_HISTORY_BUFFER_SIZE];

float mouseX=0, mouseY=0; //filtered mouse values

//wy³¹cznik wyg³adzania ruchów myszy
bool useFiltering = true;

//informacje wyjœciowe
#include <sstream>
std::stringstream msg;

//ID tekstury pod³o¿a
GLuint checkerTextureID;


//obiekt p³askiej szachownicy
#include "..\..\src\TexturedPlane.h"
CTexturedPlane* checker_plane;

//funkcja wyg³adzaj¹ca ruchy myszy
void filterMouseMoves(float dx, float dy) {
    for (int i = MOUSE_HISTORY_BUFFER_SIZE - 1; i > 0; --i) {
        mouseHistory[i] = mouseHistory[i - 1];
    }

    // zapisywanie bie¿¹cego po³o¿enia myszy do bufora historii
    mouseHistory[0] = glm::vec2(dx, dy);

    float averageX = 0.0f;
    float averageY = 0.0f;
    float averageTotal = 0.0f;
    float currentWeight = 1.0f;

    // wyg³adzanie
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
	else if(button == GLUT_RIGHT_BUTTON)
		state = 2;
	else
		state = 1;
}

//obs³uga ruchów myszy
void OnMouseMove(int x, int y)
{
	if (state == 0) {
		dist = (y - oldY)/5.0f;
		cam.Zoom(dist);
	} else if(state ==2) {
		float dy = float(y-oldY)/100.0f;
		float dx = float(oldX-x)/100.0f;
		if(useFiltering)
			filterMouseMoves(dx, dy);
		else {
			mouseX = dx;
			mouseY = dy;
		}

		cam.Pan(mouseX, mouseY);
	} else {
		rY += (y - oldY)/5.0f;
		rX += (oldX-x)/5.0f;
		if(useFiltering)
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

//inicjalizacja OpenGL
void OnInit() {
	GL_CHECK_ERRORS
	//generowanie tekstury szachownicy
	GLubyte data[128][128]={0};
	for(int j=0;j<128;j++) {
		for(int i=0;i<128;i++) {
			data[i][j]=(i<=64 && j<=64 || i>64 && j>64 )?255:0;
		}
	}

	//generowanie obiektu tekstury
	glGenTextures(1, &checkerTextureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, checkerTextureID);
	//ustalanie parametrów tekstury
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
	glTexImage2D(GL_TEXTURE_2D,0,GL_RED, 128, 128, 0, GL_RED, GL_UNSIGNED_BYTE, data);

	//generowanie mipmapy
	glGenerateMipmap(GL_TEXTURE_2D);

	GL_CHECK_ERRORS
		
	//tworzenie obiektu teksturowanej p³aszczyzny
	checker_plane = new CTexturedPlane();

	GL_CHECK_ERRORS
		
	//ustalanie po³o¿eñ celu i kamery
	cam.SetPosition(glm::vec3(5,5,5));
	cam.SetTarget(glm::vec3(0,0,0));

	//obracanie kamery we w³aœciwym kierunku
	glm::vec3 look =  glm::normalize(cam.GetTarget()-cam.GetPosition());

	float yaw = glm::degrees(float(atan2(look.z, look.x)+M_PI));
	float pitch = glm::degrees(asin(look.y));
	rX = yaw;
	rY = pitch;

	cam.Rotate(rX,rY,0);
	cout<<"Initialization successfull"<<endl;
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() { 
	delete checker_plane;
	glDeleteTextures(1, &checkerTextureID);
	cout<<"Zamkniecie powiodlo sie"<<endl;
}

//obs³uga zmiany wymiarów okna
void OnResize(int w, int h) {
	//set the viewport
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//setup the camera projection matrix
	cam.SetupProjection(45, (GLfloat)w/h); 
}

//reakcja na sygna³ bezczynnoœci procesora
void OnIdle() {
	bool bPressed = false;
	float dx=0, dy=0;
	//bs³uga klawiszy W,S,A i D s³u¿¹cych do poruszania kamer¹
	if( GetAsyncKeyState(VK_W) & 0x8000) {
		dy += (MOVE_SPEED*dt);
		bPressed = true;
	}

	if( GetAsyncKeyState(VK_S) & 0x8000) {
		dy -= (MOVE_SPEED*dt);
		bPressed = true;
	} 

	if( GetAsyncKeyState(VK_A) & 0x8000) {
		dx -= (MOVE_SPEED*dt);
		bPressed = true;
	}

	if( GetAsyncKeyState(VK_D) & 0x8000) {
		dx += (MOVE_SPEED*dt);
		bPressed = true;
	}

	if(bPressed)
		cam.Move(dx, dy);

	//call the display function
	glutPostRedisplay();
}

//funkcja zwrotna wyœwietlania
void OnRender() {
	//obliczenia zwi¹zane z czasem
	last_time = current_time;
	current_time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
	dt = current_time-last_time;

	//czyszczenie buforów koloru i g³êbi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//transformacje kamery
	glm::mat4 MV	= cam.GetViewMatrix();
	glm::mat4 P     = cam.GetProjectionMatrix();
    glm::mat4 MVP	= P*MV;

	//renderowanie p³askiej szachownicy
	checker_plane->Render(glm::value_ptr(MVP));

	//zamiana buforów w celu wyœwietlenia obrazu
	glutSwapBuffers(); 
}

//obs³uga wciœniêcia klawisza spacji w³¹czaj¹cego tryb wyg³adzania ruchów myszy
void OnKey(unsigned char key, int x, int y) {
	switch(key) {
		case ' ':
			useFiltering = !useFiltering;
		break;
	}
	//wywo³anie funkcji wyœwietlaj¹cej
	glutPostRedisplay();
}


int main(int argc, char** argv) {
	//inicjalizacja freeglut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Kamera wycelowana - OpenGL 3.3");

	//inicjalizacja glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)	{
		cerr<<"Blad: "<<glewGetErrorString(err)<<endl;
	} else {
		if (GLEW_VERSION_3_3)
		{
			cout<<"Sterownik obsluguje OpenGL 3.3\nDetails:"<<endl;
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
	glutKeyboardFunc(OnKey);
	glutIdleFunc(OnIdle);

	//wywo³anie pêtli g³ównej
	glutMainLoop();

	return 0;
}
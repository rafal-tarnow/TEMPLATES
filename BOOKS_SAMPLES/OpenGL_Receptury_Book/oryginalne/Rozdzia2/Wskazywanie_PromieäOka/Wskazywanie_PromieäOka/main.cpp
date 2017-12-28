#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <algorithm>

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

//do ustalania dok�adno�ci oblicze�
const float EPSILON = 0.001f;
const float EPSILON2 = EPSILON*EPSILON;

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=0, rY=0, fov = 45;

#include "..\..\src\FreeCamera.h"
//kody klawiszy
const int VK_W = 0x57;
const int VK_S = 0x53;
const int VK_A = 0x41;
const int VK_D = 0x44;
const int VK_Q = 0x51;
const int VK_Z = 0x5a;

//przyrost czas
float dt = 0;

//zmienne zwi�zane z up�ywem czasu
float current_time=0, last_time=0;

//instancja kamery swobodnej
CFreeCamera cam;

//siatka
#include "..\..\src\Grid.h"
CGrid* grid;

//kostka
#include "..\..\src\UnitCube.h"
CUnitCube* cube;

//macierze modelu i widoku oraz rzutowania
glm::mat4 MV,P;

//indeks wskazanej kostki
int selected_box=-1;

//po�o�enie kostki
glm::vec3 box_positions[3]={glm::vec3(-1,0.5,0),
							glm::vec3(0,0.5,1),
							glm::vec3(1,0.5,0)
							};


//struktura opisuj�ca kostk� 
struct Box { glm::vec3 min, max;}boxes[3];
#include <limits>

//struktura opisuj�ca promie�
struct Ray {

public:
	glm::vec3 origin, direction;
	float t;

	Ray() {
		t=std::numeric_limits<float>::max();
		origin=glm::vec3(0);
		direction=glm::vec3(0);
	}
}eyeRay;

//wyznaczanie przeci�� promienia z kostk�
glm::vec2 intersectBox(const Ray& ray, const Box& cube) {
	glm::vec3 inv_dir = 1.0f/ray.direction;
	glm::vec3   tMin = (cube.min - ray.origin) * inv_dir;
	glm::vec3   tMax = (cube.max - ray.origin) * inv_dir;
	glm::vec3     t1 = glm::min(tMin, tMax);
	glm::vec3     t2 = glm::max(tMin, tMax);
	float tNear = max(max(t1.x, t1.y), t1.z);
	float  tFar = min(min(t2.x, t2.y), t2.z);
	return glm::vec2(tNear, tFar);
}

//wyj�cie informacyjne
#include <sstream>
std::stringstream msg;

//zmienne zwi�zane z wyg�adzaniem ruch�w myszy
const float MOUSE_FILTER_WEIGHT=0.75f;
const int MOUSE_HISTORY_BUFFER_SIZE = 10;

//bufor historii po�o�e� myszy
glm::vec2 mouseHistory[MOUSE_HISTORY_BUFFER_SIZE];

float mouseX=0, mouseY=0; //wyg�adzone wsp�rz�dne myszy

//wy��cznik wyg�adzania ruch�w myszy
bool useFiltering = true;

//funkcja wyg�adzaj�ca ruchy myszy
void filterMouseMoves(float dx, float dy) {
    for (int i = MOUSE_HISTORY_BUFFER_SIZE - 1; i > 0; --i) {
        mouseHistory[i] = mouseHistory[i - 1];
    }

    // zapisywanie bie��cego po�o�enia myszy do bufora historii
    mouseHistory[0] = glm::vec2(dx, dy);

    float averageX = 0.0f;
    float averageY = 0.0f;
    float averageTotal = 0.0f;
    float currentWeight = 1.0f;

    // wyg�adzanie
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

//obs�uga klikni�cia przyciskiem myszy
void OnMouseDown(int button, int s, int x, int y)
{
	if (s == GLUT_DOWN)
	{
		oldX = x;
		oldY = y;

		//rzutowanie wsteczne punktu klikni�cia z dwiema warto�ciami g��bi: 0 dla bli�szej p�aszczyzny odcinania
                // i 1 dla dalszej. Daje to dwa punkty w przestrzeni �wiata. Po odj�ciu jednego od drugiego otrzymujemy 
		//wektor wyznaczaj�cy kierunek promienia.
		glm::vec3 start = glm::unProject(glm::vec3(x,HEIGHT-y,0), MV, P, glm::vec4(0,0,WIDTH,HEIGHT));
		glm::vec3   end = glm::unProject(glm::vec3(x,HEIGHT-y,1), MV, P, glm::vec4(0,0,WIDTH,HEIGHT));

		eyeRay.origin=cam.GetPosition();
		eyeRay.direction =  glm::normalize(end-start);
		float tMin = numeric_limits<float>::max();
		selected_box = -1;

		//nast�pnie wyznaczamy punkty przeci�cia promienia z wszystkimi proostopad�o�cianami otaczaj�cymi 
		//poszczeg�lne obiekty. Zapami�tywany jest indeks obiektu, dla kt�rego punkt przeci�cia le�y najbli�ej kamery
		for(int i=0;i<3;i++) {
			glm::vec2 tMinMax = intersectBox(eyeRay, boxes[i]);
			if(tMinMax.x<tMinMax.y && tMinMax.x<tMin) {
				selected_box=i;
				tMin = tMinMax.x;
			}
		}
		if(selected_box==-1)
			cout<<"Nie wskazano �adnej kostki"<<endl;
		else
			cout<<"Wskazano kostk� nr: "<<selected_box<<endl;
	}

	if(button == GLUT_MIDDLE_BUTTON)
		state = 0;
	else
		state = 1;
}


//obs�uga ruch�w myszy
void OnMouseMove(int x, int y)
{
	if(selected_box == -1) {
		if (state == 0) {
			fov += (y - oldY)/5.0f;
			cam.SetupProjection(fov, cam.GetAspectRatio());
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
	}
	glutPostRedisplay();
}

//inicjalizacja OpenGL
void OnInit() {

	GL_CHECK_ERRORS

	//tworzenie siatki o wymiarach 20x20 w p�aszczy�nie XZ
	grid = new CGrid(20,20);

	//tworzenie kostki
	cube = new CUnitCube();

	GL_CHECK_ERRORS

	//ustalanie po�o�enia kamery
	glm::vec3 p = glm::vec3(10,10,10);
	cam.SetPosition(p);

	//pobieranie kierunku patrzenia kamery w celu ustalenia warto�ci parametr�w yaw i pitch	
	glm::vec3 look=  glm::normalize(p);

	float yaw = glm::degrees(float(atan2(look.z, look.x)+M_PI));
	float pitch = glm::degrees(asin(look.y));
	rX = yaw;
	rY = pitch;

	//zapisywanie po�o�enia myszy do bufora historii, je�li wyg�adzanie jest w��czone
	if(useFiltering) {
		for (int i = 0; i < MOUSE_HISTORY_BUFFER_SIZE ; ++i) {
			mouseHistory[i] = glm::vec2(rX, rY);
		}
	}
	cam.Rotate(rX,rY,0);

	//w��czenie testu g��bi
	glEnable(GL_DEPTH_TEST);

	//wyznaczanie prostopad�o�cian�w otaczaj�cych obiekty w scenie
	for(int i=0;i<3;i++) {
		boxes[i].min=box_positions[i]-0.5f;
		boxes[i].max=box_positions[i]+0.5f;
	}

	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich alokowanych zasob�w
void OnShutdown() {

	delete grid;
	delete cube;
	cout<<"Shutdown successfull"<<endl;
}

//obs�uga zmiany wymiar�w okna
void OnResize(int w, int h) {
	//wymiary okna widokowego
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//macierz rzutowania dla kamery
	cam.SetupProjection(fov, (GLfloat)w/h);
}

//reakcja na sygna� bezczynno�ci procesora
void OnIdle() {
	//obs�uga klawiszy W,S,A,D,Q i Z su��cych do poruszania kamer�
	if( GetAsyncKeyState(VK_W) & 0x8000) {
		cam.Walk(dt);
	}

	if( GetAsyncKeyState(VK_S) & 0x8000) {
		cam.Walk(-dt);
	}

	if( GetAsyncKeyState(VK_A) & 0x8000) {
		cam.Strafe(-dt);
	}

	if( GetAsyncKeyState(VK_D) & 0x8000) {
		cam.Strafe(dt);
	}

	if( GetAsyncKeyState(VK_Q) & 0x8000) {
		cam.Lift(dt);
	}

	if( GetAsyncKeyState(VK_Z) & 0x8000) {
		cam.Lift(-dt);
	}

	glm::vec3 t = cam.GetTranslation(); 
	if(glm::dot(t,t)>EPSILON2) {
		cam.SetTranslation(t*0.95f);
	}
	
	//wywo�anie funkcji wy�wietlaj�cej
	glutPostRedisplay();
}

//funkcja zwrotna wy�wietlania
void OnRender() {
	
	//obliczenia zwi�zane z czasem
	last_time = current_time;
	current_time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
	dt = current_time-last_time;

	//czyszczenie bufor�w koloru i g��bi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//przygotowanie komunikatu
	msg.str(std::string());
	if(selected_box==-1)
		msg<<"Nie wskazano �adnej kostki";
	else
		msg<<"Wskazano kostk� nr: "<<selected_box;

	//przygotowanie paska tytu�owego
	glutSetWindowTitle(msg.str().c_str());

	//tworzenie macierzy MVP z macierzy MV i P
	MV	= cam.GetViewMatrix();
	P   = cam.GetProjectionMatrix();
    glm::mat4 MVP	= P*MV;

	//renderowanie siatki
	grid->Render(glm::value_ptr(MVP));

	//ustalenie po�o�enia pierwszej kostki 
	//i przypisanie jej koloru cyjanowego, je�li zosta�a wskazana, albo czerwonego
	glm::mat4 T = glm::translate(glm::mat4(1), box_positions[0]);
	cube->color = (selected_box==0)?glm::vec3(0,1,1):glm::vec3(1,0,0);
	cube->Render(glm::value_ptr(MVP*T));

	///ustalenie po�o�enia drugiej kostki 
	//i przypisanie jej koloru cyjanowego, je�li zosta�a wskazana, albo zielonego
	T = glm::translate(glm::mat4(1), box_positions[1]);
	cube->color = (selected_box==1)?glm::vec3(0,1,1):glm::vec3(0,1,0);
	cube->Render(glm::value_ptr(MVP*T));

	//ustalenie po�o�enia trzeciej kostki 
	//i przypisanie jej koloru cyjanowego, je�li zosta�a wskazana, albo niebieskiego
	T = glm::translate(glm::mat4(1), box_positions[2]);
	cube->color = (selected_box==2)?glm::vec3(0,1,1):glm::vec3(0,0,1);
	cube->Render(glm::value_ptr(MVP*T));

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
	glutCreateWindow("Wskazywanie na podsyawie zapyta� o przeci�cia - OpenGL 3.3");

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
	err = glGetError(); //w celu ignorowania b��du 1282 INVALID ENUM
	GL_CHECK_ERRORS

	//wypisywanie informacji na ekranie
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
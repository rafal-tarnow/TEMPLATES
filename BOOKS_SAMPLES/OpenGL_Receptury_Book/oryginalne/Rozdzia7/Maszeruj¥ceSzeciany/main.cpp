#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "..\src\GLSLShader.h"
#include <fstream>

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);


#pragma comment(lib, "glew32.lib")

using namespace std;

//wymiary okna
const int WIDTH  = 1280;
const int HEIGHT = 960;

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=4, rY=50, dist = -2;

//obiekt siatki
#include "..\src\Grid.h"
CGrid* grid;

//macierze rzutowania oraz modelu i widoku
glm::mat4 MV,P;
 
//znacznik do ustalania trybu renderowania
bool bWireframe = false;

//identyfikatory tablicy wierzchołków i obiektów bufora dla maszerującego sześcianu
GLuint volumeMarcherVBO;
GLuint volumeMarcherVAO;

//shader
GLSLShader shader;

//kolor tła
glm::vec4 bg=glm::vec4(0.5,0.5,1,1);

//nazwa pliku z danymi wolumetrycznymi
const std::string volume_file = "../media/Engine256.raw";
 
//instancja klasy TetrahedraMarcher
#include "TetrahedraMarcher.h"
TetrahedraMarcher* marcher;

//obsługa kliknięcia myszą
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
		dist += (y - oldY)/50.0f;
	} else {
		rX += (y - oldY)/5.0f;
		rY += (x - oldX)/5.0f;
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}
 
//inicjalizacja OpenGL
void OnInit() {

	GL_CHECK_ERRORS

	//tworzenie regularnej siatki o wymiarach 20x20 w płaszczyźnie XZ
	grid = new CGrid(20,20);

	GL_CHECK_ERRORS

	//tworzenie nowej instancji klasy TetrahedraMarcher
	marcher = new TetrahedraMarcher();
	//wymiary obszaru wolumetrycznego
	marcher->SetVolumeDimensions(256,256,256);
	//wczytanie danych wolumetrycznych
	marcher->LoadVolume(volume_file);
	//ustalenie wartości określającej izopowierzchnię
	marcher->SetIsosurfaceValue(48);
	//ustalenie liczby próbkowanych wokseli 
	marcher->SetNumSamplingVoxels(128,128,128);
	//uruchomienie procedury maszerującego sześcianu
	marcher->MarchVolume();

	//ustawianie obiektu tablicy wierzchołków i obiektów bufora dla maszerującego sześcianu
	glGenVertexArrays(1, &volumeMarcherVAO);
	glGenBuffers(1, &volumeMarcherVBO);
	glBindVertexArray(volumeMarcherVAO);
	glBindBuffer (GL_ARRAY_BUFFER, volumeMarcherVBO);

	//przekazanie wierzchołków wygenerowanych przez maszerujący sześcian do 
	//pamięci obiektu bufora
	glBufferData (GL_ARRAY_BUFFER, marcher->GetTotalVertices()*sizeof(Vertex), marcher->GetVertexPointer(), GL_STATIC_DRAW);

	//włączenie tablicy atrybutów wierzchołka dla położenia
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);

	//włączenie tablicy atrybutów wierzchołka dla normalnych
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),(const GLvoid*)offsetof(Vertex, normal));

	GL_CHECK_ERRORS

	//wczytanie shaderów 
	shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/marcher.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/marcher.frag");

	//kompilacja i konsolidacja programu shaderowego
	shader.CreateAndLinkProgram();
	shader.Use();
		//dodawanie atrybutów i uniformów
		shader.AddAttribute("vVertex");
		shader.AddAttribute("vNormal");
		shader.AddUniform("MVP");
	shader.UnUse();

	GL_CHECK_ERRORS

	//ustawienie koloru tła
	glClearColor(bg.r, bg.g, bg.b, bg.a);

	//włączenie testowania głębi i zasłaniania
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {
	shader.DeleteShaderProgram();
	glDeleteVertexArrays(1, &volumeMarcherVAO);
	glDeleteBuffers(1, &volumeMarcherVBO);

	delete grid;
	delete marcher;
	cout<<"Zamkniecie powiodlo sie"<<endl;
}

//obsługa zmiany wymiarów okna
void OnResize(int w, int h) {
	//ustawienie wymiarów okna widokowego
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//wyznaczanie macierzy rzutowania
	P = glm::perspective(60.0f,(float)w/h, 0.1f,1000.0f);
}

//funkcja zwrotna wyświetlania
void OnRender() {
	GL_CHECK_ERRORS
	//transformacje kamery
	glm::mat4 Tr	= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(Tr,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//czyszczenie buforów koloru i głębi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//wyznaczanie połączonej macierzy modelu, widoku i rzutowania
    glm::mat4 MVP	= P*MV;

	//renderowanie siatki
	grid->Render(glm::value_ptr(MVP));

	//przenoszenie rezultatów procedury maszerującego sześcianu do początku układu współrzędnych
	glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(-0.5,-0.5,-0.5));

	//jeśli znacznik do ustalania trybu renderowania jest ustawiony na tryb krawędziowy 
	//ustawi tryb rysowania wielokątów na GL_LINE
	if(bWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	//wiązanie obiektu tablicy wierzchołków 
	glBindVertexArray(volumeMarcherVAO);
		//wiązanie shadera
		shader.Use();
			//ustawianie uniformów shadera
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP*T));
				//rysowanie trójkątów
				glDrawArrays(GL_TRIANGLES, 0, marcher->GetTotalVertices());
		//wyłączenie shadera
		shader.UnUse();
	
	//przywrócenie domyślnego trybu rysowania wielkątów
	if(bWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//zamiana buforów w celu wyświetlenia wyrenderowanego obrazu
	glutSwapBuffers();
}

//obsługa klawiatury w celu zmiany trybu renderowania
void OnKey(unsigned char key, int x, int y) {
	switch(key) {
		case 'w': bWireframe = !bWireframe; break; 
	}
	//wywołanie funkcji wyświetlającej
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	//inicjalizacja freeglut 
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Wydzielanie izopowierzchni metodą maszerujących sześcianów - OpenGL 3.3");

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
	err = glGetError(); //w celu pominięcia błędu 1282 INVALID ENUM
	GL_CHECK_ERRORS

	//wyprowadzanie informacji sprzętowych
	cout<<"\tWersja GLEW "<<glewGetString(GLEW_VERSION)<<endl;
	cout<<"\tProducent: "<<glGetString (GL_VENDOR)<<endl;
	cout<<"\tRenderer: "<<glGetString (GL_RENDERER)<<endl;
	cout<<"\tWersja OpenGL: "<<glGetString (GL_VERSION)<<endl;
	cout<<"\tGLSL: "<<glGetString (GL_SHADING_LANGUAGE_VERSION)<<endl;

	GL_CHECK_ERRORS

	//inicjalizacja OpenGL
	OnInit();

	//rejestrowanie funkcji zwrornych
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutKeyboardFunc(OnKey);

	//wywołanie pętli głównej
	glutMainLoop();

	return 0;
}
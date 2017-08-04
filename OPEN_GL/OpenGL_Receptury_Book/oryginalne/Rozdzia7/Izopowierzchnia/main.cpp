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

float last_time=0, current_time=0;

//identyfikatory tablicy wierzchołków i obiektów bufora dla kostki
GLuint cubeVBOID;
GLuint cubeVAOID;
GLuint cubeIndicesID;

//pseudo iso-surface ray casting shader
GLSLShader shader;

//kolor tła
glm::vec4 bg=glm::vec4(0.5,0.5,1,1);

//nazwa pliku z danymi wolumetrycznymi 
const std::string volume_file = "../media/Engine256.raw";

//wymiary obszaru wolumetrycznego
const int XDIM = 256;
const int YDIM = 256;
const int ZDIM = 256;

//volume texture ID
GLuint textureID;

//ta funkcja wczytuje dane wolumetryczne z podanego pliku  
//i tworzy z nich nową teksturę OpenGL
bool LoadVolume() {
	std::ifstream infile(volume_file.c_str(), std::ios_base::binary);

	if(infile.good()) {
		//odczytywanie pliku z danymi wolumetrycznymi
		GLubyte* pData = new GLubyte[XDIM*YDIM*ZDIM];
		infile.read(reinterpret_cast<char*>(pData), XDIM*YDIM*ZDIM*sizeof(GLubyte));
		infile.close();

		//generowanie tekstury OpenGL
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_3D, textureID);

		//ustawianie parametrów tekstury
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		//ustalanie poziomów mipmap (bazowego i maksymalnego)
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 4);

		//alokowanie pamięci dla tekstury o formatach (wewnętrznym i pikselowym) GL_RED			
		glTexImage3D(GL_TEXTURE_3D,0,GL_RED,XDIM,YDIM,ZDIM,0,GL_RED,GL_UNSIGNED_BYTE,pData);
		GL_CHECK_ERRORS

		//generowanie mipmap
		glGenerateMipmap(GL_TEXTURE_3D);

		//usuwanie danych wolumetrycznych ze stosu
		delete [] pData;
		return true;
	} else {
		return false;
	}
}

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

	//wczytanie shadera rzucającego promienie
	shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/raycaster.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/raycaster.frag");

	//kompilacja i konsolidacja programu shaderowego
	shader.CreateAndLinkProgram();
	shader.Use();
		//dodawanie atrybutów i uniformów
		shader.AddAttribute("vVertex");
		shader.AddUniform("MVP");
		shader.AddUniform("volume");
		shader.AddUniform("camPos");
		shader.AddUniform("step_size");

		//ustalanie wartości stałych uniformów
		glUniform3f(shader("step_size"), 1.0f/XDIM, 1.0f/YDIM, 1.0f/ZDIM);
		glUniform1i(shader("volume"),0);
	shader.UnUse();

	GL_CHECK_ERRORS

	//wczytanie danych wolumetrycznych
	if(LoadVolume()) {
		std::cout<<"Wczytanie danych wolumetrycznych powiodlo sie."<<std::endl; 
	} else {
		cout<<"Nie mogę wczytać danych wolumetrycznych"<<endl;
		exit(EXIT_FAILURE);
	}

	//ustawienie koloru tła
	glClearColor(bg.r, bg.g, bg.b, bg.a);
	
	//generowanie tablicy wierzchołków i obiektów bufora dla kostki
	glGenVertexArrays(1, &cubeVAOID);
	glGenBuffers(1, &cubeVBOID);
	glGenBuffers(1, &cubeIndicesID);

	//wierzchołki kostki 
	glm::vec3 vertices[8]={ glm::vec3(-0.5f,-0.5f,-0.5f),
							glm::vec3( 0.5f,-0.5f,-0.5f),
							glm::vec3( 0.5f, 0.5f,-0.5f),
							glm::vec3(-0.5f, 0.5f,-0.5f),
							glm::vec3(-0.5f,-0.5f, 0.5f),
							glm::vec3( 0.5f,-0.5f, 0.5f),
							glm::vec3( 0.5f, 0.5f, 0.5f),
							glm::vec3(-0.5f, 0.5f, 0.5f)};

	//indeksy kostki
	GLushort cubeIndices[36]={0,5,4,
							  5,0,1,
							  3,7,6,
							  3,6,2,
							  7,4,6,
							  6,4,5,
							  2,1,3,
							  3,1,0,
							  3,0,7,
							  7,0,4,
							  6,5,2,
							  2,5,1};
	glBindVertexArray(cubeVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, cubeVBOID);
		//przekazanie wierzchołków kostki do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), &(vertices[0].x), GL_STATIC_DRAW);

		GL_CHECK_ERRORS

		//włączenie tablicy atrybutów wierzchołka dla położenia
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,0,0);

		//przekazanie indeksów do bufora tablicy elementów
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, cubeIndicesID);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), &cubeIndices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

	//włączenie testowania głębi
	glEnable(GL_DEPTH_TEST);

	//mieszanie nakładkowe
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {
	shader.DeleteShaderProgram();

	glDeleteVertexArrays(1, &cubeVAOID);
	glDeleteBuffers(1, &cubeVBOID);
	glDeleteBuffers(1, &cubeIndicesID);

	glDeleteTextures(1, &textureID);
	delete grid;
	cout<<"Zamkniecie powiodlo sie"<<endl;
}

//obsługa zmiany wymiarów okna
void OnResize(int w, int h) {
	//przywrócenie domyślnych wymiarów okna widokowego
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

	//wyznaczanie położenia kamery
	glm::vec3 camPos = glm::vec3(glm::inverse(MV)*glm::vec4(0,0,0,1));

	//czyszczenie buforów koloru i głębi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//wyznaczanie połączonej macierzy modelu, widoku i rzutowania
    glm::mat4 MVP	= P*MV;
	
	//renderowanie siatki
	grid->Render(glm::value_ptr(MVP));

	//włączenie mieszania i wiązanie obiektu tablicy wierzchołków kostki
	glEnable(GL_BLEND);
	glBindVertexArray(cubeVAOID);
		//wiązanie shadera rzucającego promienie
		shader.Use();
			//ustawianie uniformów shadera
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
			glUniform3fv(shader("camPos"), 1, &(camPos.x));
				//renderowanie kostki
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
		//odwiązanie shadera rzucającego promienie
		shader.UnUse();
	//wyłączenie mieszania
	glDisable(GL_BLEND);

	//zamiana buforów w celu wyświetlenia wyrenderowanego obrazu
	glutSwapBuffers();
}

int main(int argc, char** argv) {
	//inicjalizacja freeglut 
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Renderowanie izopowierzchni - OpenGL 3.3");

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

	//rejestracja funkcji zwrotnych
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);

	//wywołanie pętli głównej
	glutMainLoop();

	return 0;
}
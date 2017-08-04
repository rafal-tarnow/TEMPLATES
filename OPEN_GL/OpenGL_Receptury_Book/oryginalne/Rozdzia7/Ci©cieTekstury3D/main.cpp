#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "..\src\GLSLShader.h"
#include <fstream>

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

//precyzja zmiennoprzecinkowa
const float EPSILON = 0.0001f;

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

//obiekty tablicy i bufora wierzchołków wolumetrycznych
GLuint volumeVBO;
GLuint volumeVAO;

//shader tnący teksturę 3D na płaty
GLSLShader shader;

//maksymalna liczba płatów
const int MAX_SLICES = 512;

//wierzchołki cięć
glm::vec3 vTextureSlices[MAX_SLICES*12];

//kolor tła
glm::vec4 bg=glm::vec4(0.5,0.5,1,1);

//plik z danymi wolumetrycznymi
const std::string volume_file = "../media/Engine256.raw";

//wymiary obszaru wolumetrycznego
const int XDIM = 256;
const int YDIM = 256;
const int ZDIM = 256;

//liczba używanych płatów
int num_slices = 256;

//identyfikator tekstury OpenGL-owej
GLuint textureID;

//znacznik informujący o obrocie widoku
//jeśli kamera została obrócona, tekstura jest cięta na nowo
bool bViewRotated = false;

//wierzchołki kostki
glm::vec3 vertexList[8] = {glm::vec3(-0.5,-0.5,-0.5),
						   glm::vec3( 0.5,-0.5,-0.5),
						   glm::vec3(0.5, 0.5,-0.5),
						   glm::vec3(-0.5, 0.5,-0.5),
						   glm::vec3(-0.5,-0.5, 0.5),
						   glm::vec3(0.5,-0.5, 0.5),
						   glm::vec3( 0.5, 0.5, 0.5),
						   glm::vec3(-0.5, 0.5, 0.5)};

//krawędzie sześcianu jednostkowej
int edgeList[8][12] = {
	{ 0,1,5,6,   4,8,11,9,  3,7,2,10 }, // v0 is front
	{ 0,4,3,11,  1,2,6,7,   5,9,8,10 }, // v1 is front
	{ 1,5,0,8,   2,3,7,4,   6,10,9,11}, // v2 is front
	{ 7,11,10,8, 2,6,1,9,   3,0,4,5  }, // v3 is front
	{ 8,5,9,1,   11,10,7,6, 4,3,0,2  }, // v4 is front
	{ 9,6,10,2,  8,11,4,7,  5,0,1,3  }, // v5 is front
	{ 9,8,5,4,   6,1,2,0,   10,7,11,3}, // v6 is front
	{ 10,9,6,5,  7,2,3,1,   11,4,8,0 }  // v7 is front
};
const int edges[12][2]= {{0,1},{1,2},{2,3},{3,0},{0,4},{1,5},{2,6},{3,7},{4,5},{5,6},{6,7},{7,4}};

//bieżący kierunek widoku
glm::vec3 viewDir;


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

	if(s == GLUT_UP)
		bViewRotated = false;
}

//obsługa ruchów myszy
void OnMouseMove(int x, int y)
{
	if (state == 0) {
		dist += (y - oldY)/50.0f;
	} else {
		rX += (y - oldY)/5.0f;
		rY += (x - oldX)/5.0f;
		bViewRotated = true;
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}

//wyznaczanie największej (co do wartości bezwzględnej) współrzędnej wierzchołka v
int FindAbsMax(glm::vec3 v) {
	v = glm::abs(v);
	int max_dim = 0;
	float val = v.x;
	if(v.y>val) {
		val = v.y;
		max_dim = 1;
	}
	if(v.z > val) {
		val = v.z;
		max_dim = 2;
	}
	return max_dim;
}

//główna funkcja tnąca
void SliceVolume() {

	//wyznaczanie największej i najmniejszej odległości każdego wierzchołka jednostkowego sześcianu
	//od kamery w kierunku patrzenia
	float max_dist = glm::dot(viewDir, vertexList[0]);
	float min_dist = max_dist;
	int max_index = 0;
	int count = 0;

	for(int i=1;i<8;i++) {
		//wyznaczanie odległości między bieżącym wierzchołkiem jednostkowego sześcianu a 
		//wektorem widoku przez obliczenie iloczynu skalarnego
		float dist = glm::dot(viewDir, vertexList[i]);

		//jeśli ta odległość jest > max_dist, zapisz jej wartość i indeks
		if(dist > max_dist) {
			max_dist = dist;
			max_index = i;
		}

		//jeśli ta odległość jest < min_dist, zapisz jej wartość 
		if(dist<min_dist)
			min_dist = dist;
	}
	//wyznaczanie największej (co do wartości bezwzględnej) współrzędnej wektora widoku
	int max_dim = FindAbsMax(viewDir);

	//Niewielka modyfikacja
	min_dist -= EPSILON;
	max_dist += EPSILON;

	//zmienne lokalne do przechowywania wektorów początku, kierunku 
	//i wartości lambda
	glm::vec3 vecStart[12];
	glm::vec3 vecDir[12];
	float lambda[12];
	float lambda_inc[12];
	float denom = 0;

	//ustawianie płata tnącego w odległości min_dist
	//odjęcie odległości najmniejszej od największej i podzielenie tej różnicy przez  
	//liczbę płatów w celu wyznaczenia odległości między płatami
	float plane_dist = min_dist;
	float plane_dist_inc = (max_dist-min_dist)/float(num_slices);

	//dla wszystkich krawędzi
	for(int i=0;i<12;i++) {
		//wyznaczanie położenia początkowego przez przeglądnięcie tablicy wierzchołków
		vecStart[i] = vertexList[edges[edgeList[max_index][i]][0]];

		//wyznaczanie kierunku przez przeglądnięcie tablicy wierzchołków
		vecDir[i] = vertexList[edges[edgeList[max_index][i]][1]]-vecStart[i];

		//mnożenie skalarne wektora vecDir przez wektor kierunkowy widoku
		denom = glm::dot(vecDir[i], viewDir);

		//wyznaczanie parametru lambda dla płatu tnącego 
		//i jego przyrostu (lambda_inc)
		if (1.0 + denom != 1.0) {
			lambda_inc[i] =  plane_dist_inc/denom;
			lambda[i]     = (plane_dist - glm::dot(vecStart[i],viewDir))/denom;
		} else {
			lambda[i]     = -1.0;
			lambda_inc[i] =  0.0;
		}
	}

	//zmienne lokalne do przechowywania punktów przecięcia
	//zauważ, że dla danego płata możemy mieć  
	//od 3 do 6 wierzchołków
	glm::vec3 intersection[6];
	float dL[12];

	//pętla po wszystkich płatach
	for(int i=num_slices-1;i>=0;i--) {
		
		//wyznaczanie wartości parametru lambda dla wszystkich krawędzi
		for(int e = 0; e < 12; e++)
		{
			dL[e] = lambda[e] + i*lambda_inc[e];
		}

		//jeśli wartości mieszczą się w przedziale 0-1, krawędź jest przecinana
		//to samo powtarzamy dla wszystkich 12 krawędzi
		if  ((dL[0] >= 0.0) && (dL[0] < 1.0))	{
			intersection[0] = vecStart[0] + dL[0]*vecDir[0];
		}
		else if ((dL[1] >= 0.0) && (dL[1] < 1.0))	{
			intersection[0] = vecStart[1] + dL[1]*vecDir[1];
		}
		else if ((dL[3] >= 0.0) && (dL[3] < 1.0))	{
			intersection[0] = vecStart[3] + dL[3]*vecDir[3];
		}
		else continue;

		if ((dL[2] >= 0.0) && (dL[2] < 1.0)){
			intersection[1] = vecStart[2] + dL[2]*vecDir[2];
		}
		else if ((dL[0] >= 0.0) && (dL[0] < 1.0)){
			intersection[1] = vecStart[0] + dL[0]*vecDir[0];
		}
		else if ((dL[1] >= 0.0) && (dL[1] < 1.0)){
			intersection[1] = vecStart[1] + dL[1]*vecDir[1];
		} else {
			intersection[1] = vecStart[3] + dL[3]*vecDir[3];
		}

		if  ((dL[4] >= 0.0) && (dL[4] < 1.0)){
			intersection[2] = vecStart[4] + dL[4]*vecDir[4];
		}
		else if ((dL[5] >= 0.0) && (dL[5] < 1.0)){
			intersection[2] = vecStart[5] + dL[5]*vecDir[5];
		} else {
			intersection[2] = vecStart[7] + dL[7]*vecDir[7];
		}
		if	((dL[6] >= 0.0) && (dL[6] < 1.0)){
			intersection[3] = vecStart[6] + dL[6]*vecDir[6];
		}
		else if ((dL[4] >= 0.0) && (dL[4] < 1.0)){
			intersection[3] = vecStart[4] + dL[4]*vecDir[4];
		}
		else if ((dL[5] >= 0.0) && (dL[5] < 1.0)){
			intersection[3] = vecStart[5] + dL[5]*vecDir[5];
		} else {
			intersection[3] = vecStart[7] + dL[7]*vecDir[7];
		}
		if	((dL[8] >= 0.0) && (dL[8] < 1.0)){
			intersection[4] = vecStart[8] + dL[8]*vecDir[8];
		}
		else if ((dL[9] >= 0.0) && (dL[9] < 1.0)){
			intersection[4] = vecStart[9] + dL[9]*vecDir[9];
		} else {
			intersection[4] = vecStart[11] + dL[11]*vecDir[11];
		}

		if ((dL[10]>= 0.0) && (dL[10]< 1.0)){
			intersection[5] = vecStart[10] + dL[10]*vecDir[10];
		}
		else if ((dL[8] >= 0.0) && (dL[8] < 1.0)){
			intersection[5] = vecStart[8] + dL[8]*vecDir[8];
		}
		else if ((dL[9] >= 0.0) && (dL[9] < 1.0)){
			intersection[5] = vecStart[9] + dL[9]*vecDir[9];
		} else {
			intersection[5] = vecStart[11] + dL[11]*vecDir[11];
		}

		//po wyznaczeniu wszystkich możliwych punktów przecięcia 
		//ustalamy właściwe indeksy dla wielokątnego płata, układając trójkąty w wachlarz 
		int indices[]={0,1,2, 0,2,3, 0,3,4, 0,4,5};

		//używając indeksów, przekaż punkty przecięcia do wektora vTextureSlices
		for(int i=0;i<12;i++)
			vTextureSlices[count++]=intersection[indices[i]];
	}

	//zaktualizuj obiekt bufora nowymi nwierzchołkami
	glBindBuffer(GL_ARRAY_BUFFER, volumeVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0,  sizeof(vTextureSlices), &(vTextureSlices[0].x));
}

//inicjalizacja OpenGL
void OnInit() {

	GL_CHECK_ERRORS

	//tworzenie regularnej siatki o wymiarach 20x20 w płaszczyźnie XZ
	grid = new CGrid(20,20);

	GL_CHECK_ERRORS
		 
	//wczytanie shaderów cięcia tekstury
	shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/textureSlicer.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/textureSlicer.frag");

	//kompilacja i konsolidacja programu shaderowego
	shader.CreateAndLinkProgram();
	shader.Use();
		//dodawanie atrybutów i uniformów
		shader.AddAttribute("vVertex");
		shader.AddUniform("MVP");
		shader.AddUniform("volume");
		//ustalanie wartości stałych uniformów
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

	//ustalenie bieżących transformacji kamery i wyznaczenie wektora kierunkowego
	glm::mat4 T	= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//wyznaczenie wektora kierunkowego widoku
	viewDir = -glm::vec3(MV[0][2], MV[1][2], MV[2][2]);

	//obiekty tablicy i bufora wierzchołków 
	glGenVertexArrays(1, &volumeVAO);
	glGenBuffers(1, &volumeVBO);

	glBindVertexArray(volumeVAO);
	glBindBuffer (GL_ARRAY_BUFFER, volumeVBO);

	//przekazanie wektora punktów przecięcia do obiektu bufora
	glBufferData (GL_ARRAY_BUFFER, sizeof(vTextureSlices), 0, GL_DYNAMIC_DRAW);

	GL_CHECK_ERRORS
	
	//włączenie tablicy atrybutów wierzchołka dla położenia
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,0,0);

	glBindVertexArray(0);

	//wstępne cięcie danych wolumetrycznych na płaty
	SliceVolume();
	 
	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {
	shader.DeleteShaderProgram();

	glDeleteVertexArrays(1, &volumeVAO);
	glDeleteBuffers(1, &volumeVBO);

	glDeleteTextures(1, &textureID);
	delete grid;
	cout<<"Zamkniecie powiodlo sie"<<endl;
}

//obsługa zmiany wymiarów okna
void OnResize(int w, int h) {
	//ustawienie wymiarów okna widokowego
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//wyznaczanie macierzy rzutowania
	P = glm::perspective(60.0f,(float)w/h, 0.1f,1000.0f);
}

//funkcja wyświetlania
void OnRender() {
	GL_CHECK_ERRORS
	//transformacje kamery
	glm::mat4 Tr	= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(Tr,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//ustalenie kierunku patrzenia
	viewDir = -glm::vec3(MV[0][2], MV[1][2], MV[2][2]);

	//czyszczenie buforów koloru i głębi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//wyznaczanie połączonej macierzy modelu, widoku i rzutowania
    glm::mat4 MVP	= P*MV;

	//renderowanie siatki
	grid->Render(glm::value_ptr(MVP));

	//jeśli kamera została obrócona, tekstura jest cięta na nowo
	if(bViewRotated)
	{
		SliceVolume();
	}

	//włączenie mieszania
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//wiązanie obiektu tablicy wierzchołków wolumetrycznych
	glBindVertexArray(volumeVAO);
		//uruchamianie shadera wolumetrycznego
		shader.Use();
			//ustawianie uniformów shadera
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
				//rysowanie trójkątów
				glDrawArrays(GL_TRIANGLES, 0, sizeof(vTextureSlices)/sizeof(vTextureSlices[0]));
		//wyłączenie shadera
		shader.UnUse();

	//wyłączenie mieszania
	glDisable(GL_BLEND);

	//zamiana buforów w celu wyświetlenia wyrenderowanego obrazu
	glutSwapBuffers();
}

//obsługa klawiatury w celu zmiany liczby płatów
void OnKey(unsigned char key, int x, int y) {
	switch(key) {
		case '-':
			num_slices--;
			break;

		case '+':
			num_slices++;
			break;
	}
	//sprawdzian zakresu wartości zmiennej num_slices
	num_slices = min(MAX_SLICES, max(num_slices,3));

	//cięcie danych wolumetrycznych na płaty
	SliceVolume();

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
	glutCreateWindow("Rendering wolumetryczny z cięciem tekstury 3D - OpenGL 3.3");
	
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
	glutKeyboardFunc(OnKey);

	//wywołanie pętli głównej
	glutMainLoop();

	return 0;
}
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

//shader tnący teksturę 3D na płaty, shader cienia, shader płaszczyzny i shader czworokąta
GLSLShader shader, shaderShadow, flatShader, quadShader;

//maksymalna liczba płatów
const int MAX_SLICES = 512;

//wierzchołki cięć
glm::vec3 vTextureSlices[MAX_SLICES*12];
 
//plik z danymi wolumetrycznymi
const std::string volume_file = "../media/Engine256.raw";

//wymiary obszaru wolumetrycznego
const int XDIM = 256;
const int YDIM = 256;
const int ZDIM = 256;

//liczba używanych płatów
int num_slices =  256;

//identyfikator tekstury OpenGL-owej
GLuint textureID;

//znacznik informujący o obrocie widoku
//jeśli kamera została obrócona, tekstura jest cięta na nowo
bool bViewRotated = false;

//wierzchołki sześcianu jednostkowego
glm::vec3 vertexList[8] = {glm::vec3(-0.5,-0.5,-0.5),
						   glm::vec3( 0.5,-0.5,-0.5),
						   glm::vec3(0.5, 0.5,-0.5),
						   glm::vec3(-0.5, 0.5,-0.5),
						   glm::vec3(-0.5,-0.5, 0.5),
						   glm::vec3(0.5,-0.5, 0.5),
						   glm::vec3( 0.5, 0.5, 0.5),
						   glm::vec3(-0.5, 0.5, 0.5)};

//krawędzie sześcianu jednostkowego
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

//identyfikatory tablicy wierzchołków i obiektu bufora dla światła
GLuint lightVAOID;
GLuint lightVerticesVBO;
glm::vec3 lightPosOS=glm::vec3(0, 2,0); //położenie światła w przestrzenie obiektu

//współrzędne sferyczne światła
float theta = 0.66f;
float phi = -1.0f;
float radius = 2;

//bieżące wektory kierunkowe widoku i światła oraz wektor pośredni
glm::vec3 viewVec, 
			lightVec, 
			halfVec;

//pozaekranowe bufory dla światła i oka
GLuint lightBufferID, eyeBufferID;

GLuint colorTexID; //identyfikator przyłącza koloru w FBO

//identyfikator obiektu FBO do mieszania światła
GLuint lightFBOID;

//identyfikatory tablicy wierzchołków i obiektów bufora dla czworokąta
GLuint quadVAOID, quadVBOID;


//krotność pomniejszenia obrazu
const int DOWN_SAMPLE = 2;

//szerokość i wysokość pomniejszonego obrazu
const int IMAGE_WIDTH = WIDTH/DOWN_SAMPLE;
const int IMAGE_HEIGHT = HEIGHT/DOWN_SAMPLE;

glm::mat4 MV_L; //macierz modelu i widoku dla swiatła
glm::mat4 P_L;	//macierz rzutowania dla światła
glm::mat4 B;    //macierz przesunięcia dla światła
glm::mat4 BP;   //połączona macierz przesunięcia i rzutowania dla światła
glm::mat4 S;    //połączona macierz BPMV dla światła

//znacznik informujący o odwróceniu kierunku patrzenia
bool bIsViewInverted = false;
//kolor światła
glm::vec4 lightColor=glm::vec4(1.0,1.0,1.0,1.0);
//kolor tłumienia światła 
glm::vec3 lightAttenuation=glm::vec3(0.1,0.2,0.3);

//wartość alfa dla cienia
float fShadowAlpha = 1;
 
//identyfikatory przyłączy koloru w FBO
GLenum attachIDs[2]={GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

//funkcja do tworzenia OpenGL-owej tekstury o określonej szerokości (w) i wysokości (h), a także z zadanymi 
//formatami wewnętrznym (internalFormat) i pikselowym (format)
//Funkcja ta zwraca identyfikator utworzonej tekstury
GLuint CreateTexture(const int w,const int h, GLenum internalFormat, GLenum format) {
	GLuint texid;
    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_FLOAT, 0);
    return texid;
}

//inicjalizacja FBO
void InitFBO() {
	
	//wygeneruj FBO dla renderingu pozaekranowego
	glGenFramebuffers(1, &lightFBOID);

	//ustaw dwie tekstury do zapisu zakumulowanych rezultatów w buforach światła i oka
	glGenTextures (1, &lightBufferID);
	glGenTextures (1, &eyeBufferID);

	GL_CHECK_ERRORS

	//utwórz tekstury dla buforów światła i oka
	glActiveTexture(GL_TEXTURE2);
	lightBufferID = CreateTexture(IMAGE_WIDTH, IMAGE_HEIGHT, GL_RGBA16F, GL_RGBA);
	eyeBufferID = CreateTexture(IMAGE_WIDTH, IMAGE_HEIGHT, GL_RGBA16F, GL_RGBA);

	//wiązanie FBO 
	glBindFramebuffer(GL_FRAMEBUFFER, lightFBOID);
	//ustaw przyłącza koloru
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightBufferID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, eyeBufferID, 0);

	//sprawdzian kompletności FBO
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status == GL_FRAMEBUFFER_COMPLETE )
		printf("Ustawienie FBO dla swiatla powiodlo sie !!! \n");
	else
		printf("Problem z ustawieniem FBO dla swiatla");

	//odwiązanie FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//zwalnianie wszystkich zasobów związanych z FBO
void ShutdownFBO() {
	glDeleteFramebuffers(1, &lightFBOID);
	glDeleteTextures (1, &lightBufferID);
	glDeleteTextures (1, &eyeBufferID);
}

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
		glActiveTexture(GL_TEXTURE0);
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
	else if(button == GLUT_RIGHT_BUTTON)
		state = 2;
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
	} else if(state ==2) {
		theta += (oldX - x)/60.0f;
		phi += (y - oldY)/60.0f;

		//zmiana położenia źródła światła 
		lightPosOS.x = radius * cos(theta)*sin(phi);
		lightPosOS.y = radius * cos(phi);
		lightPosOS.z = radius * sin(theta)*sin(phi);

		//aktualizacja macierzy MV dla światła
		MV_L = glm::lookAt(lightPosOS,glm::vec3(0,0,0), glm::vec3(0,1,0));
		S = BP*MV_L;
	} else {
		rX += (y - oldY)/5.0f;
		rY += (x - oldX)/5.0f;
		bViewRotated = true;
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}

//obsługa rolki do przewijania w celu zmiany promienia obrotów źródła światła
void OnMouseWheel(int button, int dir, int x, int y) {

	if (dir > 0)
    {
        radius += 0.1f;
    }
    else
    {
        radius -= 0.1f;
    }

	radius = max(radius,0.0f);

	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	//aktualizacja macierzy MV dla światła
	MV_L = glm::lookAt(lightPosOS,glm::vec3(0,0,0),glm::vec3(0,1,0));
	S = BP*MV_L;
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
	float max_dist = glm::dot(halfVec, vertexList[0]);
	float min_dist = max_dist;
	int max_index = 0;
	int count = 0;

	for(int i=1;i<8;i++) {
		//wyznaczanie odległości między bieżącym wierzchołkiem jednostkowego sześcianu a 
		//wektorem połówkowym przez obliczenie iloczynu skalarnego
		float dist = glm::dot(halfVec, vertexList[i]);

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
	int max_dim = FindAbsMax(halfVec);

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

		//mnożenie skalarne wektora vecDir przez wektor pośredni
		denom = glm::dot(vecDir[i], halfVec);

		//wyznaczanie parametru lambda dla płatu tnącego 
		//i jego przyrostu (lambda_inc)
		if (1.0 + denom != 1.0) {
			lambda_inc[i] =  plane_dist_inc/denom;
			lambda[i]     = (plane_dist - glm::dot(vecStart[i],halfVec))/denom;
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

	//zaktualizuj obiekt bufora nowymi wierzchołkami
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
		shader.AddUniform("color");
		shader.AddUniform("volume");

		//ustalanie wartości stałych uniformów
		glUniform1i(shader("volume"),0);
		glUniform4f(shader("color"),lightAttenuation.x*fShadowAlpha, lightAttenuation.y * fShadowAlpha, lightAttenuation.z * fShadowAlpha, 1);

	shader.UnUse();

	GL_CHECK_ERRORS

	//wczytanie shaderów cięcia cienia
	shaderShadow.LoadFromFile(GL_VERTEX_SHADER, "shadery/slicerShadow.vert");
	shaderShadow.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/slicerShadow.frag");

	//kompilacja i konsolidacja programu shaderowego
	shaderShadow.CreateAndLinkProgram();
	shaderShadow.Use();
		//dodawanie atrybutów i uniformów
		shaderShadow.AddAttribute("vVertex");
		shaderShadow.AddUniform("MVP");
		shaderShadow.AddUniform("S");
		shaderShadow.AddUniform("color");
		shaderShadow.AddUniform("shadowTex");
		shaderShadow.AddUniform("volume");

		//ustalanie wartości stałych uniformów
		glUniform1i(shaderShadow("volume"),0);
		glUniform1i(shaderShadow("shadowTex"),1);
		glUniform4f(shaderShadow("color"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);

	shaderShadow.UnUse();

	//wczytywanie shaderów płaszczyzny
	flatShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/shader.vert");
	flatShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/shader.frag");

	//kompilacja i konsolidacja programu shaderowego
	flatShader.CreateAndLinkProgram();
	flatShader.Use();
		//dodawanie atrybutów i uniformów
		flatShader.AddAttribute("vVertex");
		flatShader.AddUniform("MVP");
	flatShader.UnUse();

	GL_CHECK_ERRORS

	//wczytywanie shaderów czworokąta
	quadShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/quad_shader.vert");
	quadShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/quad_shader.frag");

	//kompilacja i konsolidacja programu shaderowego
	quadShader.CreateAndLinkProgram();
	quadShader.Use();
		//dodawanie atrybutów i uniformów
		quadShader.AddAttribute("vVertex");
		quadShader.AddUniform("MVP");
		quadShader.AddUniform("textureMap");
		//ustalanie wartości stałych uniformów
		glUniform1i(quadShader("textureMap"),1);
	quadShader.UnUse();

	GL_CHECK_ERRORS

	//wczytanie danych wolumetrycznych
	if(LoadVolume()) {
		std::cout<<"Wczytanie danych wolumetrycznych powiodlo sie."<<std::endl;
		 
	} else {
		cout<<"Nie mogę wczytać danych wolumetrycznych"<<endl;
		exit(EXIT_FAILURE);
	}

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

	//vao i vbo dla położenia gizma światła
	glm::vec3 crossHairVertices[6];
	crossHairVertices[0] = glm::vec3(-0.5f,0,0);
	crossHairVertices[1] = glm::vec3(0.5f,0,0);
	crossHairVertices[2] = glm::vec3(0, -0.5f,0);
	crossHairVertices[3] = glm::vec3(0, 0.5f,0);
	crossHairVertices[4] = glm::vec3(0,0, -0.5f);
	crossHairVertices[5] = glm::vec3(0,0, 0.5f);

	//ustawianie obiektów tablicy i bufora wierzchołków 
	glGenVertexArrays(1, &lightVAOID);
	glGenBuffers(1, &lightVerticesVBO);
	glBindVertexArray(lightVAOID);

	glBindBuffer (GL_ARRAY_BUFFER, lightVerticesVBO);

	//przekazanie wierzchołków gizma światła do obiektu bufora
	glBufferData (GL_ARRAY_BUFFER, sizeof(crossHairVertices), &(crossHairVertices[0].x), GL_STATIC_DRAW);
	GL_CHECK_ERRORS

	//włączenie tablicy atrybutów wierzchołka 
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,0);

	GL_CHECK_ERRORS

	//wierzchołki pełnoekranowego czworokąta
	glm::vec2 quadVerts[6];
	quadVerts[0] = glm::vec2(0,0);
	quadVerts[1] = glm::vec2(1,0);
	quadVerts[2] = glm::vec2(1,1);
	quadVerts[3] = glm::vec2(0,0);
	quadVerts[4] = glm::vec2(1,1);
	quadVerts[5] = glm::vec2(0,1);

	//generowanie tablicy wierzchołków i obiektów bufora dla czworokąta
	glGenVertexArrays(1, &quadVAOID);
	glGenBuffers(1, &quadVBOID);

	glBindVertexArray(quadVAOID);
	glBindBuffer (GL_ARRAY_BUFFER, quadVBOID);
	
	//przekazanie wierzchołków czworokąta do obiektu bufora
	glBufferData (GL_ARRAY_BUFFER, sizeof(quadVerts), &quadVerts[0], GL_STATIC_DRAW);

	GL_CHECK_ERRORS

	//włączenie tablicy atrybutów wierzchołka 
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,0,0);

	//położenie światła w przestrzenie obiektu
	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	//ustalenie bieżących transformacji kamery i wyznaczenie wektora kierunkowego
	glm::mat4 T	= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//wyznaczenie wektora kierunkowego widoku
	viewVec = -glm::vec3(MV[0][2], MV[1][2], MV[2][2]);

	//wstępne cięcie danych wolumetrycznych na płaty
	SliceVolume();

	//inicjalizacja FBO
	InitFBO();

	//macierze MV, P i przesunięcia dla światła
	MV_L = glm::lookAt(lightPosOS,glm::vec3(0,0,0),glm::vec3(0,1,0));
	P_L  = glm::perspective(45.0f,1.0f,1.0f, 200.0f);
	B    = glm::scale(glm::translate(glm::mat4(1),glm::vec3(0.5,0.5,0.5)), glm::vec3(0.5,0.5,0.5));
	BP   = B*P_L;
	S    = BP*MV_L;

	cout<<"Inicjalizacja powiodla sie"<<endl;

	//ustawienie jednostki teksturującej nr 1 jako aktywnej 
	glActiveTexture(GL_TEXTURE1);
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {
	ShutdownFBO();

	shader.DeleteShaderProgram();
	shaderShadow.DeleteShaderProgram();
	flatShader.DeleteShaderProgram();
	quadShader.DeleteShaderProgram();

	glDeleteVertexArrays(1, &volumeVAO);
	glDeleteBuffers(1, &volumeVBO);

	glDeleteVertexArrays(1, &quadVAOID);
	glDeleteBuffers(1, &quadVBOID);

	glDeleteVertexArrays(1, &lightVAOID);
	glDeleteBuffers(1, &lightVerticesVBO);

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
//funkcja renderująca scenę z punktu widzenia oka w buforze oka
void DrawSliceFromEyePointOfView(const int i) {
	GL_CHECK_ERRORS

	//ustaw pierwsze przyłącze koloru jako bufor rysowania
	glDrawBuffer(attachIDs[1]);

	//ustaw wymiary okna widokowego równe wymiarom przyłącza koloru w FBO
	glViewport(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);

	GL_CHECK_ERRORS

	//sprawdź, czy kierunek patrzenia nie został odwrócony
	if(bIsViewInverted) {
		//jeśli tak, zmień równanie mieszania
		glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
	} else {
		//w przeciwnym razie zastosuj typowe równanie
		//z GL_ONE jako źródłem, aby  
		//zachować kolor źródła
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	//narysuj płat 
	glDrawArrays(GL_TRIANGLES, 12*i, 12);

	GL_CHECK_ERRORS

}
//funkcja renderująca scenę z punktu widzenia światła w buforze światła
void DrawSliceFromLightPointOfView(const int i) {

	//ustawianie zerowego przyłącza koloru w buforze światła jako bufora rysowania
	glDrawBuffer(attachIDs[0]);

	//ustaw wymiary okna widokowego równe wymiarom przyłącza koloru 
	glViewport(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);

	//w buforze światła zastosuj klasyczne, nakładkowe mieszanie
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//narysuj płat 
	glDrawArrays(GL_TRIANGLES, 12*i, 12);
}

//funkcja renderująca płaty do bufora światła lub oka
void DrawSlices(glm::mat4 MVP) {

	GL_CHECK_ERRORS

	//wyczyść bufor światła
	glBindFramebuffer(GL_FRAMEBUFFER, lightFBOID);
	glDrawBuffer(attachIDs[0]);
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT );

	//wyczyść bufor oka
	glDrawBuffer(attachIDs[1]);
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT  );

	GL_CHECK_ERRORS

	//wiązanie obiektu tablicy wierzchołków wolumetrycznych
	glBindVertexArray(volumeVAO);

	//dla wszystkich płatów
	for(int i =0;i<num_slices;i++) {
		//uaktywnij shader cienia
		shaderShadow.Use();
		//ustaw uniformy shadera cienia
		glUniformMatrix4fv(shaderShadow("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix4fv(shaderShadow("S"), 1, GL_FALSE, glm::value_ptr(S));

		//zwiąż bufor światła jako teksturę bieżącą
		glBindTexture(GL_TEXTURE_2D, lightBufferID);

		//narysuj płat z punktu widzenia oka w buforze oka
		DrawSliceFromEyePointOfView(i);

		GL_CHECK_ERRORS

		//uaktywnij zwykły shader  
		shader.Use();

		//ustaw uniformy shadera
		glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(P_L*MV_L));

		//narysuj płat z punktu widzenia światła w buforze światła
		DrawSliceFromLightPointOfView(i);

		GL_CHECK_ERRORS
	}
	//odwiąż obiekt tablicy wierzchołków  
	glBindVertexArray(0);

	//odwiąż FBO i przywróć renderowanie w tylnym buforze ekranu
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK_LEFT);

	GL_CHECK_ERRORS

	//przywrócenie domyślnych wymiarów okna widokowego
	glViewport(0,0,WIDTH, HEIGHT);

	//zwiąż bufor oka jako teksturę bieżącą
	glBindTexture(GL_TEXTURE_2D, eyeBufferID);

	//wiązanie obiektu tablicy wierzchołków czworokąta
	glBindVertexArray(quadVAOID);

	//uaktywnij shader czworokąta
	quadShader.Use();
		//rysowanie pełnoekranowego czworokąta
		glDrawArrays(GL_TRIANGLES, 0, 6);
	quadShader.UnUse();

	glBindVertexArray(0);

	GL_CHECK_ERRORS
}

//funkcja zwrotna wyświetlania
void OnRender() {
	GL_CHECK_ERRORS

	//transformacje kamery
	glm::mat4 Tr	= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(Tr,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	 
	//wyznaczanie wektora widoku
	viewVec = -glm::vec3(MV[0][2], MV[1][2], MV[2][2]);

	//wyznaczanie wektora światła
	lightVec = glm::normalize(lightPosOS);

	//sprawdź, czy kierunek patrzenia nie został odwrócony
	bIsViewInverted = glm::dot(viewVec, lightVec)<0;

	//wyznacz wektor pośredni między wektorami widoku i światła
	halfVec = glm::normalize( (bIsViewInverted?-viewVec:viewVec) + lightVec);

	//czyszczenie buforów koloru i głębi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//wyznaczanie połączonej macierzy modelu, widoku i rzutowania
    glm::mat4 MVP	= P*MV;

	//renderowanie siatki
	grid->Render(glm::value_ptr(MVP));

	//cięcie danych wolumetrycznych na płaty
	SliceVolume();

	//wyrenderuj płaty połówkowokątowe
	glEnable(GL_BLEND);
		DrawSlices(MVP);
	glDisable(GL_BLEND);

	//wyrenderuj gizmo światła
	glBindVertexArray(lightVAOID); {
		//ustaw transformacje światła
		glm::mat4 T = glm::translate(glm::mat4(1), lightPosOS);
		//uaktywnij shader płaszczyzny
		flatShader.Use();
			//ustaw uniformy shadera
			glUniformMatrix4fv(flatShader("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV*T));
				//wyrenderuj 6 linii
				glDrawArrays(GL_LINES, 0, 6);
		//odłącz shader płaszczyzny
		flatShader.UnUse();
	}
	
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
	glutCreateWindow("Wolumetryczne oświetlenie z cięciem połówkowokątowym - OpenGL 3.3");

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
	glutMouseWheelFunc(OnMouseWheel);
	glutKeyboardFunc(OnKey);

	//wywołanie pętli głównej
	glutMainLoop();

	return 0;
}
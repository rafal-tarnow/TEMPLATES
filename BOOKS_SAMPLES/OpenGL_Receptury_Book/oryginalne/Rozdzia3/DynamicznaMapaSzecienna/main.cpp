

#include <GL/glew.h>

#define _USE_MATH_DEFINES
#include <cmath>


#include <GL/freeglut.h>
#include <iostream>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "../src/GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")

using namespace std;

//wymiary okna
const int WIDTH  = 1024;
const int HEIGHT = 768;

//rozmiar mapy szeœciennej
const int CUBEMAP_SIZE = 1024;

//shadery do renderowania i do generowania mapy szeœciennej
GLSLShader shader, cubemapShader;

//struktura do przechowywania po³o¿enia i normalnej pojedynczego wierzcho³ka
struct Vertex {
	glm::vec3 pos, normal;
};

//tablica wierzcho³ków sfery i obiekty buforów wiercho³ków
GLuint sphereVAOID;
GLuint sphereVerticesVBO;
GLuint sphereIndicesVBO;

//odleg³oœæ radialnego przemieszczania kostek
float radius = 2;

//macierze rzutowania, modelu i widoku oraz rotacji
glm::mat4  P = glm::mat4(1);
glm::mat4  Pcubemap = glm::mat4(1);
glm::mat4  MV = glm::mat4(1);
glm::mat4 Rot;

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=25, rY=-40, dist = -10;

//ID dynamicznej mapy szeœciennej
GLuint dynamicCubeMapID;

//ID dla FBO i RBO 
GLuint fboID, rboID;


//siatka
#include "../src/Grid.h"
CGrid* grid;


//kostka
#include "../src/UnitCube.h"
CUnitCube* cube;

//po³o¿enie oka
glm::vec3 eyePos;

#include <vector>
using namespace std;

//wierzcho³ki i indeksy geometrii
std::vector<Vertex> vertices;
std::vector<GLushort> indices;

//k¹t automatycznego obrotu
float angle = 0; 

float dx=-0.1f;	//kierunek przesuwania kostek 
 

//tablica sta³ych kolorów
const glm::vec3 colors[8]={glm::vec3(1,0,0),glm::vec3(0,1,0),glm::vec3(0,0,1),glm::vec3(1,1,0),glm::vec3(1,0,1),glm::vec3(0,1,1),glm::vec3(1,1,1),glm::vec3(0.5,0.5,0.5)};

//dodawanie indeksów sfery do wektora indeksów
inline void push_indices(int sectors, int r, int s) {
    int curRow = r * sectors;
    int nextRow = (r+1) * sectors;

    indices.push_back(curRow + s);
    indices.push_back(nextRow + s);
    indices.push_back(nextRow + (s+1));

    indices.push_back(curRow + s);
    indices.push_back(nextRow + (s+1));
    indices.push_back(curRow + (s+1));
}

//generowanie prymitywu sfery o zadanym promieniu oraz liczbach po³udników i równole¿ników
void createSphere( float radius, unsigned int slices, unsigned int stacks)
{
    float const R = 1.0f/(float)(slices-1);
    float const S = 1.0f/(float)(stacks-1);

    for(size_t r = 0; r < slices; ++r) {
        for(size_t s = 0; s < stacks; ++s) {
            float const y = (float)(sin( -M_PI_2 + M_PI * r * R ));
            float const x = (float)(cos(2*M_PI * s * S) * sin( M_PI * r * R ));
            float const z = (float)(sin(2*M_PI * s * S) * sin( M_PI * r * R ));

			Vertex v;
			v.pos = glm::vec3(x,y,z)*radius;
			v.normal = glm::normalize(v.pos);
            vertices.push_back(v);
            push_indices(stacks, r, s);
        }
    }

}

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

	//wczytywanie shaderów mapy szeœciennej
	cubemapShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/cubemap.vert");
	cubemapShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/cubemap.frag");
	//kompilacja i konsolidacja programu shaderowego
	cubemapShader.CreateAndLinkProgram();
	cubemapShader.Use();
		//dodawanie atrybutów i uniformów
		cubemapShader.AddAttribute("vVertex");
		cubemapShader.AddAttribute("vNormal");
		cubemapShader.AddUniform("MVP");
		cubemapShader.AddUniform("eyePosition");
		cubemapShader.AddUniform("cubeMap");
		//ustalanie wartoœci sta³ych uniformów
		glUniform1i(cubemapShader("cubeMap"), 1);
	cubemapShader.UnUse();

	GL_CHECK_ERRORS

	//ustalanie geometrii sfery
	createSphere(1,10,10);

	GL_CHECK_ERRORS

	//ustawianie vao i vbo dla sfery
	glGenVertexArrays(1, &sphereVAOID);
	glGenBuffers(1, &sphereVerticesVBO);
	glGenBuffers(1, &sphereIndicesVBO);
	glBindVertexArray(sphereVAOID);

		glBindBuffer (GL_ARRAY_BUFFER, sphereVerticesVBO);
		//przekazanie wierzcho³ków do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//w³¹czenie tablicy atrybutów wierzho³ka dla po³o¿enia
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);
		GL_CHECK_ERRORS
		//w³¹czenie tablicy atrybutów wierzho³ka dla wektora normalnego
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, normal)));
		GL_CHECK_ERRORS
		//przekazanie indeksów sfery do bufora tablicy elementów 
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, sphereIndicesVBO);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);
		 
	//generowanie dynamicznej tekstury szeœciennej i zwi¹zanie jej z jednostk¹ teksturuj¹c¹ nr 1
	glGenTextures(1, &dynamicCubeMapID);
	glActiveTexture(GL_TEXTURE1);	
	glBindTexture(GL_TEXTURE_CUBE_MAP, dynamicCubeMapID);
	//ustalanie parametrów tekstury
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//dla wszystkich 6 œcian 
	for (int face = 0; face < 6; face++) {
		//ka¿dej œcianie przydzielana jest inna tekstura
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA, CUBEMAP_SIZE, CUBEMAP_SIZE, 0, GL_RGBA, GL_FLOAT, NULL);
	}

	GL_CHECK_ERRORS

	//ustawianie FBO
	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);

	//ustawianie obiektu bufora renderingu (RBO)
	glGenRenderbuffers(1, &rboID);
	glBindRenderbuffer(GL_RENDERBUFFER, rboID);

	//ustawianie wymiarów bufora renderingu na równe wymiarom tekstury szeœciennej
	//i przy³¹czanie tego bufora do prezy³¹cza g³êbi w FBO
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, CUBEMAP_SIZE, CUBEMAP_SIZE);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fboID);

	//do³¹czanie dynamicznej tekstury szeœciennej do przy³¹cza koloru w FBO
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, dynamicCubeMapID, 0);

	//sprawdzanie kompletnoœci bufora ramki
	GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE) {
		cerr<<"Blad przy ustawianiu FBO."<<endl;
		exit(EXIT_FAILURE);
	} else {
		cerr<<"Ustawianie FBO powiodlo sie."<<endl;
	}
	//odwi¹zywanie FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//odwi¹zywanie bufora renderingu
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	GL_CHECK_ERRORS

	//tworzenie obiektu siatki
	grid = new CGrid();

	//tworzenie obiektu kostki
	cube = new CUnitCube(glm::vec3(1,0,0));

	GL_CHECK_ERRORS

	//w³¹czenie testowania g³êbi ukrywania œcianek niewidocznych
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {
	//likwidacja programu shaderowego
	shader.DeleteShaderProgram();
	cubemapShader.DeleteShaderProgram();

	//likwidacja vao i vbo
	glDeleteBuffers(1, &sphereVerticesVBO);
	glDeleteBuffers(1, &sphereIndicesVBO);
	glDeleteVertexArrays(1, &sphereVAOID);

	delete grid;
	delete cube;

	glDeleteTextures(1, &dynamicCubeMapID);


	glDeleteFramebuffers(1, &fboID);
	glDeleteRenderbuffers(1, &rboID);
	cout<<"Zamkniecie powiodlo sie"<<endl;
}

//obs³uga zmiany wymiarów okna
void OnResize(int w, int h) {
	//ustalanie wymiarów okna widokowego
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//wyznaczanie macierzy rzutowania
	P = glm::perspective(45.0f, (GLfloat)w/h, 0.1f, 1000.f);
	//wyznaczanie macierzy rzutowania dla mapy szeœciennej
	Pcubemap = glm::perspective(90.0f,1.0f,0.1f,1000.0f);
}

//obs³uga sygna³u bezczynnoœci procesora
void OnIdle() {
	//generowanie nowej macierzy obrotu wokó³ osi Y
	Rot = glm::rotate(glm::mat4(1), angle++, glm::vec3(0,1,0));

	//wywo³anie wunkcji wyœwietlaj¹cej
	glutPostRedisplay();
}

//funkcja renderuj¹ca scenê
void DrawScene(glm::mat4 MView, glm::mat4 Proj ) {

	//dla ka¿dej kostki
	for(int i=0;i<8;i++){
		//okreœle przekszta³cenia
		float angle = (float)(i/8.0f*2*M_PI);
		glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(radius*cos(angle),0.5,radius*sin(angle)));

		//wyznacz po³¹czon¹ macierz modelu, widoku i rzutowania
		glm::mat4 MVP = Proj*MView*Rot*T;

		//ustal kolor
		cube->color = colors[i];

		//wyrenderuj kostkê
		cube->Render(glm::value_ptr(MVP));
	}

	//wyrenderuj siatkê
	grid->Render(glm::value_ptr(Proj*MView));
}

//funkcja zwrotna wyœwietlania
void OnRender() {

	//zmienianie promienia
	radius+=dx;

	//jeœli promieñ wykracza poza dopuszczalny przedzia³, odwróæ kierunek zmian 
	if(radius<1 || radius>5)
		dx=-dx;

	GL_CHECK_ERRORS

	//czyszczenie buforów koloru i g³êbi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//transformacje kamery
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//po³o¿enie oka
	eyePos.x = -(MV[0][0] * MV[3][0] + MV[0][1] * MV[3][1] + MV[0][2] * MV[3][2]);
    eyePos.y = -(MV[1][0] * MV[3][0] + MV[1][1] * MV[3][1] + MV[1][2] * MV[3][2]);
    eyePos.z = -(MV[2][0] * MV[3][0] + MV[2][1] * MV[3][1] + MV[2][2] * MV[3][2]);

	//p jest translacj¹ sprowadzaj¹c¹ sferê do poziomu pod³o¿a
	glm::vec3 p=glm::vec3(0,1,0);

	//podczas renderowania do tekstury szeœciennej wszystkie kostki s¹ przesuwane w kierunku przeciwnym,
	//aby rzutowanie by³o dobrze widoczne
	T = glm::translate(glm::mat4(1), -p);
	 
	//ustaw wymiary okna widokowego takie same jak dla tekstury szeœciennej
	glViewport(0,0,CUBEMAP_SIZE,CUBEMAP_SIZE);

	//wi¹zanie FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);

		//po³¹cz GL_TEXTURE_CUBE_MAP_POSITIVE_X z przy³¹czem koloru w FBO
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, dynamicCubeMapID, 0);
		//wyczyœæ bufory koloru i g³êbi
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		//ustaw wirtualnego obserwatora w miejscu zajmowanym przez obiekt lustrzany i wyrenderuj scenê,
		//u¿ywaj¹c macierzy rzutowania dla tekstury szeœciennej i odpowiednich ustawieñ widoku
		glm::mat4 MV1 = glm::lookAt(glm::vec3(0),glm::vec3(1,0,0), glm::vec3(0,-1,0));
 		DrawScene( MV1*T, Pcubemap);

		//po³¹cz GL_TEXTURE_CUBE_MAP_NEGATIVE_X z przy³¹czem koloru w FBO
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, dynamicCubeMapID, 0);
		//wyczyœæ bufory koloru i g³êbi
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		//ustaw wirtualnego obserwatora w miejscu zajmowanym przez obiekt lustrzany i wyrenderuj scenê,
		//u¿ywaj¹c macierzy rzutowania dla tekstury szeœciennej i odpowiednich ustawieñ widoku
		glm::mat4 MV2 = glm::lookAt(glm::vec3(0),glm::vec3(-1,0,0), glm::vec3(0,-1,0));
		DrawScene( MV2*T, Pcubemap);
		
		//po³¹cz GL_TEXTURE_CUBE_MAP_POSITIVE_Y z przy³¹czem koloru w FBO		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, dynamicCubeMapID, 0);
		//wyczyœæ bufory koloru i g³êbi 
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		//ustaw wirtualnego obserwatora w miejscu zajmowanym przez obiekt lustrzany i wyrenderuj scenê,
		//u¿ywaj¹c macierzy rzutowania dla tekstury szeœciennej i odpowiednich ustawieñ widoku
		glm::mat4 MV3 = glm::lookAt(glm::vec3(0),glm::vec3(0,1,0), glm::vec3(1,0,0));
		DrawScene( MV3*T, Pcubemap);

		//po³¹cz GL_TEXTURE_CUBE_MAP_NEGATIVE_Y z przy³¹czem koloru w FBO		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, dynamicCubeMapID, 0);
		//wyczyœæ bufory koloru i g³êbi
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		//ustaw wirtualnego obserwatora w miejscu zajmowanym przez obiekt lustrzany i wyrenderuj scenê,
		//u¿ywaj¹c macierzy rzutowania dla tekstury szeœciennej i odpowiednich ustawieñ widoku
		glm::mat4 MV4 = glm::lookAt(glm::vec3(0),glm::vec3(0,-1,0), glm::vec3(1,0,0));
		DrawScene( MV4*T, Pcubemap);

		//po³¹cz GL_TEXTURE_CUBE_MAP_POSITIVE_Z z przy³¹czem koloru w FBO		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, dynamicCubeMapID, 0);
		//wyczyœæ bufory koloru i g³êbi
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		//ustaw wirtualnego obserwatora w miejscu zajmowanym przez obiekt lustrzany i wyrenderuj scenê,
		//u¿ywaj¹c macierzy rzutowania dla tekstury szeœciennej i odpowiednich ustawieñ widoku
		glm::mat4 MV5 = glm::lookAt(glm::vec3(0),glm::vec3(0,0,1), glm::vec3(0,-1,0));
		DrawScene(MV5*T, Pcubemap);

		//po³¹cz GL_TEXTURE_CUBE_MAP_NEGATIVE_Z z przy³¹czem koloru w FBO		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, dynamicCubeMapID, 0);
		//wyczyœæ bufory koloru i g³êbi
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		//ustaw wirtualnego obserwatora w miejscu zajmowanym przez obiekt lustrzany i wyrenderuj scenê,
		//u¿ywaj¹c macierzy rzutowania dla tekstury szeœciennej i odpowiednich ustawieñ widoku
		glm::mat4 MV6 = glm::lookAt(glm::vec3(0),glm::vec3(0,0,-1), glm::vec3(0,-1,0));
		DrawScene( MV6*T, Pcubemap);

	//odwi¹¿ FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	//przywróæ domyœlne wymiary okna widokowego
	glViewport(0,0,WIDTH,HEIGHT);

	//wyrenderuj scenê z punktu widzenia kamery
	DrawScene(MV, P);

	//wi¹zanie obiektu tablicy wierzcho³ków sfery
	glBindVertexArray(sphereVAOID);

	//renderowanie lustrzanej sfery przy u¿yciu shadera mapy szeœciennej
	cubemapShader.Use();
		//transformacje sfery
		T = glm::translate(glm::mat4(1), p);
		//uniformy shadera
		glUniformMatrix4fv(cubemapShader("MVP"), 1, GL_FALSE, glm::value_ptr(P*(MV*T)));
		glUniform3fv(cubemapShader("eyePosition"), 1,  glm::value_ptr(eyePos));
			//rysowanie trójk¹tów sfery
			glDrawElements(GL_TRIANGLES,indices.size(),GL_UNSIGNED_SHORT,0);

	//wy³¹czanie shadwera
	cubemapShader.UnUse();

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
	glutCreateWindow("Dynamiczne mapowanie szeœcienne - OpenGL 3.3");

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
	glutIdleFunc(OnIdle);

	//wywo³anie pêtli g³ównej
	glutMainLoop();

	return 0;
}
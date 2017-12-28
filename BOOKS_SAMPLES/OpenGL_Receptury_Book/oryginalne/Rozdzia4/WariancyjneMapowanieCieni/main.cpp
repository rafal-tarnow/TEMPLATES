#include <GL/glew.h>

#define _USE_MATH_DEFINES
#include <cmath>

#include <GL/freeglut.h>
#include <iostream>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "../src/GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")

using namespace std;

//rozmiar okna
const int WIDTH  = 1024;
const int HEIGHT = 768;

//wymiary mapy cienia
const int SHADOWMAP_WIDTH = 256;
const int SHADOWMAP_HEIGHT = SHADOWMAP_WIDTH;

//wariacyjne shadery mapowania cieni 
GLSLShader shader;				//g³ówny wariancyjny shader mapowania cieni
GLSLShader firstStep;			//shader pierwszego etapu wyznaczaj¹cy momenty
GLSLShader flatshader;			//shader renderuj¹cy gizmo œwiat³o
GLSLShader gaussianH_shader;	//shader poziomego wyg³adzania gaussowskiego
GLSLShader gaussianV_shader;	//shader pionowego wyg³adzania gaussowskiego

//vertex struct with position and normal
struct Vertex {
	glm::vec3 pos, normal;
};

//tablica wierzcho³ków sfery oraz ID obiektu bufora wiercho³ków
GLuint sphereVAOID;
GLuint sphereVerticesVBO;
GLuint sphereIndicesVBO;

//tablica wierzcho³ków kostki oraz ID obiektu bufora wiercho³ków
GLuint cubeVAOID;
GLuint cubeVerticesVBO;
GLuint cubeIndicesVBO;

//tablica wierzcho³ków p³aszczyzny oraz ID obiektów bufora wiercho³ków
GLuint planeVAOID;
GLuint planeVerticesVBO;
GLuint planeIndicesVBO;

//tablica wierzcho³ków gizma œwiat³a oraz ID obiektu bufora wiercho³ków
GLuint lightVAOID;
GLuint lightVerticesVBO;

//tablica wierzcho³ków pe³noekranowego czworok¹ta i obiekty bufora wierzcho³ków 
GLuint quadVAOID;
GLuint quadVBOID;
GLuint quadIndicesID;

//macierze rzutowania, modelu i widoku 
glm::mat4  P = glm::mat4(1);
glm::mat4  MV = glm::mat4(1);

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=25, rY=-40, dist = -10;

glm::vec3 lightPosOS=glm::vec3(0, 2,0); //po³o¿enie œwiat³a w przestrzeni obiektu
 
#include <vector>

//vwierzcho³ki i indeksy sfery (szeœcianu)
std::vector<Vertex> vertices;
std::vector<GLushort> indices;
int totalSphereTriangles = 0;

//Wspó³rzêdne sferyczne po³o¿enia œwiat³a
float theta = -7;
float phi = -0.77f;
float radius = 7.5f;

//ID tekstury mapy cienia
GLuint shadowMapTexID;

//FBO and render buffer object IDs
GLuint fboID, rboID;

//filtrowanie FBO ID
GLuint filterFBOID;
//filtrowanie tekstury z przy³¹cza koloru w FBO 
GLuint blurTexID[2];

glm::mat4 MV_L; //macierz modelu i widoku dla œwiat³a
glm::mat4 P_L;	//macierz rzutowania dla swiat³a
glm::mat4 B;    //macierz przesuniêcia dla œwiat³a
glm::mat4 BP;   //po³¹czona macierz rzutowania i przesuniêcia
glm::mat4 S;    //po³¹czona macierz MVPB dla œwiat³a

//ping pong IDs
int readID =0, writeID=1;

//tablica przy³¹czy koloru
GLenum attachID[2]={GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

//dodawanie indeksów sfery do wektora indeksów
inline void push_indices(int sectors, int r, int s, std::vector<GLushort>& indices) {
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
void CreateSphere( float radius, unsigned int slices, unsigned int stacks, std::vector<Vertex>& vertices, std::vector<GLushort>& indices)
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
            push_indices(stacks, r, s, indices);
        }
    }

}

//generowanie szeœcianu o zadanym rozmiarze
void CreateCube(const float& size, std::vector<Vertex>& vertices, std::vector<GLushort>& indices) {
	float halfSize = size/2.0f;
	glm::vec3 positions[8];
	positions[0]=glm::vec3(-halfSize,-halfSize,-halfSize);
	positions[1]=glm::vec3( halfSize,-halfSize,-halfSize);
	positions[2]=glm::vec3( halfSize, halfSize,-halfSize);
	positions[3]=glm::vec3(-halfSize, halfSize,-halfSize);
	positions[4]=glm::vec3(-halfSize,-halfSize, halfSize);
	positions[5]=glm::vec3( halfSize,-halfSize, halfSize);
	positions[6]=glm::vec3( halfSize, halfSize, halfSize);
	positions[7]=glm::vec3(-halfSize, halfSize, halfSize);

	glm::vec3 normals[6];
	normals[0]=glm::vec3(-1.0,0.0,0.0);
	normals[1]=glm::vec3(1.0,0.0,0.0);
	normals[2]=glm::vec3(0.0,1.0,0.0);
	normals[3]=glm::vec3(0.0,-1.0,0.0);
	normals[4]=glm::vec3(0.0,0.0,1.0);
	normals[5]=glm::vec3(0.0,0.0,-1.0);

	indices.resize(36);
	vertices.resize(36);

	//wype³nianie tablicy indeksów
	GLushort* id=&indices[0];
	//left face
	*id++ = 7; 	*id++ = 3; 	*id++ = 4;
	*id++ = 3; 	*id++ = 0; 	*id++ = 4;

	//œciana prawa
	*id++ = 2; 	*id++ = 6; 	*id++ = 1;
	*id++ = 6; 	*id++ = 5; 	*id++ = 1;

	//œciana górna
	*id++ = 7; 	*id++ = 6; 	*id++ = 3;
	*id++ = 6; 	*id++ = 2; 	*id++ = 3;
	//œciana dolna
	*id++ = 0; 	*id++ = 1; 	*id++ = 4;
	*id++ = 1; 	*id++ = 5; 	*id++ = 4;

	//œciana przednia
	*id++ = 6; 	*id++ = 4; 	*id++ = 5;
	*id++ = 6; 	*id++ = 7; 	*id++ = 4;
	//back face
	*id++ = 0; 	*id++ = 2; 	*id++ = 1;
	*id++ = 0; 	*id++ = 3; 	*id++ = 2;


	for(int i=0;i<36;i++) {
		int normal_index = i/6;
		vertices[i].pos=positions[indices[i]];
		vertices[i].normal=normals[normal_index];
		indices[i]=i;
	}
}

//generowanie p³aszczyzny o zadanych wymiarach 
void CreatePlane(const float width, const float depth, std::vector<Vertex>& vertices, std::vector<GLushort>& indices) {
	float halfW = width/2.0f;
	float halfD = depth/2.0f;

	indices.resize(6);
	vertices.resize(4);
	glm::vec3 normal=glm::vec3(0,1,0);

	vertices[0].pos = glm::vec3(-halfW,0.01,-halfD); vertices[0].normal=normal;
	vertices[1].pos = glm::vec3(-halfW,0.01, halfD); vertices[1].normal=normal;
	vertices[2].pos = glm::vec3( halfW,0.01, halfD); vertices[2].normal=normal;
	vertices[3].pos = glm::vec3( halfW,0.01,-halfD); vertices[3].normal=normal;

	//wype³nianie tablicy indeksów
	indices[0]=0;
	indices[1]=1;
	indices[2]=2;

	indices[3]=0;
	indices[4]=2;
	indices[5]=3;
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
	else if(button == GLUT_RIGHT_BUTTON)
		state = 2;
	else
		state = 1;
}

//obs³uga ruchów myszy
void OnMouseMove(int x, int y)
{
	if (state == 0)
		dist *= (1 + (y - oldY)/60.0f);
	else if(state ==2) {
		theta += (oldX - x)/60.0f;
		phi += (y - oldY)/60.0f;

		//aktualizacja po³o¿enia œwiat³a
		lightPosOS.x = radius * cos(theta)*sin(phi);
		lightPosOS.y = radius * cos(phi);
		lightPosOS.z = radius * sin(theta)*sin(phi);

		//aktualizacja macierzy MV dla swiat³a
		MV_L = glm::lookAt(lightPosOS,glm::vec3(0,0,0),glm::vec3(0,1,0));
		S = BP*MV_L;
	} else {
		rY += (x - oldX)/5.0f;
		rX += (y - oldY)/5.0f;
	}
	oldX = x;
	oldY = y;

	//wywo³anie funkcji wyœwietlaj¹cej
	glutPostRedisplay();
}

//Inicjalizacja OpenGL
void OnInit() {
	//wczytanie shaderów p³aszczyzny
	flatshader.LoadFromFile(GL_VERTEX_SHADER, "shadery/shader.vert");
	flatshader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/shader.frag");
	//kompilacja i konsolidacja programu shaderowego
	flatshader.CreateAndLinkProgram();
	flatshader.Use();
		//dodawanie atrybutów i uniformów
		flatshader.AddAttribute("vVertex");
		flatshader.AddUniform("MVP");
	flatshader.UnUse(); 

	GL_CHECK_ERRORS

	//wczytanie shaderów poziomego wyg³adzania gaussowskiego
	gaussianH_shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/Passthrough.vert");
	gaussianH_shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/GaussH.frag");
	//kompilacja i konsolidacja programu shaderowego
	gaussianH_shader.CreateAndLinkProgram();
	gaussianH_shader.Use();
		//dodawanie atrybutów i uniformów
		gaussianH_shader.AddAttribute("vVertex");
		gaussianH_shader.AddUniform("textureMap");
		//przekzanie wartoœci uniformów
		glUniform1i(gaussianH_shader("textureMap"),1);
	gaussianH_shader.UnUse();

	GL_CHECK_ERRORS

	//wczytanie shaderów pionowego wyg³adzania gaussowskiego
	gaussianV_shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/Passthrough.vert");
	gaussianV_shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/GaussV.frag");
	//kompilacja i konsolidacja programu shaderowego
	gaussianV_shader.CreateAndLinkProgram();
	gaussianV_shader.Use();
		//dodawanie atrybutów i uniformów
		gaussianV_shader.AddAttribute("vVertex");
		gaussianV_shader.AddUniform("textureMap");
		//przekzanie wartoœci uniformów
		glUniform1i(gaussianV_shader("textureMap"),0);
	gaussianV_shader.UnUse();

	GL_CHECK_ERRORS

	//wczytanie shaderów pierwszego etapu wariancyjnego mapowania cieni
	firstStep.LoadFromFile(GL_VERTEX_SHADER, "shadery/firstStep.vert");
	firstStep.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/firstStep.frag");
	//kompilacja i konsolidacja programu shaderowego
	firstStep.CreateAndLinkProgram();
	firstStep.Use();
		//dodawanie atrybutów i uniformów
		firstStep.AddAttribute("vVertex");
		firstStep.AddUniform("MVP");
	firstStep.UnUse();

	GL_CHECK_ERRORS

	//wczytanie shaderów  wariancyjnego mapowania cieni
	shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/VarianceShadowMapping.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/VarianceShadowMapping.frag");
	//kompilacja i konsolidacja programu shaderowego
	shader.CreateAndLinkProgram();
	shader.Use();
		//dodawanie atrybutów i uniformów
		shader.AddAttribute("vVertex");
		shader.AddAttribute("vNormal");
		shader.AddUniform("MVP");
		shader.AddUniform("MV");
		shader.AddUniform("M");
		shader.AddUniform("N");
		shader.AddUniform("S");
		shader.AddUniform("light_position");
		shader.AddUniform("diffuse_color");
		shader.AddUniform("shadowMap");
		//pprzekzanie wartoœci uniformów
		glUniform1i(shader("shadowMap"),2);
	shader.UnUse();

	GL_CHECK_ERRORS

	//ustalanie geometrii sfery
	CreateSphere(1.0f,10,10, vertices, indices);

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

	//zapisywanie liczby trójk¹tów dla sfery
	totalSphereTriangles = indices.size();

	//czyszczenie wektorów wierzcho³ków i wskaŸników, bo bêd¹ potrzebne do
	//wyznaczania kostki
	vertices.clear();
	indices.clear();

	//ustalanie geometrii kostki
	CreateCube(2,vertices, indices);

	//vao i vbo dla kostki
	glGenVertexArrays(1, &cubeVAOID);
	glGenBuffers(1, &cubeVerticesVBO);
	glGenBuffers(1, &cubeIndicesVBO);
	glBindVertexArray(cubeVAOID);

		glBindBuffer (GL_ARRAY_BUFFER, cubeVerticesVBO);
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
		//przekazanie indeksów kostki do bufora tablicy elementów
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, cubeIndicesVBO);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

	GL_CHECK_ERRORS

	//czyszczenie wektorów wierzcho³ków i wskaŸników, bo bêd¹ potrzebne do
	//wyznaczania p³aszczyzny
	vertices.clear();
	indices.clear();
	//tworzenie obiektu p³aszczyzny
	CreatePlane(100,100,vertices, indices);

	//vao i vbo dla p³aszczyzny
	glGenVertexArrays(1, &planeVAOID);
	glGenBuffers(1, &planeVerticesVBO);
	glGenBuffers(1, &planeIndicesVBO);
	glBindVertexArray(planeVAOID);

		glBindBuffer (GL_ARRAY_BUFFER, planeVerticesVBO);
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
		//przekazanie indeksów p³aszczyzny do bufora tablicy elementów
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, planeIndicesVBO);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

	GL_CHECK_ERRORS

	//wierzcho³ki czworok¹ta pe³noekranowego
	glm::vec2 quadVerts[4];
	quadVerts[0] = glm::vec2(0,0);
	quadVerts[1] = glm::vec2(1,0);
	quadVerts[2] = glm::vec2(1,1);
	quadVerts[3] = glm::vec2(0,1);

	//przekazanie indeksów czworok¹ta
	indices.clear();
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	//generowanietablicy wierzcho³ków czworok¹ta i obiektów bufora wierzcho³ków
	glGenVertexArrays(1, &quadVAOID);
	glGenBuffers(1, &quadVBOID);
	glGenBuffers(1, &quadIndicesID);

	glBindVertexArray(quadVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, quadVBOID);
		//przekazanie wierzcho³ków czworok¹ta do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, sizeof(quadVerts), &quadVerts[0], GL_STATIC_DRAW);

		GL_CHECK_ERRORS
		//w³¹czenie tablicy atrybutów wierzho³ka dla po³o¿enia
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,0,0);
		//przekazanie indeksów czworok¹ta do bufora tablicy elementów
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, quadIndicesID);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

	//vao i vbo dla odcinków wskazuj¹cych po³o¿enie Ÿród³a œwiat³a
	glm::vec3 crossHairVertices[6];
	crossHairVertices[0] = glm::vec3(-0.5f,0,0);
	crossHairVertices[1] = glm::vec3(0.5f,0,0);
	crossHairVertices[2] = glm::vec3(0, -0.5f,0);
	crossHairVertices[3] = glm::vec3(0, 0.5f,0);
	crossHairVertices[4] = glm::vec3(0,0, -0.5f);
	crossHairVertices[5] = glm::vec3(0,0, 0.5f);

	//przygotowanie tablicy wierzcho³ków i obiektu bufora do przechowania gizma œwiat³a
	glGenVertexArrays(1, &lightVAOID);
	glGenBuffers(1, &lightVerticesVBO);
	glBindVertexArray(lightVAOID);
		
		glBindBuffer (GL_ARRAY_BUFFER, lightVerticesVBO);
		//przekazanie wierzcho³ków gizma do bufora
		glBufferData (GL_ARRAY_BUFFER, sizeof(crossHairVertices), &(crossHairVertices[0].x), GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//w³¹czenie tablicy atrybutów wierzho³ka dla po³o¿enia
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,0);


	GL_CHECK_ERRORS

	//wyznaczanie po³o¿enia œwiat³a na podstawie wspó³rzêdnych sferycznych
	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	//generowanie FBO i obiektu bufora renderingu (RBO) potrzebnych do uzyskania g³êbi
	glGenFramebuffers(1,&fboID);
	glGenRenderbuffers(1, &rboID);

	//wi¹zanie FBO i RBO
	glBindFramebuffer(GL_FRAMEBUFFER,fboID);
	glBindRenderbuffer(GL_RENDERBUFFER, rboID);

	//ustawienie rozdzielczoœci mapy cienia 
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
	
	//ustawienie koloru brzegowego 
	GLfloat border[4]={1,0,0,0};
	
	//generowanie tekstury na zerowej jednostce teksturuj¹cej
	glGenTextures(1, &shadowMapTexID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowMapTexID);
	        //parametry tekstury
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,border);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,SHADOWMAP_WIDTH,SHADOWMAP_HEIGHT,0,GL_RGBA,GL_FLOAT,NULL);

		//w³¹czenie mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
		glGenerateMipmap(GL_TEXTURE_2D);

	//ustawienie tekstury mapy cienia jako przy³¹cza koloru
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,shadowMapTexID,0);
	//ustawienie bufora renderingu jako przy³¹cza g³êbi
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID);

	//sprawdzanie kompletnoœci bufora ramki
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status == GL_FRAMEBUFFER_COMPLETE) {
		cout<<"Ustawienie FBO powiodlo sie."<<endl;
	} else {
		cout<<"Problem z ustawieniem FBO."<<endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER,0);

	//ustawienie filtruj¹cego FBO
	glGenFramebuffers(1,&filterFBOID);
	glBindFramebuffer(GL_FRAMEBUFFER,filterFBOID);

	//generowanie dwóch przy³¹czy koloru na jednostkach teksturuj¹cych 1 i 2
	glGenTextures(2, blurTexID);
	for(int i=0;i<2;i++) {
		//parametry tekstury
		glActiveTexture(GL_TEXTURE1+i);
		glBindTexture(GL_TEXTURE_2D, blurTexID[i]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,border);
		//alokowanie obiektu tekstury
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,SHADOWMAP_WIDTH,SHADOWMAP_HEIGHT,0,GL_RGBA,GL_FLOAT,NULL);
		//dodawanie przy³¹czy koloru 0 and 1
		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0+i,GL_TEXTURE_2D,blurTexID[i],0);
	}
	//sprawdzanie kompletnoœci bufora ramki
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status == GL_FRAMEBUFFER_COMPLETE) {
		cout<<"Ustawienie filtrujacego FBO powiodlo sie."<<endl;
	} else {
		cout<<"Problem z ustawieniem filtruj¹cego FBO."<<endl;
	}

	//unbind FBO
	glBindFramebuffer(GL_FRAMEBUFFER,0);

	//wyznaczenie macierzy MV, P i B dla œwiat³a
	MV_L = glm::lookAt(lightPosOS,glm::vec3(0,0,0),glm::vec3(0,1,0));
	P_L  = glm::perspective(50.0f,1.0f,1.0f, 50.0f);
	B    = glm::scale(glm::translate(glm::mat4(1),glm::vec3(0.5,0.5,0.5)), glm::vec3(0.5,0.5,0.5));
	BP   = B*P_L;
	S    = BP*MV_L;

	//w³¹czenie testowania g³êbi i zas³aniania œcianek
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {

	glDeleteTextures(1, &shadowMapTexID);
	glDeleteTextures(2, blurTexID);

	//likwidacja shaderów
	shader.DeleteShaderProgram();
	flatshader.DeleteShaderProgram();
	firstStep.DeleteShaderProgram();
	gaussianH_shader.DeleteShaderProgram();
	gaussianV_shader.DeleteShaderProgram();

	//likwidacja vao i vbo
	glDeleteBuffers(1, &sphereVerticesVBO);
	glDeleteBuffers(1, &sphereIndicesVBO);
	glDeleteVertexArrays(1, &sphereVAOID);

	glDeleteBuffers(1, &cubeVerticesVBO);
	glDeleteBuffers(1, &cubeIndicesVBO);
	glDeleteVertexArrays(1, &cubeVAOID);

	glDeleteBuffers(1, &planeVerticesVBO);
	glDeleteBuffers(1, &planeIndicesVBO);
	glDeleteVertexArrays(1, &planeVAOID);

	glDeleteVertexArrays(1, &quadVAOID);
	glDeleteBuffers(1, &quadVBOID);
	glDeleteBuffers(1, &quadIndicesID);

	glDeleteVertexArrays(1, &lightVAOID);
	glDeleteBuffers(1, &lightVerticesVBO);

	glDeleteFramebuffers(1, &fboID);
	glDeleteFramebuffers(1, &filterFBOID);
	glDeleteRenderbuffers(1, &rboID);

	cout<<"Zamkniecie powiodlo sie"<<endl;
}

//obs³uga zmiany wymiarów okna
void OnResize(int w, int h) {
	//ustalanie wymiarów okna widokowego
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//wyznaczanie macierzy rzutowania
	P = glm::perspective(45.0f, (GLfloat)w/h, 0.1f, 1000.f);
}

//obs³uga sygna³u bezczynnoœci procesora - wywo³uje tylko funkcjê wyœwietlaj¹c¹
void OnIdle() {
	glutPostRedisplay();
}

//funkcja renderuj¹ca scenê
void DrawSceneFirstPass(glm::mat4 View, glm::mat4 Proj) {

	GL_CHECK_ERRORS

	//wi¹zanie shadera pierwszego etapu
	firstStep.Use();
		//wi¹zanie VAO dla p³aszczyzny
		glBindVertexArray(planeVAOID); {
		//ustawianie uniformów shadera
			glUniformMatrix4fv(firstStep("MVP"), 1, GL_FALSE, glm::value_ptr(Proj*View));
				//rysowanie trójk¹tów p³aszczyzny
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		}

	        //renderowanie kostki 
		//wi¹zanie VAO kostki
		glBindVertexArray(cubeVAOID); {
		        //okreœlenie przekszta³ceñ dla kostki
			glm::mat4 T = glm::translate(glm::mat4(1),  glm::vec3(-1,1,0));
			glm::mat4 M = T;
			glm::mat4 MV = View*M;
			glm::mat4 MVP = Proj*MV;
		        //przekazanie uniformu shadera
			glUniformMatrix4fv(firstStep("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
				//rysowanie trójk¹tów kostki
	 			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
		}

		//renderowanie sfery
		//wi¹zanie VAO sfery
		glBindVertexArray(sphereVAOID); {
		        //przekszta³cenia sfery
			glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(1,1,0));
			glm::mat4 M = T;
			glm::mat4 MV = View*M;
			glm::mat4 MVP = Proj*MV;
		        //ustawianie uniformów shadera
			glUniformMatrix4fv(firstStep("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
				//rysowanie trójk¹tów sfery
		 		glDrawElements(GL_TRIANGLES, totalSphereTriangles, GL_UNSIGNED_SHORT, 0);
		}

	//odwi¹zanie shadera pierwszego etapu
	firstStep.UnUse();

	GL_CHECK_ERRORS
}

//renderowanie sceny w ostatnim przebiegu
void DrawScene(glm::mat4 View, glm::mat4 Proj ) {

	GL_CHECK_ERRORS

	//wi¹zanie shadera wariancyjnego mapowania cieni
	shader.Use();
		

		//wi¹zanie VAO p³aszczyzny
		glBindVertexArray(planeVAOID); {
		        //ustawianie uniformów shadera
			glUniform3fv(shader("light_position"),1, &(lightPosOS.x));
			glUniformMatrix4fv(shader("S"), 1, GL_FALSE, glm::value_ptr(S));
			glUniformMatrix4fv(shader("M"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(Proj*View));
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(View));
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(View))));
			glUniform3f(shader("diffuse_color"), 1.0f,1.0f,1.0f);
			        //rysowanie trójk¹tów p³aszczyzny
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		}

	        //renderowanie kostki 
		//wi¹zanie VAO kostki
		glBindVertexArray(cubeVAOID); {
		        //okreœlenie przekszta³ceñ dla kostki
			glm::mat4 T = glm::translate(glm::mat4(1),  glm::vec3(-1,1,0));
			glm::mat4 M = T;
			glm::mat4 MV = View*M;
			glm::mat4 MVP = Proj*MV;
		        //przekazanie uniformów shadera
			glUniformMatrix4fv(shader("S"), 1, GL_FALSE, glm::value_ptr(S));
			glUniformMatrix4fv(shader("M"), 1, GL_FALSE, glm::value_ptr(M));
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
			glUniform3f(shader("diffuse_color"), 1.0f,0.0f,0.0f);
			        //rysowanie trójk¹tów kostki
	 			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
		}
	        //renderowanie sfery
		//wi¹zanie VAO sfery
		glBindVertexArray(sphereVAOID); {
		        //przekszta³cenia sfery
			glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(1,1,0));
			glm::mat4 M = T;
			glm::mat4 MV = View*M;
			glm::mat4 MVP = Proj*MV;
		        //ustawianie uniformów shadera
			glUniformMatrix4fv(shader("S"), 1, GL_FALSE, glm::value_ptr(S));
			glUniformMatrix4fv(shader("M"), 1, GL_FALSE, glm::value_ptr(M));
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
			glUniform3f(shader("diffuse_color"), 0.0f, 0.0f, 1.0f);
			        //rysowanie trójk¹tów sfery
		 		glDrawElements(GL_TRIANGLES, totalSphereTriangles, GL_UNSIGNED_SHORT, 0);
		}
	
	//odwi¹zanie shadera
	shader.UnUse();

	GL_CHECK_ERRORS
}


//funkcja zwrotna wyœwietlania
void OnRender() {

	GL_CHECK_ERRORS

	//czyszczenie buforów koloru i g³êbi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//transformacje kamery
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	  
	//1) Renderowanie sceny z punktu widzenia Ÿród³a œwiat³a
	//w³¹czenie renderowania do FBO
	glBindFramebuffer(GL_FRAMEBUFFER,fboID);
	//ustawienie wymiarów okna widokowego zgodnie z wymiarami tekstury mapy cienia
	glViewport(0,0,SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
		//ustawienie rysowania w przy³¹czu 0
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		//czyszczenie buforów koloru i g³êbi
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			//renderowanie sceny z punktu widzenia  Ÿród³a œwiat³a przy uŸyciu shadera pierwszego przejœcia
			DrawSceneFirstPass(MV_L, P_L);

	//wi¹zanie filtruj¹cego FBO 
	glBindFramebuffer(GL_FRAMEBUFFER,filterFBOID);
	//ustawienie rysowania w przy³¹czu 0
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//wi¹zanie VAO czworok¹ta pe³noekranowego
	glBindVertexArray(quadVAOID);
		//uruchamianie shadera pionowego wyg³adzania gaussowskiego
		gaussianV_shader.Use();
			//renderowanie trójk¹tów czworok¹ta
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	//ustawienie rysowania w przy³¹czu 1
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
		//uruchamianie shadera poziomego wyg³adzania gaussowskiego
		gaussianH_shader.Use();
			//renderowanie trójk¹tów czworok¹ta
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	//odwi¹zanie FBO
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	//przywrócenie renderingu do bufora tylnego
	glDrawBuffer(GL_BACK_LEFT);
	//przywrócenie pierwotnyvh wymiarów okna widokowego
	glViewport(0,0,WIDTH, HEIGHT);
		//zwyk³e renderowanie sceny
		DrawScene(MV, P); 
		  
	//wi¹zanie obiektu tablicy wierzcho³ków gizma œwiat³a
	glBindVertexArray(lightVAOID); {
		//ustawianie shadera p³aszczyzny
		flatshader.Use();
			//ustalenie transformacji œwiat³a i wyrenderowanie trzech odcinków
			glm::mat4 T = glm::translate(glm::mat4(1), lightPosOS);
			glUniformMatrix4fv(flatshader("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV*T));
				glDrawArrays(GL_LINES, 0, 6);
		//odwi¹zanie shadera
		flatshader.UnUse();
	}
        //odwi¹zanie obiektu tablicy wierzcho³ków
	glBindVertexArray(0);	

	//zamiana buforów ekranu
	glutSwapBuffers(); 
}

//obs³uga rolki przewijania zmienia wspó³rzêdn¹ radialn¹ Ÿród³a œwiat³a
//jako ¿e po³o¿enie Ÿród³a jest wyra¿one we wspó³rzêdnych sferycznych promieñ
//okreœla odleg³oœæ Ÿród³a od œrodka uk³adu.
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

	//wyznaczanie nowego po³o¿enia Ÿród³a œwiat³a
	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	//Aktualizacja macierzy MV dla œwiat³a
	MV_L = glm::lookAt(lightPosOS,glm::vec3(0,0,0),glm::vec3(0,1,0));
	S = BP*MV_L;
		
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
	glutCreateWindow("Wariancyjne mapowanie cieni - OpenGL 3.3");

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
	glutMouseWheelFunc(OnMouseWheel);
	glutIdleFunc(OnIdle);
	
	//wywo³anie pêtli g³ównej
	glutMainLoop();

	return 0;
}
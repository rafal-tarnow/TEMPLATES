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
GLSLShader shader;				//g��wny wariancyjny shader mapowania cieni
GLSLShader firstStep;			//shader pierwszego etapu wyznaczaj�cy momenty
GLSLShader flatshader;			//shader renderuj�cy gizmo �wiat�o
GLSLShader gaussianH_shader;	//shader poziomego wyg�adzania gaussowskiego
GLSLShader gaussianV_shader;	//shader pionowego wyg�adzania gaussowskiego

//vertex struct with position and normal
struct Vertex {
	glm::vec3 pos, normal;
};

//tablica wierzcho�k�w sfery oraz ID obiektu bufora wiercho�k�w
GLuint sphereVAOID;
GLuint sphereVerticesVBO;
GLuint sphereIndicesVBO;

//tablica wierzcho�k�w kostki oraz ID obiektu bufora wiercho�k�w
GLuint cubeVAOID;
GLuint cubeVerticesVBO;
GLuint cubeIndicesVBO;

//tablica wierzcho�k�w p�aszczyzny oraz ID obiekt�w bufora wiercho�k�w
GLuint planeVAOID;
GLuint planeVerticesVBO;
GLuint planeIndicesVBO;

//tablica wierzcho�k�w gizma �wiat�a oraz ID obiektu bufora wiercho�k�w
GLuint lightVAOID;
GLuint lightVerticesVBO;

//tablica wierzcho�k�w pe�noekranowego czworok�ta i obiekty bufora wierzcho�k�w 
GLuint quadVAOID;
GLuint quadVBOID;
GLuint quadIndicesID;

//macierze rzutowania, modelu i widoku 
glm::mat4  P = glm::mat4(1);
glm::mat4  MV = glm::mat4(1);

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=25, rY=-40, dist = -10;

glm::vec3 lightPosOS=glm::vec3(0, 2,0); //po�o�enie �wiat�a w przestrzeni obiektu
 
#include <vector>

//vwierzcho�ki i indeksy sfery (sze�cianu)
std::vector<Vertex> vertices;
std::vector<GLushort> indices;
int totalSphereTriangles = 0;

//Wsp�rz�dne sferyczne po�o�enia �wiat�a
float theta = -7;
float phi = -0.77f;
float radius = 7.5f;

//ID tekstury mapy cienia
GLuint shadowMapTexID;

//FBO and render buffer object IDs
GLuint fboID, rboID;

//filtrowanie FBO ID
GLuint filterFBOID;
//filtrowanie tekstury z przy��cza koloru w FBO 
GLuint blurTexID[2];

glm::mat4 MV_L; //macierz modelu i widoku dla �wiat�a
glm::mat4 P_L;	//macierz rzutowania dla swiat�a
glm::mat4 B;    //macierz przesuni�cia dla �wiat�a
glm::mat4 BP;   //po��czona macierz rzutowania i przesuni�cia
glm::mat4 S;    //po��czona macierz MVPB dla �wiat�a

//ping pong IDs
int readID =0, writeID=1;

//tablica przy��czy koloru
GLenum attachID[2]={GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

//dodawanie indeks�w sfery do wektora indeks�w
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

//generowanie prymitywu sfery o zadanym promieniu oraz liczbach po�udnik�w i r�wnole�nik�w
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

//generowanie sze�cianu o zadanym rozmiarze
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

	//wype�nianie tablicy indeks�w
	GLushort* id=&indices[0];
	//left face
	*id++ = 7; 	*id++ = 3; 	*id++ = 4;
	*id++ = 3; 	*id++ = 0; 	*id++ = 4;

	//�ciana prawa
	*id++ = 2; 	*id++ = 6; 	*id++ = 1;
	*id++ = 6; 	*id++ = 5; 	*id++ = 1;

	//�ciana g�rna
	*id++ = 7; 	*id++ = 6; 	*id++ = 3;
	*id++ = 6; 	*id++ = 2; 	*id++ = 3;
	//�ciana dolna
	*id++ = 0; 	*id++ = 1; 	*id++ = 4;
	*id++ = 1; 	*id++ = 5; 	*id++ = 4;

	//�ciana przednia
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

//generowanie p�aszczyzny o zadanych wymiarach 
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

	//wype�nianie tablicy indeks�w
	indices[0]=0;
	indices[1]=1;
	indices[2]=2;

	indices[3]=0;
	indices[4]=2;
	indices[5]=3;
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
	else if(button == GLUT_RIGHT_BUTTON)
		state = 2;
	else
		state = 1;
}

//obs�uga ruch�w myszy
void OnMouseMove(int x, int y)
{
	if (state == 0)
		dist *= (1 + (y - oldY)/60.0f);
	else if(state ==2) {
		theta += (oldX - x)/60.0f;
		phi += (y - oldY)/60.0f;

		//aktualizacja po�o�enia �wiat�a
		lightPosOS.x = radius * cos(theta)*sin(phi);
		lightPosOS.y = radius * cos(phi);
		lightPosOS.z = radius * sin(theta)*sin(phi);

		//aktualizacja macierzy MV dla swiat�a
		MV_L = glm::lookAt(lightPosOS,glm::vec3(0,0,0),glm::vec3(0,1,0));
		S = BP*MV_L;
	} else {
		rY += (x - oldX)/5.0f;
		rX += (y - oldY)/5.0f;
	}
	oldX = x;
	oldY = y;

	//wywo�anie funkcji wy�wietlaj�cej
	glutPostRedisplay();
}

//Inicjalizacja OpenGL
void OnInit() {
	//wczytanie shader�w p�aszczyzny
	flatshader.LoadFromFile(GL_VERTEX_SHADER, "shadery/shader.vert");
	flatshader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/shader.frag");
	//kompilacja i konsolidacja programu shaderowego
	flatshader.CreateAndLinkProgram();
	flatshader.Use();
		//dodawanie atrybut�w i uniform�w
		flatshader.AddAttribute("vVertex");
		flatshader.AddUniform("MVP");
	flatshader.UnUse(); 

	GL_CHECK_ERRORS

	//wczytanie shader�w poziomego wyg�adzania gaussowskiego
	gaussianH_shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/Passthrough.vert");
	gaussianH_shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/GaussH.frag");
	//kompilacja i konsolidacja programu shaderowego
	gaussianH_shader.CreateAndLinkProgram();
	gaussianH_shader.Use();
		//dodawanie atrybut�w i uniform�w
		gaussianH_shader.AddAttribute("vVertex");
		gaussianH_shader.AddUniform("textureMap");
		//przekzanie warto�ci uniform�w
		glUniform1i(gaussianH_shader("textureMap"),1);
	gaussianH_shader.UnUse();

	GL_CHECK_ERRORS

	//wczytanie shader�w pionowego wyg�adzania gaussowskiego
	gaussianV_shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/Passthrough.vert");
	gaussianV_shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/GaussV.frag");
	//kompilacja i konsolidacja programu shaderowego
	gaussianV_shader.CreateAndLinkProgram();
	gaussianV_shader.Use();
		//dodawanie atrybut�w i uniform�w
		gaussianV_shader.AddAttribute("vVertex");
		gaussianV_shader.AddUniform("textureMap");
		//przekzanie warto�ci uniform�w
		glUniform1i(gaussianV_shader("textureMap"),0);
	gaussianV_shader.UnUse();

	GL_CHECK_ERRORS

	//wczytanie shader�w pierwszego etapu wariancyjnego mapowania cieni
	firstStep.LoadFromFile(GL_VERTEX_SHADER, "shadery/firstStep.vert");
	firstStep.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/firstStep.frag");
	//kompilacja i konsolidacja programu shaderowego
	firstStep.CreateAndLinkProgram();
	firstStep.Use();
		//dodawanie atrybut�w i uniform�w
		firstStep.AddAttribute("vVertex");
		firstStep.AddUniform("MVP");
	firstStep.UnUse();

	GL_CHECK_ERRORS

	//wczytanie shader�w  wariancyjnego mapowania cieni
	shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/VarianceShadowMapping.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/VarianceShadowMapping.frag");
	//kompilacja i konsolidacja programu shaderowego
	shader.CreateAndLinkProgram();
	shader.Use();
		//dodawanie atrybut�w i uniform�w
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
		//pprzekzanie warto�ci uniform�w
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
		//przekazanie wierzcho�k�w do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//w��czenie tablicy atrybut�w wierzho�ka dla po�o�enia
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);
		GL_CHECK_ERRORS
		//w��czenie tablicy atrybut�w wierzho�ka dla wektora normalnego
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, normal)));
		GL_CHECK_ERRORS
		//przekazanie indeks�w sfery do bufora tablicy element�w
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, sphereIndicesVBO);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

	//zapisywanie liczby tr�jk�t�w dla sfery
	totalSphereTriangles = indices.size();

	//czyszczenie wektor�w wierzcho�k�w i wska�nik�w, bo b�d� potrzebne do
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
		//przekazanie wierzcho�k�w do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//w��czenie tablicy atrybut�w wierzho�ka dla po�o�enia
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);
		GL_CHECK_ERRORS
		//w��czenie tablicy atrybut�w wierzho�ka dla wektora normalnego
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, normal)));
		GL_CHECK_ERRORS
		//przekazanie indeks�w kostki do bufora tablicy element�w
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, cubeIndicesVBO);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

	GL_CHECK_ERRORS

	//czyszczenie wektor�w wierzcho�k�w i wska�nik�w, bo b�d� potrzebne do
	//wyznaczania p�aszczyzny
	vertices.clear();
	indices.clear();
	//tworzenie obiektu p�aszczyzny
	CreatePlane(100,100,vertices, indices);

	//vao i vbo dla p�aszczyzny
	glGenVertexArrays(1, &planeVAOID);
	glGenBuffers(1, &planeVerticesVBO);
	glGenBuffers(1, &planeIndicesVBO);
	glBindVertexArray(planeVAOID);

		glBindBuffer (GL_ARRAY_BUFFER, planeVerticesVBO);
		//przekazanie wierzcho�k�w do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//w��czenie tablicy atrybut�w wierzho�ka dla po�o�enia
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);
		GL_CHECK_ERRORS
		//w��czenie tablicy atrybut�w wierzho�ka dla wektora normalnego
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, normal)));
		GL_CHECK_ERRORS
		//przekazanie indeks�w p�aszczyzny do bufora tablicy element�w
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, planeIndicesVBO);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

	GL_CHECK_ERRORS

	//wierzcho�ki czworok�ta pe�noekranowego
	glm::vec2 quadVerts[4];
	quadVerts[0] = glm::vec2(0,0);
	quadVerts[1] = glm::vec2(1,0);
	quadVerts[2] = glm::vec2(1,1);
	quadVerts[3] = glm::vec2(0,1);

	//przekazanie indeks�w czworok�ta
	indices.clear();
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	//generowanietablicy wierzcho�k�w czworok�ta i obiekt�w bufora wierzcho�k�w
	glGenVertexArrays(1, &quadVAOID);
	glGenBuffers(1, &quadVBOID);
	glGenBuffers(1, &quadIndicesID);

	glBindVertexArray(quadVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, quadVBOID);
		//przekazanie wierzcho�k�w czworok�ta do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, sizeof(quadVerts), &quadVerts[0], GL_STATIC_DRAW);

		GL_CHECK_ERRORS
		//w��czenie tablicy atrybut�w wierzho�ka dla po�o�enia
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,0,0);
		//przekazanie indeks�w czworok�ta do bufora tablicy element�w
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, quadIndicesID);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

	//vao i vbo dla odcink�w wskazuj�cych po�o�enie �r�d�a �wiat�a
	glm::vec3 crossHairVertices[6];
	crossHairVertices[0] = glm::vec3(-0.5f,0,0);
	crossHairVertices[1] = glm::vec3(0.5f,0,0);
	crossHairVertices[2] = glm::vec3(0, -0.5f,0);
	crossHairVertices[3] = glm::vec3(0, 0.5f,0);
	crossHairVertices[4] = glm::vec3(0,0, -0.5f);
	crossHairVertices[5] = glm::vec3(0,0, 0.5f);

	//przygotowanie tablicy wierzcho�k�w i obiektu bufora do przechowania gizma �wiat�a
	glGenVertexArrays(1, &lightVAOID);
	glGenBuffers(1, &lightVerticesVBO);
	glBindVertexArray(lightVAOID);
		
		glBindBuffer (GL_ARRAY_BUFFER, lightVerticesVBO);
		//przekazanie wierzcho�k�w gizma do bufora
		glBufferData (GL_ARRAY_BUFFER, sizeof(crossHairVertices), &(crossHairVertices[0].x), GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//w��czenie tablicy atrybut�w wierzho�ka dla po�o�enia
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,0);


	GL_CHECK_ERRORS

	//wyznaczanie po�o�enia �wiat�a na podstawie wsp�rz�dnych sferycznych
	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	//generowanie FBO i obiektu bufora renderingu (RBO) potrzebnych do uzyskania g��bi
	glGenFramebuffers(1,&fboID);
	glGenRenderbuffers(1, &rboID);

	//wi�zanie FBO i RBO
	glBindFramebuffer(GL_FRAMEBUFFER,fboID);
	glBindRenderbuffer(GL_RENDERBUFFER, rboID);

	//ustawienie rozdzielczo�ci mapy cienia 
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
	
	//ustawienie koloru brzegowego 
	GLfloat border[4]={1,0,0,0};
	
	//generowanie tekstury na zerowej jednostce teksturuj�cej
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

		//w��czenie mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
		glGenerateMipmap(GL_TEXTURE_2D);

	//ustawienie tekstury mapy cienia jako przy��cza koloru
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,shadowMapTexID,0);
	//ustawienie bufora renderingu jako przy��cza g��bi
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID);

	//sprawdzanie kompletno�ci bufora ramki
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status == GL_FRAMEBUFFER_COMPLETE) {
		cout<<"Ustawienie FBO powiodlo sie."<<endl;
	} else {
		cout<<"Problem z ustawieniem FBO."<<endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER,0);

	//ustawienie filtruj�cego FBO
	glGenFramebuffers(1,&filterFBOID);
	glBindFramebuffer(GL_FRAMEBUFFER,filterFBOID);

	//generowanie dw�ch przy��czy koloru na jednostkach teksturuj�cych 1 i 2
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
		//dodawanie przy��czy koloru 0 and 1
		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0+i,GL_TEXTURE_2D,blurTexID[i],0);
	}
	//sprawdzanie kompletno�ci bufora ramki
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status == GL_FRAMEBUFFER_COMPLETE) {
		cout<<"Ustawienie filtrujacego FBO powiodlo sie."<<endl;
	} else {
		cout<<"Problem z ustawieniem filtruj�cego FBO."<<endl;
	}

	//unbind FBO
	glBindFramebuffer(GL_FRAMEBUFFER,0);

	//wyznaczenie macierzy MV, P i B dla �wiat�a
	MV_L = glm::lookAt(lightPosOS,glm::vec3(0,0,0),glm::vec3(0,1,0));
	P_L  = glm::perspective(50.0f,1.0f,1.0f, 50.0f);
	B    = glm::scale(glm::translate(glm::mat4(1),glm::vec3(0.5,0.5,0.5)), glm::vec3(0.5,0.5,0.5));
	BP   = B*P_L;
	S    = BP*MV_L;

	//w��czenie testowania g��bi i zas�aniania �cianek
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich alokowanych zasob�w
void OnShutdown() {

	glDeleteTextures(1, &shadowMapTexID);
	glDeleteTextures(2, blurTexID);

	//likwidacja shader�w
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

//obs�uga zmiany wymiar�w okna
void OnResize(int w, int h) {
	//ustalanie wymiar�w okna widokowego
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//wyznaczanie macierzy rzutowania
	P = glm::perspective(45.0f, (GLfloat)w/h, 0.1f, 1000.f);
}

//obs�uga sygna�u bezczynno�ci procesora - wywo�uje tylko funkcj� wy�wietlaj�c�
void OnIdle() {
	glutPostRedisplay();
}

//funkcja renderuj�ca scen�
void DrawSceneFirstPass(glm::mat4 View, glm::mat4 Proj) {

	GL_CHECK_ERRORS

	//wi�zanie shadera pierwszego etapu
	firstStep.Use();
		//wi�zanie VAO dla p�aszczyzny
		glBindVertexArray(planeVAOID); {
		//ustawianie uniform�w shadera
			glUniformMatrix4fv(firstStep("MVP"), 1, GL_FALSE, glm::value_ptr(Proj*View));
				//rysowanie tr�jk�t�w p�aszczyzny
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		}

	        //renderowanie kostki 
		//wi�zanie VAO kostki
		glBindVertexArray(cubeVAOID); {
		        //okre�lenie przekszta�ce� dla kostki
			glm::mat4 T = glm::translate(glm::mat4(1),  glm::vec3(-1,1,0));
			glm::mat4 M = T;
			glm::mat4 MV = View*M;
			glm::mat4 MVP = Proj*MV;
		        //przekazanie uniformu shadera
			glUniformMatrix4fv(firstStep("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
				//rysowanie tr�jk�t�w kostki
	 			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
		}

		//renderowanie sfery
		//wi�zanie VAO sfery
		glBindVertexArray(sphereVAOID); {
		        //przekszta�cenia sfery
			glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(1,1,0));
			glm::mat4 M = T;
			glm::mat4 MV = View*M;
			glm::mat4 MVP = Proj*MV;
		        //ustawianie uniform�w shadera
			glUniformMatrix4fv(firstStep("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
				//rysowanie tr�jk�t�w sfery
		 		glDrawElements(GL_TRIANGLES, totalSphereTriangles, GL_UNSIGNED_SHORT, 0);
		}

	//odwi�zanie shadera pierwszego etapu
	firstStep.UnUse();

	GL_CHECK_ERRORS
}

//renderowanie sceny w ostatnim przebiegu
void DrawScene(glm::mat4 View, glm::mat4 Proj ) {

	GL_CHECK_ERRORS

	//wi�zanie shadera wariancyjnego mapowania cieni
	shader.Use();
		

		//wi�zanie VAO p�aszczyzny
		glBindVertexArray(planeVAOID); {
		        //ustawianie uniform�w shadera
			glUniform3fv(shader("light_position"),1, &(lightPosOS.x));
			glUniformMatrix4fv(shader("S"), 1, GL_FALSE, glm::value_ptr(S));
			glUniformMatrix4fv(shader("M"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(Proj*View));
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(View));
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(View))));
			glUniform3f(shader("diffuse_color"), 1.0f,1.0f,1.0f);
			        //rysowanie tr�jk�t�w p�aszczyzny
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		}

	        //renderowanie kostki 
		//wi�zanie VAO kostki
		glBindVertexArray(cubeVAOID); {
		        //okre�lenie przekszta�ce� dla kostki
			glm::mat4 T = glm::translate(glm::mat4(1),  glm::vec3(-1,1,0));
			glm::mat4 M = T;
			glm::mat4 MV = View*M;
			glm::mat4 MVP = Proj*MV;
		        //przekazanie uniform�w shadera
			glUniformMatrix4fv(shader("S"), 1, GL_FALSE, glm::value_ptr(S));
			glUniformMatrix4fv(shader("M"), 1, GL_FALSE, glm::value_ptr(M));
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
			glUniform3f(shader("diffuse_color"), 1.0f,0.0f,0.0f);
			        //rysowanie tr�jk�t�w kostki
	 			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
		}
	        //renderowanie sfery
		//wi�zanie VAO sfery
		glBindVertexArray(sphereVAOID); {
		        //przekszta�cenia sfery
			glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(1,1,0));
			glm::mat4 M = T;
			glm::mat4 MV = View*M;
			glm::mat4 MVP = Proj*MV;
		        //ustawianie uniform�w shadera
			glUniformMatrix4fv(shader("S"), 1, GL_FALSE, glm::value_ptr(S));
			glUniformMatrix4fv(shader("M"), 1, GL_FALSE, glm::value_ptr(M));
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
			glUniform3f(shader("diffuse_color"), 0.0f, 0.0f, 1.0f);
			        //rysowanie tr�jk�t�w sfery
		 		glDrawElements(GL_TRIANGLES, totalSphereTriangles, GL_UNSIGNED_SHORT, 0);
		}
	
	//odwi�zanie shadera
	shader.UnUse();

	GL_CHECK_ERRORS
}


//funkcja zwrotna wy�wietlania
void OnRender() {

	GL_CHECK_ERRORS

	//czyszczenie bufor�w koloru i g��bi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//transformacje kamery
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	  
	//1) Renderowanie sceny z punktu widzenia �r�d�a �wiat�a
	//w��czenie renderowania do FBO
	glBindFramebuffer(GL_FRAMEBUFFER,fboID);
	//ustawienie wymiar�w okna widokowego zgodnie z wymiarami tekstury mapy cienia
	glViewport(0,0,SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
		//ustawienie rysowania w przy��czu 0
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		//czyszczenie bufor�w koloru i g��bi
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			//renderowanie sceny z punktu widzenia  �r�d�a �wiat�a przy u�yciu shadera pierwszego przej�cia
			DrawSceneFirstPass(MV_L, P_L);

	//wi�zanie filtruj�cego FBO 
	glBindFramebuffer(GL_FRAMEBUFFER,filterFBOID);
	//ustawienie rysowania w przy��czu 0
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//wi�zanie VAO czworok�ta pe�noekranowego
	glBindVertexArray(quadVAOID);
		//uruchamianie shadera pionowego wyg�adzania gaussowskiego
		gaussianV_shader.Use();
			//renderowanie tr�jk�t�w czworok�ta
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	//ustawienie rysowania w przy��czu 1
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
		//uruchamianie shadera poziomego wyg�adzania gaussowskiego
		gaussianH_shader.Use();
			//renderowanie tr�jk�t�w czworok�ta
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	//odwi�zanie FBO
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	//przywr�cenie renderingu do bufora tylnego
	glDrawBuffer(GL_BACK_LEFT);
	//przywr�cenie pierwotnyvh wymiar�w okna widokowego
	glViewport(0,0,WIDTH, HEIGHT);
		//zwyk�e renderowanie sceny
		DrawScene(MV, P); 
		  
	//wi�zanie obiektu tablicy wierzcho�k�w gizma �wiat�a
	glBindVertexArray(lightVAOID); {
		//ustawianie shadera p�aszczyzny
		flatshader.Use();
			//ustalenie transformacji �wiat�a i wyrenderowanie trzech odcink�w
			glm::mat4 T = glm::translate(glm::mat4(1), lightPosOS);
			glUniformMatrix4fv(flatshader("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV*T));
				glDrawArrays(GL_LINES, 0, 6);
		//odwi�zanie shadera
		flatshader.UnUse();
	}
        //odwi�zanie obiektu tablicy wierzcho�k�w
	glBindVertexArray(0);	

	//zamiana bufor�w ekranu
	glutSwapBuffers(); 
}

//obs�uga rolki przewijania zmienia wsp�rz�dn� radialn� �r�d�a �wiat�a
//jako �e po�o�enie �r�d�a jest wyra�one we wsp�rz�dnych sferycznych promie�
//okre�la odleg�o�� �r�d�a od �rodka uk�adu.
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

	//wyznaczanie nowego po�o�enia �r�d�a �wiat�a
	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	//Aktualizacja macierzy MV dla �wiat�a
	MV_L = glm::lookAt(lightPosOS,glm::vec3(0,0,0),glm::vec3(0,1,0));
	S = BP*MV_L;
		
	//wywo�anie funkcji wy�wietlaj�cej
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
	glutMouseWheelFunc(OnMouseWheel);
	glutIdleFunc(OnIdle);
	
	//wywo�anie p�tli g��wnej
	glutMainLoop();

	return 0;
}
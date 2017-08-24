#define _USE_MATH_DEFINES

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "..\src\GLSLShader.h"
#include <vector>
#include "Obj.h"

#include <SOIL.h>

#include <cstdlib>

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace std;

//wymiary okna
const int WIDTH  = 1280;
const int HEIGHT = 960;

//wymiary tekstury pozaekranowej
const int RTT_WIDTH = WIDTH/4;
const int RTT_HEIGHT = HEIGHT/4;

//shadery u¿ywane w recepturze 
GLSLShader shader,
			flatShader,
			finalShader,
			ssaoFirstShader,
			ssaoSecondShader,
			gaussianH_shader,
			gaussianV_shader;

//ID tablicy wierzcho³ków i obiektów bufora
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

//macierze rzutowania oraz modelu i widoku
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//instancja obiektu ObjLoader
ObjLoader obj;
vector<Mesh*> meshes;			//wszystkie siatki 
vector<Material*> materials;	//wszystkie materia³y 
vector<unsigned short> indices;	//wszystkie indeksy siatki 
vector<Vertex> vertices;		//wszystkie wierzcho³ki siatki  
vector<GLuint> textures;		//wszystkie tekstury

//identyfikatory tablicy wierzcho³ków i obiektów bufora dla gizma œwiat³a
GLuint lightVAOID;
GLuint lightVerticesVBO;
glm::vec3 lightPosOS=glm::vec3(0,2,0); //po³o¿enie œwiat³a w przestrzenie obiektu

//wspó³rzêdne sferyczne dla obrotów œwiat³a
float theta = 1.6f;
float phi = -0.6f;
float radius = 70;

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=42, rY=180, dist = -80;

//nazwa pliku z siatk¹ OBJ do wczytania
const std::string mesh_filename = "../media/blocks.obj";

//identyfikatory FBO dla normalnych i dla filtrowania
GLuint fboID, filterFBOID;
//identyfikatory przy³¹czy koloru i g³êbi
GLuint normalTextureID, depthTextureID;
//identyfikatory przefiltrowanej tekstury w przy³¹czu koloru 
GLuint blurTexID[2];

//identyfikatory tablicy wierzcho³ków i obiektów bufora dla czworok¹ta
GLuint quadVAOID;
GLuint quadVBOID;
GLuint quadIndicesID;

//ID tekstury szumu
GLuint noiseTexID;

//promieñ próbkowania dla SSAO
float sampling_radius = 0.25f;
//znacznik w³¹czaj¹cy i wy³¹czaj¹cy SSAO
bool bUseSSAO = true;

//inicjalizacja obiektów FBO
void InitFBO() {
//ustawianie FBO dla renderingu pozaekranowego
	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);

	//generowanie jednej tekstury dla koloru i jednej dla g³êbi
	glGenTextures(1, &normalTextureID);
	glGenTextures(1, &depthTextureID);

	//przy³¹czanie tekstury koloru do jednostki 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalTextureID);

//ustawianie parametrów tekstury
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, RTT_WIDTH, RTT_HEIGHT, 0, GL_BGRA, GL_FLOAT, NULL);

	//przy³¹czanie tekstury koloru do jednostki 3
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthTextureID);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, RTT_WIDTH, RTT_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	//przy³¹czanie tekstur g³êbi oraz koloru do FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normalTextureID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D, depthTextureID, 0);

	//sprawdzian kompletnoœci FBO
	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if(status==GL_FRAMEBUFFER_COMPLETE) {
		printf("Ustawienie FBO powiodlo siê");
	} else {
		printf("Problem z ustawieniem FBO.");
	}

//ustawianie FBO dla filtrowania
	glGenFramebuffers(1,&filterFBOID);
	glBindFramebuffer(GL_FRAMEBUFFER,filterFBOID);

	//ustawianie dwóch tekstur koloru dla filtrowania
	glGenTextures(2, blurTexID);
	for(int i=0;i<2;i++) {
		glActiveTexture(GL_TEXTURE4+i);
		glBindTexture(GL_TEXTURE_2D, blurTexID[i]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,RTT_WIDTH,RTT_HEIGHT,0,GL_RGBA,GL_FLOAT,NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0+i,GL_TEXTURE_2D,blurTexID[i],0);
	}

	//sprawdzian kompletnoœci FBO
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status == GL_FRAMEBUFFER_COMPLETE) {
		cout<<"\nUstawienie filtrujacego FBO powiodlo siê."<<endl;
	} else {
		cout<<"Problem z ustawieniem filtruj¹cego FBO."<<endl;
	}

//wi¹zanie jednostki teksturuj¹cej nr 0, bo tutaj bêd¹ ³adowane  
	//tekstury modelu
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//odwi¹zanie FBO 
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//zwalnianie wszystkich zasobów zwi¹zanych z FBO
void ShutdownFBO() {
	glDeleteTextures(2, blurTexID);
	glDeleteTextures(1, &normalTextureID);
	glDeleteTextures(1, &depthTextureID);
	glDeleteFramebuffers(1, &fboID);
	glDeleteFramebuffers(1, &filterFBOID);
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

		//aktualizowanie po³o¿enia œwiat³a 
		lightPosOS.x = radius * cos(theta)*sin(phi);
		lightPosOS.y = radius * cos(phi);
		lightPosOS.z = radius * sin(theta)*sin(phi);

	} else {
		rY += (x - oldX)/5.0f;
		rX += (y - oldY)/5.0f;
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}

//inicjalizacja OpenGL
void OnInit() {

	//generowanie pseudolosowego szumu
	glm::vec4 pData[64][64];
	for(int j=0;j<64;j++) {
		for(int i=0;i<64;i++) {
			pData[i][j].x = (float)rand() / RAND_MAX;
			pData[i][j].y = (float)rand() / RAND_MAX;
			pData[i][j].z = (float)rand() / RAND_MAX;
			pData[i][j].w = (float)rand() / RAND_MAX;
		}
	}

	//generowanie tekstury pseudolosowego szumu
	glGenTextures(1, &noiseTexID);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noiseTexID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 64, 64, 0, GL_BGRA, GL_FLOAT, pData);

	//wyznaczanie œcie¿ki do wczytywanych tekstur
	std::string mesh_path = mesh_filename.substr(0, mesh_filename.find_last_of("/")+1);

	//wczytanie modelu OBJ
	if(!obj.Load(mesh_filename.c_str(), meshes, vertices, indices, materials)) {
		cout<<"Cannot load the Obj mesh"<<endl;
		exit(EXIT_FAILURE);
	}
	GL_CHECK_ERRORS

	//uaktywnienie jednostki teksturuj¹cej nr 0
	glActiveTexture(GL_TEXTURE0);
	//za³adowanie tekstur materia³u
	for(size_t k=0;k<materials.size();k++) {
		//jeœli nazwa tekstury rozproszenia nie jest pusta
		if(materials[k]->map_Kd != "") {
			GLuint id = 0;

			//wygeneruj now¹ teksturê w OpenGL
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			int texture_width = 0, texture_height = 0, channels=0;

			const string& filename =  materials[k]->map_Kd;

			std::string full_filename = mesh_path;
			full_filename.append(filename);

			//wczytaj teksturê przy u¿yciu biblioteki SOIL
			GLubyte* pData = SOIL_load_image(full_filename.c_str(), &texture_width, &texture_height, &channels, SOIL_LOAD_AUTO);
			if(pData == NULL) {
				cerr<<"Nie moge wczytac obrazu: "<<full_filename.c_str()<<endl;
				exit(EXIT_FAILURE);
			}

			//Odwracanie obrazu w pionie
			int i,j;
			for( j = 0; j*2 < texture_height; ++j )
			{
				int index1 = j * texture_width * channels;
				int index2 = (texture_height - 1 - j) * texture_width * channels;
				for( i = texture_width * channels; i > 0; --i )
				{
					GLubyte temp = pData[index1];
					pData[index1] = pData[index2];
					pData[index2] = temp;
					++index1;
					++index2;
				}
			}
			//ustalanie formatu obrazu
			GLenum format = GL_RGBA;
			switch(channels) {
				case 2:	format = GL_RG32UI; break;
				case 3: format = GL_RGB;	break;
				case 4: format = GL_RGBA;	break;
			}
			//alokowanie teksturye
			glTexImage2D(GL_TEXTURE_2D, 0, format, texture_width, texture_height, 0, format, GL_UNSIGNED_BYTE, pData);

			//zwalnianie zasobów zajêtych przez bibliotekê SOIL
			SOIL_free_image_data(pData);

			//dodaj identyfikator tekstury do wektora
			textures.push_back(id);
		}
	}
	GL_CHECK_ERRORS

	//wczytywanie shadera p³aszczyzny
	flatShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/flat.vert");
	flatShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/flat.frag");
	//kompilacja i konsolidacja programu shaderowego
	flatShader.CreateAndLinkProgram();
	flatShader.Use();
		//dodawanie atrybutów i uniformów
		flatShader.AddAttribute("vVertex");
		flatShader.AddUniform("MVP");
	flatShader.UnUse();

	//wczytanie shadera finalnego
	finalShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/Passthrough.vert");
	finalShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/final.frag");
	//kompilacja i konsolidacja programu shaderowego
	finalShader.CreateAndLinkProgram();
	finalShader.Use();
		//dodawanie atrybutów i uniformów
		finalShader.AddAttribute("vVertex");
		finalShader.AddUniform("MVP");
		finalShader.AddUniform("textureMap");
		//ustalanie wartoœci sta³ych uniformów
		glUniform1i(finalShader("textureMap"), 4);
	finalShader.UnUse();

	//wczytywanie shadera renderuj¹cego œwiat³o punktowe
	shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/shader.frag");
	shader.CreateAndLinkProgram();
	shader.Use();
		//dodawanie atrybutów i uniformów
		shader.AddAttribute("vVertex");
		shader.AddAttribute("vNormal");
		shader.AddAttribute("vUV");

		shader.AddUniform("MV");
		shader.AddUniform("N");
		shader.AddUniform("P");
		shader.AddUniform("textureMap");
		shader.AddUniform("useDefault");

		shader.AddUniform("light_position");
		shader.AddUniform("diffuse_color");
		//ustalanie wartoœci sta³ych uniformów
		glUniform1i(shader("textureMap"), 0);
	shader.UnUse();

	//wczytywanie shadera wyg³adzaj¹cego poziomo
	gaussianH_shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/Passthrough.vert");
	gaussianH_shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/GaussH.frag");
	//kompilacja i konsolidacja programu shaderowego
	gaussianH_shader.CreateAndLinkProgram();
	gaussianH_shader.Use();
		//dodawanie atrybutów i uniformów
		gaussianH_shader.AddAttribute("vVertex");
		gaussianH_shader.AddUniform("textureMap");
		//ustalanie wartoœci sta³ych uniformów
		glUniform1i(gaussianH_shader("textureMap"),5);
	gaussianH_shader.UnUse();

	//wczytywanie shadera wyg³adzaj¹cego pionowo
	gaussianV_shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/Passthrough.vert");
	gaussianV_shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/GaussV.frag");

	//kompilacja i konsolidacja programu shaderowego
	gaussianV_shader.CreateAndLinkProgram();
	gaussianV_shader.Use();
		//dodawanie atrybutów i uniformów
		gaussianV_shader.AddAttribute("vVertex");
		gaussianV_shader.AddUniform("textureMap");
		//ustalanie wartoœci sta³ych uniformów
		glUniform1i(gaussianV_shader("textureMap"),4);
	gaussianV_shader.UnUse();

	//wczytanie shadera pierwszego etapu SSAO
	ssaoFirstShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/SSAO_FirstStep.vert");
	ssaoFirstShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/SSAO_FirstStep.frag");
	//kompilacja i konsolidacja programu shaderowego
	ssaoFirstShader.CreateAndLinkProgram();
	ssaoFirstShader.Use();
		//dodawanie atrybutów i uniformów
		ssaoFirstShader.AddAttribute("vVertex");
		ssaoFirstShader.AddAttribute("vNormal");
		ssaoFirstShader.AddUniform("MVP");
		ssaoFirstShader.AddUniform("N");
	ssaoFirstShader.UnUse();

	//wczytanie shadera drugiego etapu SSAO
	ssaoSecondShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/Passthrough.vert");
	ssaoSecondShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/SSAO_SecondStep.frag");
	//kompilacja i konsolidacja programu shaderowego
	ssaoSecondShader.CreateAndLinkProgram();
	ssaoSecondShader.Use();
		//dodawanie atrybutów i uniformów
		ssaoSecondShader.AddAttribute("vVertex");
		ssaoSecondShader.AddUniform("samples");
		ssaoSecondShader.AddUniform("invP");
		ssaoSecondShader.AddUniform("normalTex");
		ssaoSecondShader.AddUniform("depthTex");
		ssaoSecondShader.AddUniform("noiseTex");
		ssaoSecondShader.AddUniform("radius");
		ssaoSecondShader.AddUniform("viewportSize");
		ssaoSecondShader.AddUniform("invViewportSize");

		//ustalanie wartoœci sta³ych uniformów
		glUniform2f(ssaoSecondShader("viewportSize"), float(RTT_WIDTH), float(RTT_HEIGHT));
		glUniform2f(ssaoSecondShader("invViewportSize"), 1.0f/float(RTT_WIDTH), 1.0f/float(RTT_HEIGHT));
		glUniform1i(ssaoSecondShader("normalTex"),1);
		glUniform1i(ssaoSecondShader("noiseTex"),2);
		glUniform1i(ssaoSecondShader("depthTex"),3);

		glm::mat4 biasMat;
		biasMat = glm::translate(glm::mat4(1),glm::vec3(0.5,0.5,0.5));
		biasMat = glm::scale(biasMat, glm::vec3(0.5,0.5,0.5));
		glm::mat4 invP = biasMat*glm::inverse(P);
		glUniformMatrix4fv(ssaoSecondShader("invP"), 1, GL_FALSE, glm::value_ptr(invP));

		glm::vec2 samples[16];
		float angle = (float)M_PI_4;
		for(int i=0;i<16;i++) {
			samples[i].x = cos(angle) * (float)(i+1)/16.0f;
			samples[i].y = sin(angle) * (float)(i+1)/16.0f;
			angle += (float)M_PI_2;
			if(((i + 1) % 4) == 0)
				angle += (float)M_PI_4;
		}
		glUniform2fv(ssaoSecondShader("samples"), 16, &(samples[0].x));
	ssaoSecondShader.UnUse();

	GL_CHECK_ERRORS


	//ustawianie obiektu tablicy wierzcho³ków i obiektów bufora dla siatki
	//obs³uga geometrii 
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);

	glBindVertexArray(vaoID);
		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		//przekazanie wierzcho³ków siatki
		glBufferData (GL_ARRAY_BUFFER, sizeof(Vertex)*vertices.size(), &(vertices[0].pos.x), GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//w³¹czenie tablicy atrybutów wierzcho³ka dla po³o¿enia
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);
		GL_CHECK_ERRORS
		//w³¹czenie tablicy atrybutów wierzcho³ka dla normalnej
		glEnableVertexAttribArray(shader["vNormal"]);
		glVertexAttribPointer(shader["vNormal"], 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, normal)) );

		GL_CHECK_ERRORS
		//w³¹czenie tablicy atrybutów wierzcho³ka dla wspó³rzêdnych tekstury
		glEnableVertexAttribArray(shader["vUV"]);
		glVertexAttribPointer(shader["vUV"], 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, uv)) );

		//jeœli materia³ jest jeden, to znaczy, ¿e model zawiera jedn¹ siatkê
		//wiêc ³adujemy go bufora tablicy elementów
		if(materials.size()==1) {
			//przekazanie indeksów do bufora tablicy elementów
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*indices.size(), &(indices[0]), GL_STATIC_DRAW);
		}
		GL_CHECK_ERRORS

	//wierzcho³ki pe³noekranowego czworok¹ta
	glm::vec2 quadVerts[4];
	quadVerts[0] = glm::vec2(0,0);
	quadVerts[1] = glm::vec2(1,0);
	quadVerts[2] = glm::vec2(1,1);
	quadVerts[3] = glm::vec2(0,1);

	//indeksy czworok¹ta
	GLushort quadIndices[]={ 0,1,2,0,2,3};

	//generowanie tablicy wierzcho³ków i obiektów bufora dla czworok¹ta
	glGenVertexArrays(1, &quadVAOID);
	glGenBuffers(1, &quadVBOID);
	glGenBuffers(1, &quadIndicesID);

	glBindVertexArray(quadVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, quadVBOID);
		//przekazanie wierzcho³ków czworok¹ta do obiektu bufora wierzcho³ków
		glBufferData (GL_ARRAY_BUFFER, sizeof(quadVerts), &quadVerts[0], GL_STATIC_DRAW);

		GL_CHECK_ERRORS
		//w³¹czenie tablicy atrybutów wierzcho³ka dla po³o¿enia
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,0,0);
		//przekazanie indeksów czworok¹ta do bufora tablicy elementów
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, quadIndicesID);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), &quadIndices[0], GL_STATIC_DRAW);


	//glBindVertexArray(0);

	//vao i vbo dla po³o¿enia gizma œwiat³a
	glm::vec3 crossHairVertices[6];
	crossHairVertices[0] = glm::vec3(-0.5f,0,0);
	crossHairVertices[1] = glm::vec3(0.5f,0,0);
	crossHairVertices[2] = glm::vec3(0, -0.5f,0);
	crossHairVertices[3] = glm::vec3(0, 0.5f,0);
	crossHairVertices[4] = glm::vec3(0,0, -0.5f);
	crossHairVertices[5] = glm::vec3(0,0, 0.5f);

	//identyfikatory tablicy wierzcho³ków i obiektów bufora dla gizma œwiat³a
	glGenVertexArrays(1, &lightVAOID);
	glGenBuffers(1, &lightVerticesVBO);
	glBindVertexArray(lightVAOID);

	glBindBuffer (GL_ARRAY_BUFFER, lightVerticesVBO);
	//przekazanie wierzcho³ków gizma œwiat³a do obiektu bufora
	glBufferData (GL_ARRAY_BUFFER, sizeof(crossHairVertices), &(crossHairVertices[0].x), GL_STATIC_DRAW);
	GL_CHECK_ERRORS
	//w³¹czenie tablicy atrybutów wierzcho³ka dla po³o¿enia
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,0);

	GL_CHECK_ERRORS

	//wyznaczanie po³o¿enia œwiat³a na podstawie wspó³rzêdnych sferycznych
	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	//w³¹czenie testowania g³êbi i zas³aniania
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//ustawienie niebieskofioletowego koloru
	glClearColor(0.5,0.5,1,1);

	//inicjalizacja FBO
	InitFBO();
	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {
	ShutdownFBO();

	//usuwanie tekstur
	size_t total_textures = textures.size();
	for(size_t i=0;i<total_textures;i++) {
		glDeleteTextures(1, &textures[i]);
	}
	textures.clear();

	//usuwanie siatek
	size_t total_meshes = meshes.size();
	for(size_t i=0;i<total_meshes;i++) {
		delete meshes[i];
		meshes[i]=0;
	}
	meshes.clear();

	size_t total_materials = materials.size();
	for( size_t i=0;i<total_materials;i++) {
		delete materials[i];
		materials[i] = 0;
	}
	materials.clear();

	//usuwanie tekstur
	glDeleteTextures(1, &noiseTexID);

	//likwidacja programu shaderowego
	shader.DeleteShaderProgram();
	ssaoFirstShader.DeleteShaderProgram();
	ssaoSecondShader.DeleteShaderProgram();
	gaussianH_shader.DeleteShaderProgram();
	gaussianV_shader.DeleteShaderProgram();
	finalShader.DeleteShaderProgram();
	flatShader.DeleteShaderProgram();

	//likwidacja vao i vbo
	glDeleteBuffers(1, &vboVerticesID);
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID);

	glDeleteVertexArrays(1, &quadVAOID);
	glDeleteBuffers(1, &quadVBOID);
	glDeleteBuffers(1, &quadIndicesID);

	glDeleteVertexArrays(1, &lightVAOID);
	glDeleteBuffers(1, &lightVerticesVBO);
	cout<<"Zamkniecie powiodlo sie"<<endl;
}

//obs³uga zmiany wymiarów okna
void OnResize(int w, int h) {
	//ustawienie wymiarów okna widokowego
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//wyznaczanie macierzy rzutowania
	P = glm::perspective(60.0f,(float)w/h, 0.1f,1000.0f);
}

//funkcja zwrotna wyœwietlania
void OnRender() {
	//czyszczenie buforów koloru i g³êbi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//transformacje widoku
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	
	//wi¹zanie obiektu tablicy wierzcho³ków siatki
	glBindVertexArray(vaoID); {
		//wi¹zanie shadera siatki
		shader.Use();
			//ustawianie uniformów shadera
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
			glUniformMatrix4fv(shader("P"), 1, GL_FALSE, glm::value_ptr(P));
			glUniform3fv(shader("light_position"),1, &(lightPosOS.x));
			//pêtla po wszystkich materia³ach
			for(size_t i=0;i<materials.size();i++) {
				Material* pMat = materials[i];
				//jeœli nazwa pliku tekstury nie jest pusta
				if(pMat->map_Kd !="") {
					glUniform1f(shader("useDefault"), 0.0);

					//pobierz identyfikator aktualnie zwi¹zanej tekstury i sprawdŸ, czy ró¿ni siê od identyfikatora tekstury bie¿¹cej
					//jeœli tak, zwi¹¿ now¹ teksturê 
					GLint whichID[1];
					glGetIntegerv(GL_TEXTURE_BINDING_2D, whichID);
					if(whichID[0] != textures[i]) {
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, textures[i]);
					}
				} else
					//a jeœli nie ma tekstury, u¿yj koloru domyœlnego
					glUniform1f(shader("useDefault"), 1.0);

				//jeœli materia³ jest jeden, renderujemy ca³¹ siatkê siatkê naraz
				if(materials.size()==1)
					glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);
				else
					//w przeciwnym razie renderuj siatkê sk³adow¹
					glDrawElements(GL_TRIANGLES, pMat->count, GL_UNSIGNED_SHORT, (const GLvoid*)(&indices[pMat->offset]));
			}
		//odwi¹zanie shadera
		shader.UnUse();
	}
	//jeœli SSAO jest w³¹czone
	if(bUseSSAO) {
		//wi¹zanie FBO
		glBindFramebuffer(GL_FRAMEBUFFER, fboID);
		//ustaw wymiary okna widokowego równe wymiarom pozaekranowego celu renderingu
		glViewport(0,0,RTT_WIDTH, RTT_HEIGHT);
		//ustawianie zerowego przy³¹cza koloru jako bufora rysowania
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
			//czyszczenie buforów koloru i g³êbi
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

			//wi¹zanie obiektu tablicy wierzcho³ków siatki
			glBindVertexArray(vaoID); {
			//wi¹zanie shadera 
			ssaoFirstShader.Use();
				//ustawianie uniformów shadera
				glUniformMatrix4fv(ssaoFirstShader("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV));
				glUniformMatrix3fv(ssaoFirstShader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
				//pêtla po wszystkich materia³ach
				for(size_t i=0;i<materials.size();i++) {
					Material* pMat = materials[i];
					//jeœli materia³ jest jeden, renderujemy ca³¹ siatkê siatkê naraz
					if(materials.size()==1)
						glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);
					else
						//w przeciwnym razie renderuj siatkê sk³adow¹
						glDrawElements(GL_TRIANGLES, pMat->count, GL_UNSIGNED_SHORT, (const GLvoid*)(&indices[pMat->offset]));
				}
			//wy³¹czenie shadera pierwszego etapu SSAO		
			ssaoFirstShader.UnUse();
		} 

	 	GL_CHECK_ERRORS

		//ustawianie FBO dla filtrowania
		glBindFramebuffer(GL_FRAMEBUFFER,filterFBOID);
		//ustawianie zerowego przy³¹cza koloru jako bufora rysowania
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		//wi¹zanie obiektu tablicy wierzcho³ków czworok¹ta
		glBindVertexArray(quadVAOID);
			//uruchomienie shadera drugiego etapu SSAO
			ssaoSecondShader.Use();
				//ustawianie uniformów shadera
				glUniform1f(ssaoSecondShader("radius"), sampling_radius);
				//rysowanie pe³noekranowego czworok¹ta
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
			//wy³¹czenie shadera drugiego etapu SSAO
			ssaoSecondShader.UnUse();

		GL_CHECK_ERRORS

		//ustawianie pierwszego przy³¹cza koloru jako bufora rysowania
		glDrawBuffer(GL_COLOR_ATTACHMENT1);

		//ponowne renderowanie czworok¹ta z u¿yciem shadera wyg³adzaj¹cego w pionie
		glBindVertexArray(quadVAOID);
			gaussianV_shader.Use();
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

		//ustawianie drugiego przy³¹cza koloru jako bufora rysowania i renderowanie czworok¹ta 
		//z u¿yciem shadera wyg³adzaj¹cego w poziomie
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
			gaussianH_shader.Use();
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

		//odwi¹¿ FBO i przywróæ domyœlne okno widokowe oraz bufor rysowania
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport(0,0,WIDTH, HEIGHT);
		glDrawBuffer(GL_BACK_LEFT);

		//po³¹cz przefiltrowany rezultat SSAO ze zwyk³ym renderingiem
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		finalShader.Use();
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		finalShader.UnUse();

		//wy³¹czenie mieszania
		glDisable(GL_BLEND);
	}

	//wy³¹czenie testowania g³êbi
	glDisable(GL_DEPTH_TEST);

//rysowanie gizma œwiat³a
	glBindVertexArray(lightVAOID); {
		//transformacje gizma œwiat³a
		glm::mat4 T = glm::translate(glm::mat4(1), lightPosOS);
		//wi¹zanie shadera
		flatShader.Use();
			//ustawianie uniformów shadera i rysowanie linii
			glUniformMatrix4fv(flatShader("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV*T));
				glDrawArrays(GL_LINES, 0, 6);
		//odwi¹zanie shadera
		flatShader.UnUse();
	}
	//w³¹czenie testowania g³êbi
	glEnable(GL_DEPTH_TEST);

	//zamiana buforów w celu wyœwietlenia wyrenderowanego obrazu
	glutSwapBuffers();
}

//obs³uga rolki do przewijania w celu przesuniêcia Ÿród³a œwiat³a
void OnMouseWheel(int button, int dir, int x, int y) {

	if (dir > 0)
    {
        radius += 0.1f;
    }
    else
    {
        radius -= 0.1f;
    }

	//aktualizowanie po³o¿enia œwiat³a 
	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	//wywo³anie funkcji wyœwietlaj¹cej
	glutPostRedisplay();
}

//obs³uga klawiatury
void OnKey(unsigned char k, int x, int y) {
	switch(k) {
		case '-': sampling_radius-=0.01f; break;
		case '+': sampling_radius+=0.01f; break;
		case ' ': bUseSSAO = !bUseSSAO; break;
	}
	sampling_radius = min(5.0f,max(0.0f,sampling_radius));
	std::cout<<"rad: "<<sampling_radius<<std::endl;
	glutPostRedisplay();
}

int main(int argc, char** argv) {
//inicjalizacja freeglut 
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Okluzja otoczenia w przestrzeni ekranu (SSAO) - OpenGL 3.3");

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
	err = glGetError(); //w celu pominiêcia b³êdu 1282 INVALID ENUM
	GL_CHECK_ERRORS

	//wyprowadzanie informacji sprzêtowych
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

	//wywo³anie pêtli g³ównej
	glutMainLoop();

	return 0;
}

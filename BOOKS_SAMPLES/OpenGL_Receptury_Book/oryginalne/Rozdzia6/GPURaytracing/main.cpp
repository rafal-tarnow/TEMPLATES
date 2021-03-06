﻿#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "..\src\GLSLShader.h"
#include <vector>
#include "Obj.h"

#include <SOIL.h>

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace std;

//wymiary okna
const int WIDTH  = 1280;
const int HEIGHT = 960;

//shadery używane w recepturze 
//shader renderujący siatkę, shader śledzący promienie i shader płaszczyzny
GLSLShader shader, raytraceShader, flatShader;

//ID tablicy wierzchołków i obiektów bufora
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

//macierze rzutowania oraz modelu i widoku
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//instancja obiektu ObjLoader
ObjLoader obj;
vector<Mesh*> meshes;					//wszystkie siatki 
vector<Material*> materials;			//wszystkie materiały 
vector<unsigned short> indices;			//wszystkie indeksy siatki 
vector<Vertex> vertices;				//wszystkie wierzchołki siatki  
vector<GLuint> textures;				//wszystkie tekstury

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=22, rY=116, dist = -120;

//nazwa pliku z siatką OBJ do wczytania
const std::string mesh_filename = "../media/blocks.obj";

//znacznik włączający i wyłączający śledzenie promieni
bool bRaytrace = false;

//VAO i VAO czworokąta pełnoekranowego
GLuint quadVAOID;
GLuint quadVBOID;
GLuint quadIndicesID;

//kolor tła
glm::vec4 bg = glm::vec4(0.5,0.5,1,1);

//położenia oka
glm::vec3 eyePos;

//wyrównany z osiami współrzędnych prostopadłościan otaczający scenę
BBox aabb;

GLuint texVerticesID; //tekstura przechowująca położenia wierzchołków
GLuint texTrianglesID; //tekstura przechowująca listę trójkątów 

//identyfikatory tablicy wierzchołków i obiektów bufora dla gizma światła
GLuint lightVAOID;
GLuint lightVerticesVBO;
glm::vec3 lightPosOS=glm::vec3(0, 2,0); //położenie światła w przestrzenie obiektu

//współrzędne sferyczne dla obrotów światła
float theta = 0.66f;
float phi = -1.0f;
float radius = 70;

//zmienne związane liczbą klatek na sekundę
int total_frames = 0;
float fps = 0;
float lastTime =0;

//identyfikator tablicy tekstur
GLuint textureID;

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
}

//obsługa ruchów myszy
void OnMouseMove(int x, int y)
{
	if (state == 0)
		dist *= (1 + (y - oldY)/60.0f);
	else if(state ==2) {
		theta += (oldX - x)/60.0f;
		phi += (y - oldY)/60.0f;

		//aktualizowanie położenia światła 
		lightPosOS.x = radius * cos(theta)*sin(phi);
		lightPosOS.y = radius * cos(phi);
		lightPosOS.z = radius * sin(theta)*sin(phi);

	} else {
		rY += (x - oldX)/5.0f;
		rX += (y - oldY)/5.0f;
	}
	oldX = x;
	oldY = y;


	//wywołanie funkcji wyświetlającej
	glutPostRedisplay();
}

//inicjalizacja OpenGL
void OnInit() {
	//ustawianie geometrii pełnoekranowego czworokąta
	glm::vec2 quadVerts[4];
	quadVerts[0] = glm::vec2(-1,-1);
	quadVerts[1] = glm::vec2(1,-1);
	quadVerts[2] = glm::vec2(1,1);
	quadVerts[3] = glm::vec2(-1,1);
	//indeksy czworokąta
	GLushort quadIndices[]={ 0,1,2,0,2,3};
	//generowanie tablicy wierzchołków i obiektów bufora dla czworokąta
	glGenVertexArrays(1, &quadVAOID);
	glGenBuffers(1, &quadVBOID);
	glGenBuffers(1, &quadIndicesID);

	glBindVertexArray(quadVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, quadVBOID);
		//przekazanie wierzchołków czworokąta do obiektu bufora wierzchołków
		glBufferData (GL_ARRAY_BUFFER, sizeof(quadVerts), &quadVerts[0], GL_STATIC_DRAW);

		GL_CHECK_ERRORS
		//włączenie tablicy atrybutów wierzchołka dla położenia
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,0,0);

		//przekazanie indeksów czworokąta do bufora tablicy elementów
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, quadIndicesID);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), &quadIndices[0], GL_STATIC_DRAW);

	//wyznaczanie ścieżki do tekstur	
	std::string mesh_path = mesh_filename.substr(0, mesh_filename.find_last_of("/")+1);

	//ładowanie modelu obj
	vector<unsigned short> indices2;
	vector<glm::vec3> vertices2;
	if(!obj.Load(mesh_filename.c_str(), meshes, vertices, indices, materials, aabb, vertices2, indices2)) {
		cout<<"Nie mogę wczytać siatki OBJ"<<endl;
		exit(EXIT_FAILURE);
	}
	
	GL_CHECK_ERRORS

	int total =0;
	//sprawdzanie liczby niepustych tekstur, bo ta informacja
	//będzie potrzebna do utworzenia tablicy dla wszystkich tekstur 
	for(size_t k=0;k<materials.size();k++) {
		if(materials[k]->map_Kd != "") {
			total++;
		}
	}

	//ładowanie tekstur materiału
	for(size_t k=0;k<materials.size();k++) {
		//jeśli nazwa tekstury rozproszenia nie jest pusta
		if(materials[k]->map_Kd != "") {
			if(k==0) {
				//wygeneruj nową teksturę OpenGL
				glGenTextures(1, &textureID);
				glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP);
			}
			int texture_width = 0, texture_height = 0, channels=0;

			const string& filename =  materials[k]->map_Kd;

			std::string full_filename = mesh_path;
			full_filename.append(filename);

			//ładowanie tekstury przy użyciu biblioteki SOIL
			GLubyte* pData = SOIL_load_image(full_filename.c_str(), &texture_width, &texture_height, &channels, SOIL_LOAD_AUTO);
			if(pData == NULL) {
				cerr<<"Nie moge wczytac obrazu: "<<full_filename.c_str()<<endl;
				exit(EXIT_FAILURE);
			}

			//odwracanie obrazu w osi Y
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
			//określanie formatu obrazu
			GLenum format = GL_RGBA;
			switch(channels) {
				case 2: format = GL_RG32UI; break;
				case 3: format = GL_RGB; break;
				case 4: format = GL_RGBA; break;
			}

			//jeśli to jest pierwsza tekstura, alokuj tablicę tekstur
			if(k==0) {
				glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, texture_width, texture_height, total, 0, format, GL_UNSIGNED_BYTE, NULL);
			}
			//modyfikacja istniejącej tekstury
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,0,0,k, texture_width, texture_height, 1, format, GL_UNSIGNED_BYTE, pData);

			//zwalnianie zasobów biblioteki SOIL
			SOIL_free_image_data(pData);
		}
	}
	GL_CHECK_ERRORS

	//wczytywanie shadera płaszczyzny
	flatShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/flat.vert");
	flatShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/flat.frag");
	//kompilacja i konsolidacja programu shaderowego
	flatShader.CreateAndLinkProgram();
	flatShader.Use();
		//dodawanie atrybutów i uniformów
		flatShader.AddAttribute("vVertex");
		flatShader.AddUniform("MVP");
	flatShader.UnUse();

	//wczytanie shadera śledzenia promieni
	raytraceShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/raytracer.vert");
	raytraceShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/raytracer.frag");
	//kompilacja i konsolidacja programu shaderowego
	raytraceShader.CreateAndLinkProgram();
	raytraceShader.Use();
		//dodawanie atrybutów i uniformów
		raytraceShader.AddAttribute("vVertex");
		raytraceShader.AddUniform("eyePos");
		raytraceShader.AddUniform("invMVP");
		raytraceShader.AddUniform("light_position");
		raytraceShader.AddUniform("backgroundColor");
		raytraceShader.AddUniform("aabb.min");
		raytraceShader.AddUniform("aabb.max");
		raytraceShader.AddUniform("vertex_positions");
		raytraceShader.AddUniform("triangles_list");
		raytraceShader.AddUniform("VERTEX_TEXTURE_SIZE");
		raytraceShader.AddUniform("TRIANGLE_TEXTURE_SIZE");

		//ustalanie wartości stałych uniformów		
		glUniform1f(raytraceShader("VERTEX_TEXTURE_SIZE"), (float)vertices2.size());
		glUniform1f(raytraceShader("TRIANGLE_TEXTURE_SIZE"), (float)indices2.size()/4);
		glUniform3fv(raytraceShader("aabb.min"),1, glm::value_ptr(aabb.min));
		glUniform3fv(raytraceShader("aabb.max"),1, glm::value_ptr(aabb.max));
		glUniform4fv(raytraceShader("backgroundColor"),1, glm::value_ptr(bg));
		glUniform1i(raytraceShader("vertex_positions"), 1);
		glUniform1i(raytraceShader("triangles_list"), 2);
	raytraceShader.UnUse();

	GL_CHECK_ERRORS

	//wczytywanie shadera renderującego siatkę
	shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/shader.frag");
	//kompilacja i konsolidacja programu shaderowego
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
		shader.AddUniform("textureIndex");
		shader.AddUniform("useDefault");
		shader.AddUniform("diffuse_color");
		shader.AddUniform("light_position");
		//ustalanie wartości stałych uniformów	
		glUniform1i(shader("textureMap"), 0);
	shader.UnUse();

	GL_CHECK_ERRORS
		
	//ustawianie obiektu tablicy wierzchołków i obiektów bufora dla siatki
	//obsługa geometrii 
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);

	glBindVertexArray(vaoID);
		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		//przekazanie wierzchołków siatki
		glBufferData (GL_ARRAY_BUFFER, sizeof(Vertex)*vertices.size(), &(vertices[0].pos.x), GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//włączenie tablicy atrybutów wierzchołka dla położenia
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);
		GL_CHECK_ERRORS
		
		//włączenie tablicy atrybutów wierzchołka dla normalnej
		glEnableVertexAttribArray(shader["vNormal"]);
		glVertexAttribPointer(shader["vNormal"], 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, normal)) );

		GL_CHECK_ERRORS
		//włączenie tablicy atrybutów wierzchołka dla współrzędnych tekstury
		glEnableVertexAttribArray(shader["vUV"]);
		glVertexAttribPointer(shader["vUV"], 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, uv)) );

		GL_CHECK_ERRORS

		//jeśli materiał jest jeden, to znaczy, że model zawiera jedną siatkę
		//więc ładujemy go bufora tablicy elementów
		if(materials.size()==1) {
			//przekazanie indeksów do bufora tablicy elementów			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*indices.size(), &(indices[0]), GL_STATIC_DRAW);
		}
		GL_CHECK_ERRORS

	glBindVertexArray(0);

	//vao i vbo dla położenia gizma światła
	glm::vec3 crossHairVertices[6];
	crossHairVertices[0] = glm::vec3(-0.5f,0,0);
	crossHairVertices[1] = glm::vec3(0.5f,0,0);
	crossHairVertices[2] = glm::vec3(0, -0.5f,0);
	crossHairVertices[3] = glm::vec3(0, 0.5f,0);
	crossHairVertices[4] = glm::vec3(0,0, -0.5f);
	crossHairVertices[5] = glm::vec3(0,0, 0.5f);
	
	//identyfikatory tablicy wierzchołków i obiektów bufora dla gizma światła
	glGenVertexArrays(1, &lightVAOID);
	glGenBuffers(1, &lightVerticesVBO);
	glBindVertexArray(lightVAOID);

		glBindBuffer (GL_ARRAY_BUFFER, lightVerticesVBO);
		//przekazanie wierzchołków gizma światła do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, sizeof(crossHairVertices), &(crossHairVertices[0].x), GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//włączenie tablicy atrybutów wierzchołka dla położenia
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,0);

	GL_CHECK_ERRORS

	//wyznaczanie położenia światła na podstawie współrzędnych sferycznych
	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	//przekazanie położenia wierzchołka do tekstury związanej z jednostką 0
	glGenTextures(1, &texVerticesID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture( GL_TEXTURE_2D, texVerticesID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	GLfloat* pData = new GLfloat[vertices2.size()*4];
	int count = 0;
	for(size_t i=0;i<vertices2.size();i++) {
		pData[count++] = vertices2[i].x;
		pData[count++] = vertices2[i].y;
		pData[count++] = vertices2[i].z;
		pData[count++] = 0;				
	}
	//alokowanie tekstury o formacie zmiennoprzecinkowym
 	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, vertices2.size(),1, 0, GL_RGBA, GL_FLOAT, pData);

	//usuwanie wskaźnika danych
	delete [] pData;

	GL_CHECK_ERRORS

	//zapisywanie topologii siatki w innej teksturze związanej z jednostką teksturową 2
	glGenTextures(1, &texTrianglesID);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture( GL_TEXTURE_2D, texTrianglesID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	GLushort* pData2 = new GLushort[indices2.size()];
	count = 0;
	for(size_t i=0;i<indices2.size();i+=4) {
		pData2[count++] = (indices2[i]);
		pData2[count++] = (indices2[i+1]);
		pData2[count++] = (indices2[i+2]);
		pData2[count++] = (indices2[i+3]);
	}
	//alokowanie tekstury o formacie całkowitym
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16I, indices2.size()/4,1, 0, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, pData2);
	
	//usuwanie bufora danych
	delete [] pData2;

	GL_CHECK_ERRORS

	//ustawienie jednostki teksturującej nr 0 jako aktywnej 
	glActiveTexture(GL_TEXTURE0);

	//włączenie testowania głębi i zasłaniania
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//ustawienie koloru tła
	glClearColor(bg.x, bg.y, bg.z, bg.w);

	cout<<"Inicjalizacja powiodla sie"<<endl;

	//czas początkowy
	lastTime = (float)glutGet(GLUT_ELAPSED_TIME);
}

//renderowanie pełnoekranowego czworokąta z użyciem obiektu tablicy wierzchołków 
void DrawFullScreenQuad() {
	//wiązanie obiektu tablicy wierzchołków czworokąta
	glBindVertexArray(quadVAOID);
	//rysowanie 2 trójkątów
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {

	glDeleteVertexArrays(1, &quadVAOID);
	glDeleteBuffers(1, &quadVBOID);
	glDeleteBuffers(1, &quadIndicesID);

	//usuwanie tekstur
	glDeleteTextures(1, &textureID);

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

	//likwidacja programu shaderowego
	shader.DeleteShaderProgram();
	raytraceShader.DeleteShaderProgram();
	flatShader.DeleteShaderProgram();

	//likwidacja vao i vbo
	glDeleteBuffers(1, &vboVerticesID);
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID);

	glDeleteVertexArrays(1, &lightVAOID);
	glDeleteBuffers(1, &lightVerticesVBO);

	glDeleteTextures(1, &texVerticesID);
	glDeleteTextures(1, &texTrianglesID);
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

	//wyznaczanie liczby klatek na sekundę (FPS)
	++total_frames;
	float current = (float)glutGet(GLUT_ELAPSED_TIME);
	if((current-lastTime)>1000) {
		fps = 1000.0f*total_frames/(current-lastTime);
		std::cout<<"FPS: "<<fps<<std::endl;
		lastTime= current;
		total_frames = 0;
	}
	//czyszczenie buforów koloru i głębi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//transformacje kamery
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//wyznaczanie położenia wierzchołka i odwrotności macierzy MVP
	glm::mat4 invMV  = glm::inverse(MV);
	glm::vec3 eyePos = glm::vec3(invMV[3][0],invMV[3][1],invMV[3][2]);
	glm::mat4 invMVP = glm::inverse(P*MV);
	
	//jeśli włączone jest śledzenie promieni
	if(bRaytrace) {
		//włączenie shadera rasteryzacji
		raytraceShader.Use();
			//ustawianie uniformów shadera
			glUniform3fv(raytraceShader("eyePos"), 1, glm::value_ptr(eyePos));
			glUniformMatrix4fv(raytraceShader("invMVP"), 1, GL_FALSE, glm::value_ptr(invMVP));
			glUniform3fv(raytraceShader("light_position"),1, &(lightPosOS.x));
				//rysowanie pełnoekranowego czworokąta
				DrawFullScreenQuad();
		//odwiązanie shadera rasteryzacji
		raytraceShader.UnUse();
	} else {
		//rasteryzacja
		//wiązanie obiektu tablicy wierzchołków siatki
		glBindVertexArray(vaoID); {
			//włączenie shadera renderującego siatkę
			shader.Use();
				//ustawianie uniformów shadera
				glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));
				glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
				glUniformMatrix4fv(shader("P"), 1, GL_FALSE, glm::value_ptr(P));
				glUniform3fv(shader("light_position"),1, &(lightPosOS.x));

				//pętla po wszystkich materiałach
				for(size_t i=0;i<materials.size();i++) {
					Material* pMat = materials[i];

					//jeśli nazwa pliku tekstury nie jest pusta
					//nie używaj koloru domyślnego
					if(pMat->map_Kd !="") {
						glUniform1f(shader("useDefault"), 0.0);
						glUniform1i(shader("textureIndex"), i);
					}
					else
						//a jeśli nie ma tekstury, użyj koloru domyślnego
						glUniform1f(shader("useDefault"), 1.0);
			
					//jeśli materiał jest jeden, renderujemy całą siatkę siatkę naraz
					if(materials.size()==1)
						glDrawElements(GL_TRIANGLES,  indices.size() , GL_UNSIGNED_SHORT, 0);
					else
						//w przeciwnym razie renderuj siatkę składową
						glDrawElements(GL_TRIANGLES, pMat->count, GL_UNSIGNED_SHORT, (const GLvoid*)(&indices[pMat->offset]));
					
				}

			//odwiązanie shadera
			shader.UnUse();
		}
	}

	//wyłączenie testowania głębi
	glDisable(GL_DEPTH_TEST);

	//rysowanie gizma światła, ustawianie tablicy wierzchołków światła
	glBindVertexArray(lightVAOID); {
		//transformacje gizma światła
		glm::mat4 T = glm::translate(glm::mat4(1), lightPosOS);
		//wiązanie shadera
		flatShader.Use();
			//ustawianie uniformów shadera i rysowanie linii
			glUniformMatrix4fv(flatShader("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV*T));
				glDrawArrays(GL_LINES, 0, 6);
		//odwiązanie shadera
		flatShader.UnUse();
	}
	//włączenie testowania głębi
	glEnable(GL_DEPTH_TEST);

	//zamiana buforów w celu wyświetlenia wyrenderowanego obrazu
	glutSwapBuffers();
}

//obsługa rolki do przewijania w celu przesunięcia źródła światła
void OnMouseWheel(int button, int dir, int x, int y) {

	if (dir > 0)
    {
        radius += 0.1f;
    }
    else
    {
        radius -= 0.1f;
    }
		
	//aktualizowanie położenia światła 
	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	//wywołanie funkcji wyświetlającej
	glutPostRedisplay();
}

//obsługa klawiatury w celu przełączania śledzenia promieni i rasteryzacji
void OnKey(unsigned char k, int x, int y) {
	switch(k) {
		case ' ':bRaytrace=!bRaytrace; break;
	}
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	//inicjalizacja freeglut 
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Śledzenie promieni na GPU - OpenGL 3.3");

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

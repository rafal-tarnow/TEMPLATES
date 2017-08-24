#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "..\src\GLSLShader.h"
#include "3ds.h"

#include <SOIL.h>

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace std;

//wymiary okna
const int WIDTH  = 1280;
const int HEIGHT = 960;

//shadery używane w tej recepturze
//jeden do cieniowania modeli i drugi do renderowania gizma światła
GLSLShader shader, flatShader;

//obiekt klasy C3dsLoader
C3dsLoader loader;

//identyfikatory tablicy wierzchołków i obiektów bufora
//każdy wczytany atrybut będzie umieszczany w oddzielnym buforze
GLuint vaoID;			//obiekt tablicy wierzchołków siatki
GLuint vboVerticesID;	//obiekt bufora wierzchołków siatki
GLuint vboUVsID;		//obiekt bufora współrzędnych tekstury siatki
GLuint vboNormalsID;	//obiekt bufora normalnych siatki
GLuint vboIndicesID;	//obiekt bufora tablicy z indeksami elementów siatki 

//macierze modelu, widoku i rzutowania
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

vector<C3dsMesh*> meshes;				//wektor siatek w pliku 3ds
vector<Material*> materials;			//wektor materiałów
map<std::string, GLuint> textureMaps;	//nazwa pliku z mapą tekstury oraz identyfikator tekstury
typedef map<std::string, GLuint>::iterator iter;	//iterator mapy tekstury
vector<glm::vec3> vertices;		//wierzchołki siatki
vector<glm::vec3> normals;		//normalne siatki
vector<glm::vec2> uvs;			//współrzędne tekstury siatki
vector<Face> faces;				//ścianki (trójkąty) siatki
vector<unsigned short> indices;	//indeksy siatki

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=-68, rY=33, dist = -2;
 
//współrzędne sferyczne dla obrotów źródła światła
float theta = 2.0f;
float phi = 2.0f;
float radius = 70;

//identyfikatory tablicy wierzchołków i obiektu bufora dla gizma światła
GLuint lightVAOID;
GLuint lightVerticesVBO; 
glm::vec3 lightPosOS=glm::vec3(0, 2,0); //położenie światła w przestrzeni obiektu

//nazwa pliku 3ds z siatką do wczytania
const std::string mesh_filename = "../media/blocks.3ds";  
 
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
	
		//aktualizacja położenia źródła światła
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
	//pobieranie ścieżki do siatki	 
	std::string mesh_path = mesh_filename.substr(0, mesh_filename.find_last_of("/")+1);
	 
	//wczytanie pliku 3ds
	if(!loader.Load3DS(mesh_filename.c_str( ),  meshes, vertices, normals, uvs, faces, indices, materials)) {
		cout<<"Nie moge wczytac siatki 3ds"<<endl;
		exit(EXIT_FAILURE);
	} 
	GL_CHECK_ERRORS

	//wczytywanie tekstur materiałowych
	//pętla po wszystkich materiałach
	for(size_t k=0;k<materials.size();k++) {
		//pętla po wszystkich materiałowych mapach tekstur
		for(size_t m=0;m< materials[k]->textureMaps.size();m++) {
			GLuint id = 0;
			//generowanie tekstury w OPenGL i ustawianie jej parametrów
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			int texture_width = 0, texture_height = 0, channels=0;		 	
			
			const string& filename =  materials[k]->textureMaps[m]->filename;
			std::string full_filename = mesh_path;
			full_filename.append(filename);

			//wczytywanie obrazu przy użyciu biblioteki SOIL
			GLubyte* pData = SOIL_load_image(full_filename.c_str(), &texture_width, &texture_height, &channels, SOIL_LOAD_AUTO);
			if(pData == NULL) {
				cerr<<"Niemoge wczytac obrazu: "<<full_filename.c_str()<<endl;
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
			//ustalanie formatu obrazu
			GLenum format = GL_RGBA;
			switch(channels) {
				case 2: format = GL_RG32UI; break;
				case 3: format = GL_RGB; break;
				case 4: format = GL_RGBA; break;
			}
			//alokowanie tekstury
			glTexImage2D(GL_TEXTURE_2D, 0, format, texture_width, texture_height, 0, format, GL_UNSIGNED_BYTE, pData);

			//zwalnianie zasobów zajętych przez bibliotekę SOIL
			SOIL_free_image_data(pData);

			//zapisanie identyfikatora tekstury
			textureMaps[filename]=id;
		}
	}

	GL_CHECK_ERRORS

	//wczytanie shaderów flat 
	flatShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/flat.vert");
	flatShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/flat.frag");
	//kompilacja i konsolidacja
	flatShader.CreateAndLinkProgram();
	flatShader.Use();	
		//dodawanie atrybutu i uniformu
		flatShader.AddAttribute("vVertex");
		flatShader.AddUniform("MVP"); 
	flatShader.UnUse();

	//wczytanie shaderów renderujących siatkę
	shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/shader.frag");
	//kompilacja i konsolidacja
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
		shader.AddUniform("hasTexture");	
		shader.AddUniform("light_position"); 
		shader.AddUniform("diffuse_color"); 
		//ustawianie wartości stałych uniformów		
		glUniform1i(shader("textureMap"), 0);
	shader.UnUse();

	GL_CHECK_ERRORS

	//ustawianie obiektu tablicy i obiektów bufora wierzchołków dla siatki
	//obsługa geometrii 
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboUVsID);
	glGenBuffers(1, &vboNormalsID);
	glGenBuffers(1, &vboIndicesID); 

	//prostpoadłościan otaczający siatkę
	glm::vec3 min=glm::vec3(1000.0f), max=glm::vec3(-1000);
	for(size_t j=0;j<meshes.size();j++) {
		C3dsMesh* pMesh = meshes[j];
		for(size_t i=0;i<pMesh->vertices.size();i++) {
			min = glm::min(pMesh->vertices[i], min);
			max = glm::max(pMesh->vertices[i], max);
		}
	}
	//ustawienie kamery w taki sposób, aby cała siatka 
	//była widoczna na ekranie
	glm::vec3 center = (max+min)/2.0f;
	float r = std::max(glm::distance(center,max), glm::distance(center,min));
	dist = -(r+(r*.5f));
	 
	//wiązanie obiektu tablicy wierzchołków siatki
	glBindVertexArray(vaoID); 
	glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
	//przekazanie danych wierzchołkowych do obiektu bufora
	glBufferData (GL_ARRAY_BUFFER, sizeof(glm::vec3)*vertices.size(), &(vertices[0].x), GL_STATIC_DRAW);
		
	GL_CHECK_ERRORS
	//włączenie tablicy atrybutu wierachołka dla położeñ wierzchołków		
	glEnableVertexAttribArray(shader["vVertex"]);
	glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);
		
	GL_CHECK_ERRORS

	//wiązanie obiektu bufora współrzędnych tekstury i przekazanie tych współrzędnych do pobiektu bufora
	glBindBuffer (GL_ARRAY_BUFFER, vboUVsID);
	glBufferData (GL_ARRAY_BUFFER, sizeof(glm::vec2)*uvs.size(), &(uvs[0].x), GL_STATIC_DRAW);

	GL_CHECK_ERRORS
	//włączenie tablicy atrybutu wierachołka dla współrzędnych tekstury
	glEnableVertexAttribArray(shader["vUV"]);
	glVertexAttribPointer(shader["vUV"], 2, GL_FLOAT, GL_FALSE, 0, 0);

	GL_CHECK_ERRORS

	//wiązanie obiektu bufora normalnych i przekazanie tych normalnych do pobiektu bufora
	glBindBuffer (GL_ARRAY_BUFFER, vboNormalsID);
	glBufferData (GL_ARRAY_BUFFER, sizeof(glm::vec3)*normals.size(), &(normals[0].x), GL_STATIC_DRAW);

	GL_CHECK_ERRORS
	//włączenie tablicy atrybutu wierachołka dla normalnych
	glEnableVertexAttribArray(shader["vNormal"]);
	glVertexAttribPointer(shader["vNormal"], 3, GL_FLOAT, GL_FALSE, 0, 0);

	GL_CHECK_ERRORS

	//jeśli materiał jest jeden, to znaczy, że model 3ds zawiera jedną siatkę
	//więc ładujemy go bufora tablicy elementów
	if(materials.size()==1) {
		//przekazanie indeksów do bufora tablicy elementów			
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*3*faces.size(), 0, GL_STATIC_DRAW);

		//wypełnienie bufora tablicy elementów tablicą indeksów
		GLushort* pIndices = static_cast<GLushort*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
		for(size_t i=0;i<faces.size();i++) {
			*(pIndices++)=faces[i].a;
			*(pIndices++)=faces[i].b;
			*(pIndices++)=faces[i].c;
		}
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	}  
		
	GL_CHECK_ERRORS
		  
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
	//przekazanie wierzchołków gizma światła do obiektu bufora
	glBindBuffer (GL_ARRAY_BUFFER, lightVerticesVBO);
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
			
	//włączenie testowania głębi i zasłaniania
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE); 

	//ustawienie niebieskofioletowego koloru
	glClearColor(0.5,0.5,1,1);
	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {
	//usuwanie tekstur 

	for(iter i = textureMaps.begin();i!=textureMaps.end();i++) {
		glDeleteTextures(1, &(i->second));
	}
	textureMaps.clear();

	size_t total_meshes = meshes.size();
	for(size_t i=0;i<total_meshes;i++) {
		delete meshes[i];
		meshes[i]=0;
	}
	meshes.clear();

	size_t total = materials.size();
	//usuwanie wszystkich map teksturowych z materiału
	for(size_t i=0;i<total;i++) {
		if(materials[i]!=0) {
			materials[i]->face_ids.clear();
			for(size_t j=0;j<materials[i]->textureMaps.size();j++) {
				if(materials[i]->textureMaps[j]!=0) {
					delete materials[i]->textureMaps[j];
					materials[i]->textureMaps[j] = 0;
				}
			}
			materials[i]->textureMaps.clear();
			materials[i]->face_ids.clear(); 
			delete materials[i];
			materials[i]=0;
		}
	}

	//likwidacja programu shaderowego
	shader.DeleteShaderProgram();
	flatShader.DeleteShaderProgram();

	//likwidacja vao i vbo
	glDeleteBuffers(1, &vboVerticesID);
	glDeleteBuffers(1, &vboUVsID);
	glDeleteBuffers(1, &vboNormalsID);
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID); 

	glDeleteVertexArrays(1, &lightVAOID);
	glDeleteBuffers(1, &lightVerticesVBO);
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
	//czyszczenie buforów koloru i głębi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//transformacje kamery
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 0.0f,1.0f));  

	GL_CHECK_ERRORS

	//wiązanie obiektu tablicy wierzchołków siatki
	glBindVertexArray(vaoID); {
		//włączenie shadera renderującego siatkę
		shader.Use();
			//ustawianie uniformów shadera
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));		
			glUniformMatrix4fv(shader("P"), 1, GL_FALSE, glm::value_ptr(P));
			glUniform3fv(shader("light_position"),1, &(lightPosOS.x)); 
		 
			//jeśli materiał jest jeden, renderujemy całą siatkę siatkę naraz					
			if(materials.size()==1) {
				GLint whichID[1];
				glGetIntegerv(GL_TEXTURE_BINDING_2D, whichID);
				//jeśli materiał zawiera mapy tekstury i nie jest jeszcze związany,
				//należy go związać i zmiennej hasTexture przypisać wartość 1
				if(textureMaps.size()>0) {
					if(whichID[0] != textureMaps[materials[0]->textureMaps[0]->filename]) {
						glBindTexture(GL_TEXTURE_2D, textureMaps[materials[0]->textureMaps[0]->filename]);
						glUniform1f(shader("hasTexture"),1.0);
					}
				} else {
					//w przeciwnym razie zmienną hasTexture należy ustawić na 0 i zastosować kolor rozproszenia 
					glUniform1f(shader("hasTexture"),0.0);
					glUniform3fv(shader("diffuse_color"),1, materials[0]->diffuse);	
				}
				//rysowanie trójkątów całej siatki
				glDrawElements(GL_TRIANGLES, meshes[0]->faces.size()*3, GL_UNSIGNED_SHORT, 0); 
			}  else {
				//albo podsiatek z poszczególnymi materiałami
				for(size_t i=0;i<materials.size();i++) {
					GLint whichID[1];
					glGetIntegerv(GL_TEXTURE_BINDING_2D, whichID);
					//jeśli materiał zawiera mapy tekstury i nie jest jeszcze związany,
					//należy go związać i zmiennej hasTexture przypisać wartość 1
					if(materials[i]->textureMaps.size()>0) {
						if(whichID[0] != textureMaps[materials[i]->textureMaps[0]->filename]) {
							glBindTexture(GL_TEXTURE_2D, textureMaps[materials[i]->textureMaps[0]->filename]);
						}
						glUniform1f(shader("hasTexture"),1.0);
					} else {
						//w przeciwnym razie zmienną hasTexture należy ustawić na 0 					
						glUniform1f(shader("hasTexture"),0.0);
					}
					//przekazanie koloru rozproszenia z uniformu do materiału
					glUniform3fv(shader("diffuse_color"),1, materials[i]->diffuse);	
					//rysowanie trójkątów zgodnie z indeksami podsiatek 
					glDrawElements(GL_TRIANGLES, materials[i]->sub_indices.size(), GL_UNSIGNED_SHORT, &(materials[i]->sub_indices[0])); 
			
				}
			}
		//odwiązanie shadera
		shader.UnUse(); 
	}
	
	//wyłączenie testowania głębi
	glDisable(GL_DEPTH_TEST);

	//rysowanie gizma światła
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

int main(int argc, char** argv) {
	//inicjalizacja freeglut 
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);	
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Przeglądarka 3ds - OpenGL 3.3");

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
	err = glGetError(); 	//w celu pominięcia błędu 1282 INVALID ENUM 
	GL_CHECK_ERRORS

	//wyprowadzanie informacji sprzętowych
	cout<<"\tWersja GLEW "<<glewGetString(GLEW_VERSION)<<endl;
	cout<<"\tProducent: "<<glGetString (GL_VENDOR)<<endl;
	cout<<"\tRenderer: "<<glGetString (GL_RENDERER)<<endl;
	cout<<"\tVersion: "<<glGetString (GL_VERSION)<<endl;
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


#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "..\src\GLSLShader.h"
#include <vector>
#include "Ezm.h"

#include <SOIL.h>

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace std;

//wymiary okna
const int WIDTH  = 1280;
const int HEIGHT = 960;

//shadery do rysowania siatki i gizma światła
GLSLShader shader, flatShader;

//ID tablicy wierzchołków i obiektów bufora
GLuint vaoID;
GLuint vboVerticesID; 
GLuint vboIndicesID;

//macierze rzutowania oraz modelu i widoku
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//położenie źródła światła 
glm::vec3 center;

//instancja obiektu EzmLoader
EzmLoader ezm;
vector<SubMesh> submeshes;	//wszystkie siatki składowe w pliku EZMesh
std::map<std::string, GLuint> materialMap; //nazwa materiału, mapa identyfikatorów tekstur
std::map<std::string, std::string> material2ImageMap; //nazwa materiału, mapa nazw obrazów
typedef std::map<std::string, std::string>::iterator iter;	//iterator materiał-mapa

//wierzchołki i indeksy siatki
vector<Vertex> vertices;   
vector<unsigned short> indices;

//wszystkie materiały z pliku EZMesh w porządku liniowym
vector<std::string> materialNames;

//identyfikatory tablicy wierzchołków i obiektów bufora dla gizma światła
GLuint lightVAOID;
GLuint lightVerticesVBO; 

//położenie światła w przestrzenie obiektu
glm::vec3 lightPosOS=glm::vec3(0, 2,0); //położenie światła w przestrzenie obiektu 

//współrzędne sferyczne dla obrotów światła
float theta = 1.1f;
float phi = 0.85f;
float radius = 70;

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=0, rY=0, dist = -120;

const std::string mesh_filename = "../media/dudeMesh.ezm";

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
	
		//wyznaczanie nowego położenia światła w zależności od zmodyfikowanych kątów theta i phi
		lightPosOS.x = center.x + radius * cos(theta)*sin(phi);
		lightPosOS.y = center.y + radius * cos(phi);
		lightPosOS.z = center.z + radius * sin(theta)*sin(phi);

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
 
	//wydobywanie ścieżki do siatki 
	std::string mesh_path = mesh_filename.substr(0, mesh_filename.find_last_of("//")+1);

	glm::vec3 min, max;
	//wczytywanie pliku EZMesh
	if(!ezm.Load(mesh_filename.c_str(), submeshes, vertices, indices, material2ImageMap, min, max)) { 
		cout<<"Nie mogę wczytać siatki OBJ"<<endl;
		exit(EXIT_FAILURE);
	} 
	GL_CHECK_ERRORS

	//umieszczanie wczytanych nazw materiałów w wektorze
	for(iter i = material2ImageMap.begin();i!=material2ImageMap.end();++i) {
		materialNames.push_back(i->second);
	}

	//obliczanie odległości, na jaką trzeba przesunąć kamerę, aby dobrze widziała wczytany model	
	center = (max + min) * 0.5f; 
	glm::vec3 diagonal = (max-min);
	radius = glm::length(center- diagonal * 0.5f);
	dist = -glm::length(diagonal);
	 	 
	//generowanie tekstur z nazwami wczytanych materiałów  
	for(size_t k=0;k<materialNames.size();k++) { 
		if(materialNames[k].length()==0)
			continue;

		//wygeneruj nową teksturę OpenGL
		GLuint id = 0;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		int texture_width = 0, texture_height = 0, channels=0;	

		//wyznaczanie pełnej nazwy obrazu
		const string& filename =  materialNames[k];
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
		//alokowanie tekstury 
		glTexImage2D(GL_TEXTURE_2D, 0, format, texture_width, texture_height, 0, format, GL_UNSIGNED_BYTE, pData);

		//zwalnianie zasobów biblioteki SOIL
		SOIL_free_image_data(pData);

		//dodawanie identyfikatora tekstury do mapy materiałów Wywołanie nazwy tekstury 
		//da nam jej OpeGL-owy identyfikator
		materialMap[filename] = id ;
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
		shader.AddUniform("useDefault");		
		shader.AddUniform("light_position");
		shader.AddUniform("diffuse_color");
		//ustalanie wartości stałych uniformów
		glUniform1i(shader("textureMap"), 0);
	shader.UnUse();

	GL_CHECK_ERRORS

 
	//ustawianie geometrii  
	//vao i vbo 
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID); 
	 
	glBindVertexArray(vaoID);	
		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		//przekazanie wierzchołków do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, sizeof(Vertex)*vertices.size(), &(vertices[0].pos.x), GL_DYNAMIC_DRAW);
		GL_CHECK_ERRORS
		//włączenie tablicy atrybutów wierzchołka dla położenia
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);
		GL_CHECK_ERRORS 
		//włączenie tablicy atrybutów wierzchołka dla położenia
		glEnableVertexAttribArray(shader["vNormal"]);
		glVertexAttribPointer(shader["vNormal"], 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, normal)) );
 
		GL_CHECK_ERRORS
		//włączenie tablicy atrybutów wierzchołka dla współrzędnych tekstury 
		glEnableVertexAttribArray(shader["vUV"]);
		glVertexAttribPointer(shader["vUV"], 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, uv)) );

		GL_CHECK_ERRORS
 
		//glBindVertexArray(0); 

	//vao i vbo dla położenia gizma światła
	glm::vec3 crossHairVertices[6];
	crossHairVertices[0] = glm::vec3(-0.5f,0,0);
	crossHairVertices[1] = glm::vec3(0.5f,0,0);
	crossHairVertices[2] = glm::vec3(0, -0.5f,0);
	crossHairVertices[3] = glm::vec3(0, 0.5f,0);
	crossHairVertices[4] = glm::vec3(0,0, -0.5f);
	crossHairVertices[5] = glm::vec3(0,0, 0.5f);

	//generowanie tablicy wierzchołków i obiektów bufora dla światła
	glGenVertexArrays(1, &lightVAOID);
	glGenBuffers(1, &lightVerticesVBO);   
	glBindVertexArray(lightVAOID);	
		glBindBuffer (GL_ARRAY_BUFFER, lightVerticesVBO);
		glBufferData (GL_ARRAY_BUFFER, sizeof(crossHairVertices), &(crossHairVertices[0].x), GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//enable vertex attribute array
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,0);
	 
		GL_CHECK_ERRORS 
	
	//get the light position using the center and the spherical coordinates	
	lightPosOS.x = center.x + radius * cos(theta)*sin(phi);
	lightPosOS.y = center.y + radius * cos(phi);
	lightPosOS.z = center.z + radius * sin(theta)*sin(phi);

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
	size_t total_textures = materialMap.size();
	for(size_t i=0;i<total_textures;i++) {
		glDeleteTextures(1, &materialMap[materialNames[i]]);
	}
	materialNames.clear();
	materialMap.clear(); 
	submeshes.clear(); 
	vertices.clear();
	indices.clear();
	 
	//likwidacja programu shaderowego
	shader.DeleteShaderProgram();
	flatShader.DeleteShaderProgram();

	//likwidacja vao i vbo
	glDeleteBuffers(1, &vboVerticesID); 
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID); 

	glDeleteVertexArrays(1, &lightVAOID); 
	glDeleteVertexArrays(1, &lightVerticesVBO); 

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
	//czyszczenie buforów koloru i głębi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//transformacje kamery
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(-center.x, -center.y, -center.z+dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));  
 
	//wiązanie obiektu tablicy wierzchołków siatki
	glBindVertexArray(vaoID); {
		//wiązanie shadera siatki
		shader.Use();
			//ustawianie uniformów shadera
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));	
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));	
			glUniformMatrix4fv(shader("P"), 1, GL_FALSE, glm::value_ptr(P));	
			glUniform3fv(shader("light_position"),1, &(lightPosOS.x)); 

			//dla wszystkich siatek składowych
			for(size_t i=0;i<submeshes.size();i++) {
				//jeśli nazwa materiału nie jest pusta
				if(strlen(submeshes[i].materialName)>0) {
					//wyznaczanie OpenGL-owych identyfikatorów tekstur na podstawie mapy tekstur i nazw materiałów
					GLuint id = materialMap[material2ImageMap[submeshes[i].materialName]];
					GLint whichID[1];
					glGetIntegerv(GL_TEXTURE_BINDING_2D, whichID);
					//jeśli identyfikator aktualnie związanej tekstury różni się od identyfikatora tekstury bieżącej
					//zwiąż teksturę bieżącą
					if(whichID[0] != id)
						glBindTexture(GL_TEXTURE_2D, id);
					//informowanie shadera, że siatka ma teksturę i nie należy używać domyślnego koloru					
					glUniform1f(shader("useDefault"), 0.0);
				} else {  
					//nie ma tekstury, użyj koloru domyślnego
					glUniform1f(shader("useDefault"), 1.0);
				}
				//rysowanie trójkątów zgodnie z indeksami podsiatek
 				glDrawElements(GL_TRIANGLES, submeshes[i].indices.size(), GL_UNSIGNED_INT, &submeshes[i].indices[0]);
			} //koniec pętli for

		//odwiązanie shadera
		shader.UnUse(); 
	}
 
	//wyłączenie testowania głębi
	glDisable(GL_DEPTH_TEST);

	//wiązanie obiektu tablicy wierzchołków gizma światła
	glBindVertexArray(lightVAOID); {
		//transformacje gizma światła
		glm::mat4 T = glm::translate(glm::mat4(1), lightPosOS); 
		//ustawianie shadera gizma światła
		flatShader.Use();
			//przekazanie uniformów do shadera i renderowanie gizma światła jako odcinków
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

//obsługa rolki do przewijania w celu zmiany promienia źródła światła
//jako że położenie jest dane we współrzędnych sferycznych, promień określa 
//odległość źródła światła od środka układu współrzędnych
void OnMouseWheel(int button, int dir, int x, int y) {
			
	if (dir > 0)
    {
        radius += 0.1f;
    }
    else
    {
        radius -= 0.1f;
    }

	//wyznaczanie nowego położenia światła
	lightPosOS.x = center.x + radius * cos(theta)*sin(phi);
	lightPosOS.y = center.y + radius * cos(phi);
	lightPosOS.z = center.z + radius * sin(theta)*sin(phi);

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
	glutCreateWindow("Przeglądarka siatek szkieletowych EZMesh - OpenGL 3.3");

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

	//print information on screen
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

	//wywołanie pętli głównej
	glutMainLoop();	

	return 0;
}

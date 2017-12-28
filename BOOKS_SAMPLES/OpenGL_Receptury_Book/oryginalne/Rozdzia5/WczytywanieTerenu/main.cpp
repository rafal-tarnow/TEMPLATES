#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

#include <SOIL.h>

#include "..\src\GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace std;

//wymiary okna
const int WIDTH  = 1280;
const int HEIGHT = 960;

//shadery używane w recepturze
GLSLShader shader;

//ID tablicy wierzchołków i obiektów bufora
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

//ID mapy wysokości
GLuint heightMapTextureID;

//wymiary mapy wysokości i jej wymiary połówkowe
const int TERRAIN_WIDTH = 512;
const int TERRAIN_DEPTH = 512; 
const int TERRAIN_HALF_WIDTH = TERRAIN_WIDTH>>1;
const int TERRAIN_HALF_DEPTH = TERRAIN_DEPTH>>1;

//skala mapy wysokości i jej skala połówkowa
float scale = 50;
float half_scale = scale/2.0f;

//liczba wierzchołków i indeksów terenu
const int TOTAL = (TERRAIN_WIDTH*TERRAIN_DEPTH);
const int TOTAL_INDICES = TOTAL*2*3;

//nazwa pliku z mapą wysokości
const string filename = "../media/heightmap512x512.png";
 
//wierzchołki i indeksy mapy wysokości
glm::vec3 vertices[TOTAL];
GLuint indices[TOTAL_INDICES];

//macierze rzutowania oraz modelu i widoku
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=25, rY=-40, dist = -7;

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

	GL_CHECK_ERRORS
	//wczytanie shaderów 
	shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/shader.frag");
	//kompilacja i konsolidacja programu shaderowego
	shader.CreateAndLinkProgram();
	shader.Use();	
		//dodawanie atrybutów i uniformów
		shader.AddAttribute("vVertex"); 
		shader.AddUniform("heightMapTexture");
		shader.AddUniform("scale");
		shader.AddUniform("half_scale");
		shader.AddUniform("HALF_TERRAIN_SIZE");
		shader.AddUniform("MVP");
		//ustalanie wartości stałych uniformów	
		glUniform1i(shader("heightMapTexture"), 0);
		glUniform2i(shader("HALF_TERRAIN_SIZE"), TERRAIN_WIDTH>>1, TERRAIN_DEPTH>>1);
		glUniform1f(shader("scale"), scale);
		glUniform1f(shader("half_scale"), half_scale);
	shader.UnUse();

	GL_CHECK_ERRORS
		 
		
	//wypełnianie tablicy indeksów
	GLuint* id=&indices[0];
	int i=0, j=0;
	
	//ustawianie wierzchołków 
	int count = 0;
	//wierzchołki terenu
	for( j=0;j<TERRAIN_DEPTH;j++) {		 
		for( i=0;i<TERRAIN_WIDTH;i++) {	  
			vertices[count] = glm::vec3( (float(i)/(TERRAIN_WIDTH-1)), 
										 0, 
										 (float(j)/(TERRAIN_DEPTH-1)));
			count++;
		}
	}
	 
	//indeksy terenu
	for (i = 0; i < TERRAIN_DEPTH-1; i++) {        
		for (j = 0; j < TERRAIN_WIDTH-1; j++) {			
			int i0 = j+ i*TERRAIN_WIDTH;
			int i1 = i0+1;
			int i2 = i0+TERRAIN_WIDTH;
			int i3 = i2+1;
			*id++ = i0; 
			*id++ = i2; 
			*id++ = i1; 
			*id++ = i1; 
			*id++ = i2; 
			*id++ = i3; 
		}    
	}

	GL_CHECK_ERRORS

	//tablica wierzchołków terenu i obiekty buforów  
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);
	 
	glBindVertexArray(vaoID);	

		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		//przekazanie wierzchołków terenu do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//włączenie tablicy atrybutów wierzchołka dla położenia
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);
		GL_CHECK_ERRORS 
		//przekazanie tablicy indeksów terenu do bufora tablicy elementów
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
			
	//wczytanie mapy wysokości przy użyciu biblioteki SOIL	
	int texture_width = 0, texture_height = 0, channels=0;		 
	GLubyte* pData = SOIL_load_image(filename.c_str(), &texture_width, &texture_height, &channels, SOIL_LOAD_L);
	
	//przywrócenie teksturze właściwej orientacji w pionie 
	for( j = 0; j*2 < texture_height; ++j )
	{
		int index1 = j * texture_width ;
		int index2 = (texture_height - 1 - j) * texture_width ;
		for( i = texture_width ; i > 0; --i )
		{
			GLubyte temp = pData[index1];
			pData[index1] = pData[index2];
			pData[index2] = temp;
			++index1;
			++index2;
		}
	} 

	//ustawienie parametrów tekstury
	glGenTextures(1, &heightMapTextureID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightMapTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texture_width, texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, pData);
	
	//zwalnianie zasobów zaalokowanych przez bibliotekę SOIL
	SOIL_free_image_data(pData);
	
	GL_CHECK_ERRORS

	//włączenie trybu rysowania linii
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	GL_CHECK_ERRORS

	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich zasobów
void OnShutdown() {

	//likwidacja programu shaderowego
	shader.DeleteShaderProgram();

	//likwidacja vao i vbo
	glDeleteBuffers(1, &vboVerticesID);
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID);

	//usuwanie tekstur
	glDeleteTextures(1, &heightMapTextureID);
	cout<<"Shutdown successfull"<<endl;
}

//obsługa zmiany wymiarów okna
void OnResize(int w, int h) {
	//ustawienie wymiarów okna widokowego
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);

	//wyznaczanie macierzy rzutowania
	P = glm::perspective(45.0f, (GLfloat)w/h, 0.01f, 10000.f);
}

//renderowanie
void OnRender() {
	//czyszczenie buforów koloru i głębi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//przekształcenia kamery
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 Ry	= glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 MV	= Ry;
    glm::mat4 MVP	= P*MV;

	//ponieważ obiekt tablicy wierzchołków terenu jest ciągle związany 
	//z bieżącym kontekstem, możemy bezpośrednio wywołać rysowanie elementu 
	// w celu narysowania wierzchołków ze wspomnianego obiektu tablicy 
	//uruchamianie shadera terenu
	shader.Use();				
		//przekazanie uniformów
		glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
			//rysowanie siatki terenu
			glDrawElements(GL_TRIANGLES, TOTAL_INDICES, GL_UNSIGNED_INT, 0);
	//wyłączanie shadera
	shader.UnUse();
	
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
	glutCreateWindow("Prosty teren - OpenGL 3.3");
	
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

	//wywołanie pętli głównej
	glutMainLoop();	

	return 0;
}
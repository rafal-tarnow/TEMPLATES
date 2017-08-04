#define _USE_MATH_DEFINES
#include <cmath>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

#include "..\src\GLSLShader.h"

#include <SOIL.h>

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace std;

//wymiary okna
const int WIDTH  = 1280;
const int HEIGHT = 960;

//shader cząsteczkowy, shader teksturowy i wskaźnik do shadera bieżącego
GLSLShader shader, texturedShader, *pCurrentShader;

//ID tablicy wierzchołków i obiektów bufora
GLuint vaoID;
GLuint vboVerticesID; 

//całkowita liczba cząsteczek
const int MAX_PARTICLES = 10000;
 
//macierze rzutowania, modelu i widoku oraz transformacji emitera
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);
glm::mat4 emitterXForm = glm::mat4(1);

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=0, rY=0, dist = -10;
float time = 0;
 
//nazwa pliku z teksturą cząsteczki 
const std::string texture_filename = "../media/particle.dds";

//identyfikator tekstury OpenGL-owej
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

//inicjalizacja OpenGL
void OnInit() {
	GL_CHECK_ERRORS
	//wczytywanie shaderów
	shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/shader.frag");
	//kompilacja i konsolidacja programu shaderowego
	shader.CreateAndLinkProgram();
	shader.Use();	  
		//dodawanie atrybutów i uniformów
		shader.AddUniform("MVP");
		shader.AddUniform("time"); 
	shader.UnUse();

	GL_CHECK_ERRORS

	//load textured rendering shader
	texturedShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/shader.vert");
	texturedShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/textured.frag");
	//kompilacja i konsolidacja programu shaderowego
	texturedShader.CreateAndLinkProgram();
	texturedShader.Use();	  
		//dodawanie atrybutów i uniformów
		texturedShader.AddUniform("MVP");
		texturedShader.AddUniform("time"); 
		texturedShader.AddUniform("textureMap");
		//ustalanie wartości stałych uniformów	
		glUniform1i(texturedShader("textureMap"),0);
	texturedShader.UnUse();
	 
	GL_CHECK_ERRORS

	//ustawianie obiektu tablicy wierzchołków i obiektów bufora dla siatki
	//obsługa geometrii 
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	//wywołania pomiędzy start ATI i end ATI nie są wymagane 
	//ale w przypadku kart ATI, które testowałem, musiałem tworzyć specjalny obiekt bufora
	//Jeśli używasz sprzętu ATI, usuń znaki komentarza 
	/// start ATI ///
	
	glGenBuffers(1, &vboVerticesID);  
	glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
	glBufferData (GL_ARRAY_BUFFER, sizeof(GLubyte)*MAX_PARTICLES, 0, GL_STATIC_DRAW);	 
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,1,GL_UNSIGNED_BYTE, GL_FALSE, 0,0);
	
	/// end ATI ////

	GL_CHECK_ERRORS 
	
	//rozmiar cząsteczki
	glPointSize(10);

	//włączenie mieszania
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 
	//wyłączenie testowania głębi
	glDisable(GL_DEPTH_TEST);
	 
	//macierz transformacji emitera
	emitterXForm = glm::translate(glm::mat4(1), glm::vec3(0,0,0));
	emitterXForm = glm::rotate(emitterXForm, 90.0f, glm::vec3(0.0f, 0.0f, 1.0f));  

	//load the texture
	int texture_width=0, texture_height=0, channels=0;
	GLubyte* pData = SOIL_load_image(texture_filename.c_str(), &texture_width, &texture_height, &channels, SOIL_LOAD_AUTO);
	if(pData == NULL) {
		cerr<<"Nie moge wczytac obrazu: "<<texture_filename.c_str()<<endl;
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

	//generowanie tekstury OpenGL
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	//ustawianie parametrów tekstury
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//alokowanie tekstury 
	glTexImage2D(GL_TEXTURE_2D, 0, format, texture_width, texture_height, 0, format, GL_UNSIGNED_BYTE, pData);
	
	//zwalnianie zasobów biblioteki SOIL
	SOIL_free_image_data(pData);

	//ustawienie shadera cząsteczkowego jako bieżącego
	pCurrentShader = &shader;

	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {
	//usuwanie tekstury cząsteczki
	glDeleteTextures(1, &textureID);

	//likwidacja programu shaderowego
	pCurrentShader = NULL;
	shader.DeleteShaderProgram();
	texturedShader.DeleteShaderProgram();

	//likwidacja vao i vbo
	/// start ATI ///
	//glDeleteBuffers(1, &vboVerticesID); 
	/// end ATI ///

	glDeleteVertexArrays(1, &vaoID);

	cout<<"Zamkniecie powiodlo sie"<<endl;
}

//obsługa zmiany wymiarów okna
void OnResize(int w, int h) {
	//ustawienie wymiarów okna widokowego
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//wyznaczanie macierzy rzutowania
	P = glm::perspective(60.0f, (float)w/h,0.1f,100.0f);
}

//funkcja zwrotna wyświetlania
void OnRender() { 
	//pozyskiwanie czasu bieżącego
	GLfloat time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
	
	//czyszczenie buforów koloru i głębi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//transformacje kamery
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV	= glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f)); 
    glm::mat4 MVP	= P*MV;
	 
	//wiązanie shadera bieżącego
	pCurrentShader->Use();				
		//ustawianie uniformów shadera
		glUniform1f((*pCurrentShader)("time"), time);
		glUniformMatrix4fv((*pCurrentShader)("MVP"), 1, GL_FALSE, glm::value_ptr(P*MV*emitterXForm));
		//renderowanie punktów
			glDrawArrays(GL_POINTS, 0, MAX_PARTICLES);
	//odwiązanie shadera
	pCurrentShader->UnUse();
	
	//zamiana buforów w celu wyświetlenia wyrenderowanego obrazu
	glutSwapBuffers();
}

//wywołanie funkcji wyświetlającej w czasie bezczynności
void OnIdle() {
	glutPostRedisplay();
}

//obsługa klawiatury w celu przełączania trybów z cząsteczkami kolorowanymi lub teksturowanymi
void OnKey(unsigned char key, int x, int y) {
	switch (key) {
		case ' ': 
			if(pCurrentShader == &shader) {
				pCurrentShader = &texturedShader;  
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			} else {
				pCurrentShader = &shader; 
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
		break;
	}
}

int main(int argc, char** argv) {
	//inicjalizacja freeglut 
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);	
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Proste cząstki - OpenGL 3.3");

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
	glutIdleFunc(OnIdle);
	glutKeyboardFunc(OnKey);

	//wywołanie pętli głównej
	glutMainLoop();	

	return 0;
}
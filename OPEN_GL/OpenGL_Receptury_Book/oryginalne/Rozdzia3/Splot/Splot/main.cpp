#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL.h>

#include "..\\..\\src\GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace std;

//wymiary okna
const int WIDTH  = 512;
const int HEIGHT = 512;

//shadery do renderowania obrazu i wykonywania splotu
GLSLShader shader;
GLSLShader convolution_shader;
GLSLShader* current_shader;

//ID obiektu tablicy wierzcho�k�w i obiekt�w bufor�w wierzcho�k�w
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

//ID teksturowego obrazu
GLuint textureID;

//tablice wierzcho�k�w i indeks�w dla pe�noekranowego czworok�ta
glm::vec2 vertices[4];
GLushort indices[6];

//nazwa pliku z obrazem tekstury
const string filename = "media/Lenna.png";

void OnInit() {
	GL_CHECK_ERRORS
	//wczytanie shader�w
	shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/shader.frag");
	//kompilacja i konsolidacja programu shaderowego
	shader.CreateAndLinkProgram();
	shader.Use();
		//dodawanie atrybut�w i uniform�w
		shader.AddAttribute("vVertex");
		shader.AddUniform("textureMap");
		//ustalanie warto�ci sta�ych uniform�w
		glUniform1i(shader("textureMap"), 0);
	shader.UnUse();

	current_shader = &shader;

	GL_CHECK_ERRORS

	//wczytanie shader�w
	convolution_shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/shader.vert");
	convolution_shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/shader_convolution.frag");
	//kompilacja i konsolidacja programu shaderowego
	convolution_shader.CreateAndLinkProgram();
	convolution_shader.Use();
		//dodawanie atrybut�w i uniform�w
		convolution_shader.AddAttribute("vVertex");
		convolution_shader.AddUniform("textureMap");
		//ustalanie warto�ci sta�ych uniform�w
		glUniform1i(convolution_shader("textureMap"), 0);
	convolution_shader.UnUse();

	GL_CHECK_ERRORS

	//Ustalanie geometrii czworok�ta
	//ustalanie wierzcho�k�w czworok�ta
	vertices[0] = glm::vec2(0.0,0.0);
	vertices[1] = glm::vec2(1.0,0.0);
	vertices[2] = glm::vec2(1.0,1.0);
	vertices[3] = glm::vec2(0.0,1.0);

	//wype�nianie tablicy indeks�w czworok�ta
	GLushort* id=&indices[0];
	*id++ =0;
	*id++ =1;
	*id++ =2;
	*id++ =0;
	*id++ =2;
	*id++ =3;

	GL_CHECK_ERRORS

	//przygotowanie vao i vbo dla czworok�ta
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);

	glBindVertexArray(vaoID);

		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		//przekazanie wierzcho�k�w czworok�ta do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//w��czenie tablicy atrybut�w wierzho�ka dla po�o�enia
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 2, GL_FLOAT, GL_FALSE,0,0);
		GL_CHECK_ERRORS
		//przekazanie indeks�w czworok�ta do bufora tablicy element�w
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS


	//wczytanie obrazu
	int texture_width = 0, texture_height = 0, channels=0;
	GLubyte* pData = SOIL_load_image(filename.c_str(), &texture_width, &texture_height, &channels, SOIL_LOAD_AUTO);

	//odwracanie obrazu wzgl�dem osi Y
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

	//przygotowanie tekstury i zwi�zanie jej jednostk� teksturuj�c� o numerze 0
	glGenTextures(1, &textureID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		//ustawienie parametr�w tekstury
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		//alokowanie tekstury 
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pData);

	//zwalnianie zaj�tych wcze�niej zasob�w
	SOIL_free_image_data(pData);

	GL_CHECK_ERRORS

	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich alokowanych zasob�w
void OnShutdown() {

	current_shader = 0;

	//likwidacja shadera
	shader.DeleteShaderProgram();
	convolution_shader.DeleteShaderProgram();


	//Likwidacja vao i vbo
	glDeleteBuffers(1, &vboVerticesID);
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID);

	//usuwanie tekstur
	glDeleteTextures(1, &textureID);
	cout<<"Zamkniecie powiodlo sie"<<endl;
}

//obs�uga zdarzenia zmiany wymiar�w okna
void OnResize(int w, int h) {
	//ustalanie wymiar�w okna widokowego
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
}

//funkcja wy�wietlaj�ca
void OnRender() {
	//czyszczenie bufor�w koloru i g��bi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//wi�zanie shadera
	current_shader->Use();
		//rysowanie pe�noekranowego czworok�ta
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	//odwi�zywanie shadera
	current_shader->UnUse();

	//zamiana bufor�w w celu wy�wietlenia obrazu
	glutSwapBuffers();
}

//obs�uga zdarze� klawiszowych w celu zmiany obrazu z filtrowanego na oryginalny 
void OnKey(unsigned char key, int x, int y) {
	switch(key) {
		case ' ': {
			if(current_shader == &shader) {
				current_shader = &convolution_shader;
				glutSetWindowTitle("Obraz filtrowany");
			} else {
				current_shader = &shader;
				glutSetWindowTitle("Obraz oryginalny");
			}
		}break;
	}
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
	glutCreateWindow("Splot - OpenGL 3.3");
	
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
	cout << "\tWersja GLEW " << glewGetString(GLEW_VERSION) << endl;
	cout << "\tProducent: " << glGetString(GL_VENDOR) << endl;
	cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
	cout << "\tWersja OpenGL: " << glGetString(GL_VERSION) << endl;
	cout << "\tGLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	cout<<"Wcisnij klawisz ' ', aby wlaczy� lub wylaczy� filtr\n";
	GL_CHECK_ERRORS

	//inicjalizacja OpenGL
	OnInit();

	//rejestracja funkcji zwrotnych
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutKeyboardFunc(OnKey);

	//wywo�anie p�tli g��wnej
	glutMainLoop();

	return 0;
}
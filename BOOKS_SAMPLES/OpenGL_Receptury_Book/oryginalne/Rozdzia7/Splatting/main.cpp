#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "..\src\GLSLShader.h"
#include <fstream>

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);


#pragma comment(lib, "glew32.lib")

using namespace std;

//wymiary okna
const int WIDTH  = 1280;
const int HEIGHT = 960;

//krotność pomniejszenia obrazu
const int DOWN_SAMPLE = 2;
//wymiary pomniejszonego obrazu
const int IMAGE_WIDTH = WIDTH/DOWN_SAMPLE;
const int IMAGE_HEIGHT = HEIGHT/DOWN_SAMPLE;

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=4, rY=50, dist = -2;

//macierze rzutowania oraz modelu i widoku
glm::mat4 MV,P;
 
//identyfikatory obiektów tablicy i bufora wierzchołków placków wolumetrycznych
GLuint volumeSplatterVBO;
GLuint volumeSplatterVAO;

//shadery wykonujące splatting, wygładzanie gaussowskie i renderowanie czworokąta
GLSLShader shader, gaussianV_shader, gaussianH_shader, quadShader;

//kolor tła
glm::vec4 bg=glm::vec4(0,0,0,1);

//nazwa pliku z danymi wolumetrycznymi
const std::string volume_file = "../media/Engine256.raw";
 
//instancja klasy VolumeSplatter
#include "VolumeSplatter.h"
VolumeSplatter* splatter;

//identyfikatory FBO dla normalnych,dla filtrowania i dla renderowania
GLuint filterFBOID, fboID, rboID;

//identyfikator tekstury rozmytej
GLuint blurTexID[2];
//identyfikator tekstury przyłącza koloru
GLuint texID;

//identyfikatory tablicy wierzchołków i obiektów bufora dla czworokąta
GLuint quadVAOID;
GLuint quadVBOID;
GLuint quadIndicesID;

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
	if (state == 0) {
		dist += (y - oldY)/50.0f;
	} else {
		rX += (y - oldY)/5.0f;
		rY += (x - oldX)/5.0f;
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}

//inicjalizacja OpenGL
void OnInit() {

	GL_CHECK_ERRORS

	//tworzenie instancji klasy VolumeSplatter
	splatter = new VolumeSplatter();
	//wymiary obszaru wolumetrycznego
	splatter->SetVolumeDimensions(256,256,256);
	//wczytanie danych wolumetrycznych
	splatter->LoadVolume(volume_file);
	//ustalenie wartości określającej izopowierzchnię
	splatter->SetIsosurfaceValue(40);
	//ustalenie liczby próbkowanych wokseli
	splatter->SetNumSamplingVoxels(64,64,64);
	std::cout<<"Generuje punktowe placki ...";
	//generowanie placków
	splatter->SplatVolume();
	std::cout<<"Gotowe."<<std::endl;

	//generowanie obiektów tablicy i bufora wierzchołków 
	glGenVertexArrays(1, &volumeSplatterVAO);
	glGenBuffers(1, &volumeSplatterVBO);
	glBindVertexArray(volumeSplatterVAO);
	glBindBuffer (GL_ARRAY_BUFFER, volumeSplatterVBO);

	//przekazanie wierzchołków z obiektu splatter do bufora wierzchołków
	glBufferData (GL_ARRAY_BUFFER, splatter->GetTotalVertices()*sizeof(Vertex), splatter->GetVertexPointer(), GL_STATIC_DRAW);

	//włączenie tablicy atrybutów wierzchołka dla położenia
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);

	//włączenie tablicy atrybutów wierzchołka dla normalnych
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),(const GLvoid*)offsetof(Vertex, normal));

	GL_CHECK_ERRORS

	//wczytywanie shaderów
	shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/splatShader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/splatShader.frag");
	//kompilacja i konsolidacja programu shaderowego 
	shader.CreateAndLinkProgram();
	shader.Use();
		//dodawanie atrybutów i uniformów
		shader.AddAttribute("vVertex");
		shader.AddAttribute("vNormal");
		shader.AddUniform("MV");
		shader.AddUniform("N");
		shader.AddUniform("P");
		shader.AddUniform("splatSize");
		//ustalanie wartości stałych uniformów
		glUniform1f(shader("splatSize"), 256/64);
	shader.UnUse();

	GL_CHECK_ERRORS

	//wczytanie shadera wygładzającego w poziomie
	gaussianH_shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/Passthrough.vert");
	gaussianH_shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/GaussH.frag");

	//kompilacja i konsolidacja programu shaderowego
	gaussianH_shader.CreateAndLinkProgram();
	gaussianH_shader.Use();

		//dodawanie atrybutów i uniformów
		gaussianH_shader.AddAttribute("vVertex");
		gaussianH_shader.AddUniform("textureMap");
		//pass constant uniforms once
		//ten shader czyta teksturę przyłączaną do jednostki teksturującej nr 1
		glUniform1i(gaussianH_shader("textureMap"),1);
	gaussianH_shader.UnUse();

	GL_CHECK_ERRORS

	//wczytanie shadera wygładzającego w pionie
	gaussianV_shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/Passthrough.vert");
	gaussianV_shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/GaussV.frag");

	//kompilacja i konsolidacja programu shaderowego
	gaussianV_shader.CreateAndLinkProgram();
	gaussianV_shader.Use();
		//dodawanie atrybutów i uniformów 
		gaussianV_shader.AddAttribute("vVertex");
		gaussianV_shader.AddUniform("textureMap");
		//ustalanie wartości stałych uniformów
		//ten shader czyta teksturę przyłączaną do jednostki teksturującej nr 0
		glUniform1i(gaussianV_shader("textureMap"),0);
	gaussianV_shader.UnUse();

	GL_CHECK_ERRORS

	//wczytywanie shaderów czworokąta
	quadShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/quad_shader.vert");
	quadShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/quad_shader.frag");

	//kompilacja i konsolidacja programu shaderowego 
	quadShader.CreateAndLinkProgram();
	quadShader.Use();
		//dodawanie atrybutów i uniformów
		quadShader.AddAttribute("vVertex");
		quadShader.AddUniform("MVP");
		quadShader.AddUniform("textureMap");
		//ustalanie wartości stałych uniformów
		//ten shader czyta teksturę przyłączaną do jednostki teksturującej nr 2
		glUniform1i(quadShader("textureMap"), 2);
	quadShader.UnUse();

	GL_CHECK_ERRORS

	//ustawienie koloru tła
	glClearColor(bg.r, bg.g, bg.b, bg.a);

	//włączenie testowania głębi i zasłaniania
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//zezwolenie shaderowi wierzchołków na zmianę rozmiaru punktu 
	//przez odpowiedni wpis w rejestrze gl_PointSize
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	//ustawianie FBO dla filtrowania
	glGenFramebuffers(1,&filterFBOID);
	glBindFramebuffer(GL_FRAMEBUFFER,filterFBOID);

	//wygeneruj dwie tekstury dla filtrowania i przyłącz je do jednostek 1 i 2
	// połącz je także z przyłączami koloru o numerach 0 i 1
	glGenTextures(2, blurTexID);
	for(int i=0;i<2;i++) {
		glActiveTexture(GL_TEXTURE1+i);
		glBindTexture(GL_TEXTURE_2D, blurTexID[i]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,IMAGE_WIDTH,IMAGE_HEIGHT,0,GL_RGBA,GL_FLOAT,NULL);		
		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0+i,GL_TEXTURE_2D,blurTexID[i],0);
	}

	//sprawdzian kompletności FBO
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if(status == GL_FRAMEBUFFER_COMPLETE) {
		cout<<"Ustawienie filtrujacego FBO powiodlo się."<<endl;
	} else {
		cout<<"Problem z ustawieniem filtrującego FBO."<<endl;
	}

	GL_CHECK_ERRORS

	//ustawienie FBO dla sceny i obiektu bufora renderingu (RBO)
	glGenFramebuffers(1,&fboID);
	glGenRenderbuffers(1, &rboID);
	//generowanie tekstury dla przyłącza koloru 
	glGenTextures(1, &texID);
	//wiązanie FBO i RBO
	glBindFramebuffer(GL_FRAMEBUFFER,fboID);
	glBindRenderbuffer(GL_RENDERBUFFER, rboID);
	//przyłączanie tekstury do jednostki 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);

	GL_CHECK_ERRORS
	//ustawianie parametrów tekstury
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	//alokowanie tekstury
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,IMAGE_WIDTH,IMAGE_HEIGHT,0,GL_RGBA,GL_FLOAT,NULL);

	GL_CHECK_ERRORS

	//wiązanie tekstury z przyłączem koloru nr 0
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, texID, 0);
	//powiązanie bufora renderingu z przyłączem głębi
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, IMAGE_WIDTH, IMAGE_HEIGHT);

	//sprawdzian kompletności FBO
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if(status == GL_FRAMEBUFFER_COMPLETE) {
		cout<<"Ustawienie FBO dla renderingu pozaekranowego powiodlo sie."<<endl;
	} else {
		cout<<"Problem z ustawieniem FBO dla renderingu pozaekranowego."<<endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_CHECK_ERRORS

	//ustawienie wierzchołków czworokąta
	glm::vec2 quadVerts[4];
	quadVerts[0] = glm::vec2(0,0);
	quadVerts[1] = glm::vec2(1,0);
	quadVerts[2] = glm::vec2(1,1);
	quadVerts[3] = glm::vec2(0,1);

	//indeksy czworokąta
	GLushort indices[6]={0,1,2,0,2,3};

	//generowanie tablicy wierzchołków i obiektów bufora dla czworokąta
	glGenVertexArrays(1, &quadVAOID);
	glGenBuffers(1, &quadVBOID);
	glGenBuffers(1, &quadIndicesID);

	GL_CHECK_ERRORS

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
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
	GL_CHECK_ERRORS

	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {
	glDeleteFramebuffers(1, &filterFBOID);
	glDeleteTextures(2, blurTexID);

	glDeleteFramebuffers(1,&fboID);
	glDeleteTextures(1, &texID);
	glDeleteRenderbuffers(1, &rboID);

	glDeleteVertexArrays(1, &volumeSplatterVAO);
	glDeleteBuffers(1, &volumeSplatterVBO);

	delete splatter;

	quadShader.DeleteShaderProgram();
	gaussianH_shader.DeleteShaderProgram();
	gaussianV_shader.DeleteShaderProgram();
	shader.DeleteShaderProgram();

	glDeleteVertexArrays(1, &quadVAOID);
	glDeleteBuffers(1, &quadVBOID);
	glDeleteBuffers(1, &quadIndicesID);

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
	//transformacje kamery
	glm::mat4 Tr	= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(Tr,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//czyszczenie buforów koloru i głębi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//wiązanie FBO
	glBindFramebuffer(GL_FRAMEBUFFER,fboID);

	//ustaw wymiary okna widokowego równe wymiarom przyłącza koloru w FBO
	glViewport(0,0, IMAGE_WIDTH, IMAGE_HEIGHT);
	//ustawianie zerowego przyłącza koloru jako bufora rysowania
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//czyszczenie buforów koloru i głębi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//przenoszenie placka do początku układu współrzędnych
	glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(-0.5,-0.5,-0.5));

	//wiązanie obiektu tablicy wierzchołków volumeSplatterVAO
	glBindVertexArray(volumeSplatterVAO);
		//wiązanie shadera realizującego splatting
		shader.Use();
			//ustawianie uniformów shadera
			glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV*T));
			glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV*T))));
			glUniformMatrix4fv(shader("P"), 1, GL_FALSE, glm::value_ptr(P));
				//rysowanie punktów
				glDrawArrays(GL_POINTS, 0, splatter->GetTotalVertices());
		//odwiązanie shadera realizującego splatting
		shader.UnUse();

	//wiązanie obiektu tablicy wierzchołków czworokąta
	glBindVertexArray(quadVAOID);

	//wiązanie FBO dla filtrowania
	glBindFramebuffer(GL_FRAMEBUFFER, filterFBOID);
	//ustawianie zerowego przyłącza koloru jako bufora rysowania
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
		//ustawienie shadera wygładzającego w pionie
		gaussianV_shader.Use();
			//rysowanie pełnoekranowego czworokąta
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	//ustawianie przyłącza koloru nr 1 jako bufora rysowania
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
		//ustawienie shadera wygładzającego w poziomie 
		gaussianH_shader.Use();
			//rysowanie pełnoekranowego czworokąta
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	//odwiązanie FBO
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	//przywrócenie domyślnego bufora ekranu
	glDrawBuffer(GL_BACK_LEFT);
	//przywrócenie domyślnych wymiarów okna widokowego
	glViewport(0,0,WIDTH, HEIGHT);

	//ustawienie shadera czworokąta
	quadShader.Use();
		//narysuj pełnoekranowy czworokąt, aby wyświetlić na nim przefiltrowany rezultat
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	//odłącz shader czworokąta
	quadShader.UnUse();

	//odwiązanie obiektu tablicy wierzchołków 
	glBindVertexArray(0);

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
	glutCreateWindow("Rendering wolumetryczny z użyciem splattingu - OpenGL 3.3");

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

	//wywołanie pętli głównej
	glutMainLoop();

	return 0;
}
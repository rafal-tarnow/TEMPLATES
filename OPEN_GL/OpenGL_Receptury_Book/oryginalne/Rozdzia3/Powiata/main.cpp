
#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "..\src\GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")

using namespace std;

//wymiary okna
const int WIDTH  = 1280;
const int HEIGHT = 960;

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=0, rY=0, fov = 45;


#include "..\src\FreeCamera.h"

//kody klawiszy
const int VK_W = 0x57;
const int VK_S = 0x53;
const int VK_A = 0x41;
const int VK_D = 0x44;
const int VK_Q = 0x51;
const int VK_Z = 0x5a;

//do ustalania dok³adnoœci obliczeñ
const float EPSILON = 0.001f;
const float EPSILON2 = EPSILON*EPSILON;

//przyrost czasu
float dt = 0;

//instancja kamery swobodnej
CFreeCamera cam;

//wyjœcie informacyjne
#include <sstream>
std::stringstream msg;

//siatka
#include "..\src\Grid.h"
CGrid* grid;

//kostka
#include "..\src\UnitCube.h"
CUnitCube* cube;

//macierze modelu i widoku, rzutowania oraz obrotu
glm::mat4 MV,P;
glm::mat4 Rot;

//zmienne zwi¹zane z up³ywem czasu
float last_time=0, current_time=0;

//zmienne zwi¹zane z wyg³adzaniem ruchów myszy
const float MOUSE_FILTER_WEIGHT=0.75f;
const int MOUSE_HISTORY_BUFFER_SIZE = 10;

//bufor historii po³o¿eñ myszy
glm::vec2 mouseHistory[MOUSE_HISTORY_BUFFER_SIZE];

float mouseX=0, mouseY=0; //wyg³adzone wpó³rzêdne myszy

//wy³¹cznik wyg³adzania ruchów myszy
bool useFiltering = true;

//odleg³oœæ cz¹stek
float radius = 2;

//po³o¿enia cz¹stek
glm::vec3 particles[8];

//ID obiektów VAO iVBO dla wierzcho³ków cz¹stek
GLuint particlesVAO;
GLuint particlesVBO;

//shadery cz¹stek i rozmycia
GLSLShader particleShader;
GLSLShader blurShader;

//ID VAO i VBO dla wierzcho³ków czworok¹ta
GLuint quadVAOID;
GLuint quadVBOID;
GLuint quadVBOIndicesID;

//k¹t obrotu automatycznego
float angle = 0;

//FBO ID
GLuint fboID;
//Tekstrury w przy³¹czach koloru FBO
GLuint texID[2]; //0 -> wyrenderowana poœwiata
				 //1 -> rozmycie

//szerokoœæ i wysokoœæ przy³¹cza FBO
const int RENDER_TARGET_WIDTH = WIDTH>>1;
const int RENDER_TARGET_HEIGHT = HEIGHT>>1;

//funkcja wyg³adzaj¹ca ruchy myszy
void filterMouseMoves(float dx, float dy) {
    for (int i = MOUSE_HISTORY_BUFFER_SIZE - 1; i > 0; --i) {
        mouseHistory[i] = mouseHistory[i - 1];
    }

    //zapisywanie bie¿¹cego po³o¿enia myszy do bufora historii
    mouseHistory[0] = glm::vec2(dx, dy);

    float averageX = 0.0f;
    float averageY = 0.0f;
    float averageTotal = 0.0f;
    float currentWeight = 1.0f;

    //wyg³adzanie
    for (int i = 0; i < MOUSE_HISTORY_BUFFER_SIZE; ++i)
    {
		glm::vec2 tmp=mouseHistory[i];
        averageX += tmp.x * currentWeight;
        averageY += tmp.y * currentWeight;
        averageTotal += 1.0f * currentWeight;
        currentWeight *= MOUSE_FILTER_WEIGHT;
    }

    mouseX = averageX / averageTotal;
    mouseY = averageY / averageTotal;

}

//obs³uga klikniêcia przyciskiem myszy
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

//obs³uga ruchów myszy
void OnMouseMove(int x, int y)
{
	if (state == 0) {
		fov += (y - oldY)/5.0f;
		cam.SetupProjection(fov, cam.GetAspectRatio());
	} else {
		rY += (y - oldY)/5.0f;
		rX += (oldX-x)/5.0f;
		if(useFiltering)
			filterMouseMoves(rX, rY);
		else {
			mouseX = rX;
			mouseY = rY;
		}
		cam.Rotate(mouseX,mouseY, 0);
	}
	oldX = x;
	oldY = y;
	glutPostRedisplay();
}

//inicjalizacja OpenGL
void OnInit() {

	//tworzenie obiektu siatki
	grid = new CGrid(20,20);

	//tworzenie kostki
	cube = new CUnitCube();
	//ustawienie niebieskiego koloru kostki
	cube->color = glm::vec3(0,0,1);

	GL_CHECK_ERRORS

	//ustalenie po³o¿enia kamery
	glm::vec3 p = glm::vec3(5,5,5);
	cam.SetPosition(p);

	//obracanie kamery we w³aœciwym kierunku
	glm::vec3 look=  glm::normalize(p);
	float yaw = glm::degrees(float(atan2(look.z, look.x)+M_PI));
	float pitch = glm::degrees(asin(look.y));
	rX = yaw;
	rY = pitch;
	if(useFiltering) {
		for (int i = 0; i < MOUSE_HISTORY_BUFFER_SIZE ; ++i) {
			mouseHistory[i] = glm::vec2(rX, rY);
		}
	}
	cam.Rotate(rX,rY,0);

	//w³¹czenie testu g³êbi
	glEnable(GL_DEPTH_TEST);

	//rozmiar cz¹stki
	glPointSize(50);

	//wczytanie shaderów cz¹stek
	particleShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/particle.vert");
	particleShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/particle.frag");
	//kompilacja i konsolidacja programu shaderowego
	particleShader.CreateAndLinkProgram();
	particleShader.Use();
		//ustawienie atrybutów
		particleShader.AddAttribute("vVertex");
		particleShader.AddUniform("MVP");
	particleShader.UnUse();

	//po³o¿enia cz¹stek
	for(int i=0;i<8;i++) {
		float theta = float(i/8.0f * 2 * M_PI);
		particles[i].x = radius*cos(theta);
		particles[i].y = 0.0f;
		particles[i].z = radius*sin(theta);
	}

	//generowanie VAO i VBO dla cz¹stek
	glGenVertexArrays(1, &particlesVAO);
	glGenBuffers(1, &particlesVBO);
	glBindVertexArray(particlesVAO);

		glBindBuffer (GL_ARRAY_BUFFER, particlesVBO);
		//przekazywanie wierzcho³ków cz¹stki do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, sizeof(particles), &particles[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//w³¹czenie tablicy atrybutów wierzcho³kowych dla po³o¿eñ
		glEnableVertexAttribArray(particleShader["vVertex"]);
		glVertexAttribPointer(particleShader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);

		GL_CHECK_ERRORS

	//ustawienie FBO i pozaekranowego celu renderingu
	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);

	//ustawienie dwóch przy³¹czy koloru
	glGenTextures(2, texID);
	glActiveTexture(GL_TEXTURE0);
	for(int i=0;i<2;i++) {		
		glBindTexture(GL_TEXTURE_2D, texID[i]);
		//ustawienie parametrów tekstury
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//alokowanie obiektu tekstury
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RENDER_TARGET_WIDTH, RENDER_TARGET_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		GL_CHECK_ERRORS

		//ustawienie bie¿¹cej tekstury jako przy³¹cza FBO 
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, texID[i], 0);
	}
	
	GL_CHECK_ERRORS

	//sprawdzian kompletnoœci bufora ramki
	GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE) {
		cerr<<"Blad przy ustawianiu FBO."<<endl;
		exit(EXIT_FAILURE);
	} else {
		cerr<<"Ustawianie FBO powiodlo sie."<<endl;
	}

	//unbind the FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	GL_CHECK_ERRORS

	//wierzcho³ki pe³noekranowego czworok¹ta
	glm::vec2 vertices[4];
	vertices[0]=glm::vec2(0,0);
	vertices[1]=glm::vec2( 1,0);
	vertices[2]=glm::vec2( 1, 1);
	vertices[3]=glm::vec2(0, 1);

	GLushort indices[6];

	int count = 0;

	//wype³nianie tablicy indeksów czworok¹ta
	GLushort* id=&indices[0];
	*id++ = 0; 	*id++ = 1; 	*id++ = 2;
	*id++ = 0; 	*id++ = 2; 	*id++ = 3;

	//wczytanie shaderów rozmycia
	blurShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/full_screen_shader.vert");
	blurShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/full_screen_shader.frag");
	////kompilacja i konsolidacja programu shaderowego
	blurShader.CreateAndLinkProgram();
	blurShader.Use();
		//dodawanie atrybutów i uniformów
		blurShader.AddAttribute("vVertex");
		blurShader.AddUniform("textureMap");
		//ustalanie wartoœci sta³ych uniformów
		glUniform1i(blurShader("textureMap"),0);
	blurShader.UnUse();

	GL_CHECK_ERRORS

	//ustawianie VAO i VBO dla czworok¹ta
	glGenVertexArrays(1, &quadVAOID);
	glGenBuffers(1, &quadVBOID);
	glGenBuffers(1, &quadVBOIndicesID);

	glBindVertexArray(quadVAOID);

		glBindBuffer (GL_ARRAY_BUFFER, quadVBOID);
		//przekazanie wierzcho³ków czworok¹ta
		glBufferData (GL_ARRAY_BUFFER, 4*sizeof(glm::vec2), &vertices[0], GL_STATIC_DRAW);

		//w³¹czenie tablicy atrybutów wierzcho³kowych dla po³o¿eñ
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,0,0);

		//przekazanie indeksów czworok¹ta do bufora tablicy elementów
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadVBOIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*6, &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

	GL_CHECK_ERRORS
	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {
	particleShader.DeleteShaderProgram();
	blurShader.DeleteShaderProgram();

	delete grid;
	delete cube;

	glDeleteBuffers(1, &particlesVBO);
	glDeleteBuffers(1, &particlesVAO);

	glDeleteBuffers(1, &quadVBOID);
	glDeleteBuffers(1, &quadVBOIndicesID);
	glDeleteBuffers(1, &quadVAOID);

	glDeleteTextures(2, texID);
	glDeleteFramebuffers(1, &fboID);

	cout<<"Zamkniecie powiodlo sie"<<endl;
}

//obs³uga zmiany wymiarów okna
void OnResize(int w, int h) {
	//wymiary okna widokowego
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//macierz rzutowania kamery
	cam.SetupProjection(fov, (GLfloat)w/h);
}

//reakcja na sygna³ bezczynnoœci procesora
void OnIdle() {
	//generowanie macierzy obrotu w celu wykonania obrotu wokó³ osi Y przy ka¿dym sygnale bezczynnoœci
	Rot = glm::rotate(glm::mat4(1), angle++, glm::vec3(0,1,0));

	//obs³uga klawiszy W,S,A,D,Q i Z s³u¿¹cych do poruszania kamer¹
	if( GetAsyncKeyState(VK_W) & 0x8000) {
		cam.Walk(dt);
	}

	if( GetAsyncKeyState(VK_S) & 0x8000) {
		cam.Walk(-dt);
	}

	if( GetAsyncKeyState(VK_A) & 0x8000) {
		cam.Strafe(-dt);
	}

	if( GetAsyncKeyState(VK_D) & 0x8000) {
		cam.Strafe(dt);
	}

	if( GetAsyncKeyState(VK_Q) & 0x8000) {
		cam.Lift(dt);
	}

	if( GetAsyncKeyState(VK_Z) & 0x8000) {
		cam.Lift(-dt);
	}
	glm::vec3 t = cam.GetTranslation(); 

	if(glm::dot(t,t)>EPSILON2) {
		cam.SetTranslation(t*0.95f);
	}

	//wywo³anie funkcji wyœwietlaj¹cej
	glutPostRedisplay();
}

//funkcja zwrotna wyœwietlania
void OnRender() {
	static int total = 0;
	static int offset = 0;
	total++;

	if(total%50 == 0)
	{
		offset = 4-offset;
	}
	GL_CHECK_ERRORS

	//obliczenia zwi¹zane z czasem
	last_time = current_time;
	current_time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
	dt = current_time-last_time;

	//czyszczenie buforów koloru i g³êbi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//ustawianie macierzy modelu i widoku oraz rzutowania a potem ich ³¹czenie
	MV	= cam.GetViewMatrix();
	P   = cam.GetProjectionMatrix();
    glm::mat4 MVP	= P*MV;

	//zwyk³e renderowanie sceny
	//renderowanie siatki
	grid->Render(glm::value_ptr(MVP));

	//renderowanie kostki
	cube->Render(glm::value_ptr(MVP));

	//ustawianie obiektu tablicy wierzcho³ków cz¹stek
	glBindVertexArray(particlesVAO);
		//uruchomienie shadera cz¹stek
		particleShader.Use();
			//ustawienie uniformów
			glUniformMatrix4fv(particleShader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP*Rot));
				//rysowanie cz¹stek
	     		glDrawArrays(GL_POINTS, 0, 8);

	 GL_CHECK_ERRORS


	//Aktywacja FBO i renderowanie elementów z poswiat¹
	//w naszym przyk³adzie s¹ to cztery pierwsze cz¹stki
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);
	
	//ustawienie wymiarów okna widokowego równych wymiarom pozaekranowego celu renderingu
	glViewport(0,0,RENDER_TARGET_WIDTH,RENDER_TARGET_HEIGHT);

	//ustawienie przy³¹cza koloru o numerze 0 jako bufora rysowania 
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
		//czyszczenie bufora koloru
		glClear(GL_COLOR_BUFFER_BIT);
			//renderowanie 4 cz¹stek
			glDrawArrays(GL_POINTS, offset, 4);
		//wy³¹czenie shadera cz¹stek
		particleShader.UnUse();
	GL_CHECK_ERRORS

	//ustawienie przy³¹cza koloru o numerze 1 jako bufora rysowania
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	//zwi¹zanie wyjœcia poprzedniego etapu jako tekstury
	glBindTexture(GL_TEXTURE_2D, texID[0]);
		//w³¹czenie shadera rozmycia
		blurShader.Use();
			//zwi¹zanie tablicy wierzcho³ków czworok¹ta pe³noekranowego
			glBindVertexArray(quadVAOID);
				//renderowanie czworok¹ta pe³noekranowego
				glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_SHORT,0);

	GL_CHECK_ERRORS

	//odwi¹zanie FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//przywracanie domyœlnego bufora tylnego
	glDrawBuffer(GL_BACK_LEFT);
	//wi¹zanie filtrowanej tekstury z ostatniego etapu
	glBindTexture(GL_TEXTURE_2D, texID[1]);

	GL_CHECK_ERRORS

	//przywracanie domyœlnego okna widokowego
	glViewport(0,0,WIDTH, HEIGHT);
	GL_CHECK_ERRORS
	//w³¹czenie mieszania addytywnego
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
		//rysowanie czworokata pe³noekranowego
		glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_SHORT,0);
	glBindVertexArray(0);

	//wy³¹czanie shadera rozmycia
	blurShader.UnUse();

	//wy³¹czanie mieszania
	glDisable(GL_BLEND);

	GL_CHECK_ERRORS
	
	//szamiana buforów w celu wyœwietlenia wyrenderowanego obrazu
	glutSwapBuffers();
}

int main(int argc, char** argv) {
	//inicjalizacja freeglut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Poœwiata - OpenGL 3.3");

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
	glutIdleFunc(OnIdle);

	//wywo³anie pêtli g³ównej
	glutMainLoop();

	return 0;
}
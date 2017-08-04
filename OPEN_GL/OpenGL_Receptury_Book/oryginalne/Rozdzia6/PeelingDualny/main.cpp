#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "..\src\GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

const float MAX_DEPTH = 1.0f;

#pragma comment(lib, "glew32.lib")

using namespace std;

//wymiary okna
const int WIDTH  = 1280;
const int HEIGHT = 960;

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=0, rY=300, dist = -10;

//obiekt siatki
#include "..\src\Grid.h"
CGrid* grid;

//macierze modelu i widoku, rzutowania oraz obrotu
glm::mat4 MV,P,R;

//stałe kolory kostek 
glm::vec4 box_colors[3]={glm::vec4(1,0,0,0.5),
						 glm::vec4(0,1,0,0.5),
						 glm::vec4(0,0,1,0.5)
							};

//Kąt automatycznego obrotu
float angle = 0;

//identyfikator FBO dla dualnego peelingu głębi
GLuint dualDepthFBOID;
//identyfikatory przyłączy koloru tekstury tylnej
GLuint backTexID[2];
//identyfikatory przyłączy koloru tekstury przedniej
GLuint texID[2];
//identyfikatory przyłączy głębi tekstury tylnej
GLuint depthTexID[2];
//identyfikator obiektu FBO do mieszania kolorów
GLuint colorBlenderFBOID;
//identyfikator przyłącza koloru w mieszającym FBO 
GLuint colorBlenderTexID;

//identyfikator kwerendy widoczności
GLuint queryId;

//VAO i VAO czworokąta pełnoekranowego
GLuint quadVAOID;
GLuint quadVBOID;
GLuint quadIndicesID;

//identyfikatory tablicy wierzchołków i obiektów bufora dla kostki
GLuint cubeVBOID;
GLuint cubeVAOID;
GLuint cubeIndicesID;

//shadery kostki, inicjalizacji, peelingu dualnego, mieszania i finalnego renderingu
GLSLShader cubeShader, initShader, dualPeelShader, blendShader, finalShader;

//liczba przejść peelingowych
const int NUM_PASSES=4;

//znacznik włączający kwerendę widoczności
bool bUseOQ = true;

//znacznik włączający dualny peeling głębi
bool bShowDepthPeeling = true;

//współczynnik mieszania kolorów
float alpha=0.6f;

//kolor tła
glm::vec4 bg=glm::vec4(0,0,0,0);

//identyfikatory przyłączy koloru 
GLenum attachID[2]={GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT3};

//rysowanie załączników z poszczególnych przyłączy
GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0,
						   GL_COLOR_ATTACHMENT1,
						   GL_COLOR_ATTACHMENT2,
						   GL_COLOR_ATTACHMENT3,
						   GL_COLOR_ATTACHMENT4,
						   GL_COLOR_ATTACHMENT5,
						   GL_COLOR_ATTACHMENT6
};

//inicjalizacja FBO
void initFBO() {
	//generowanie FBO dla dualnego peelingu
	glGenFramebuffers(1, &dualDepthFBOID);
	//FBO ma 4 przyłącza koloru 
	glGenTextures (2, texID);
	glGenTextures (2, backTexID);
	//FBO ma 2 przyłącza głębi 
	glGenTextures (2, depthTexID);

	//dla każdego przyłącza
	for(int i=0;i<2;i++) {
		//najpierw inicjalizujemy teksturę głębi
		glBindTexture(GL_TEXTURE_RECTANGLE, depthTexID[i]);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_RECTANGLE , 0, GL_FLOAT_RG32_NV, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);

		//a potem inicjalizujemy przyłącze koloru
		glBindTexture(GL_TEXTURE_RECTANGLE,texID[i]);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_RECTANGLE , 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

		//następnie inicjalizujemy tylne przyłącze koloru
		glBindTexture(GL_TEXTURE_RECTANGLE,backTexID[i]);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE , GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_RECTANGLE , 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	}

	GL_CHECK_ERRORS

	//ustawianie parametrów przyłącza koloru w mieszającym FBO
	glGenTextures(1, &colorBlenderTexID);
	glBindTexture(GL_TEXTURE_RECTANGLE, colorBlenderTexID);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, 0);

	//generowanie identyfikatora obiektu FBO do mieszania kolorów
	glGenFramebuffers(1, &colorBlenderFBOID);
	glBindFramebuffer(GL_FRAMEBUFFER, colorBlenderFBOID);
	//ustawianie tekstury koloru w mieszającym FBO jako przyłącza koloru w FBO 
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, colorBlenderTexID, 0);

	//wiązanie FBO dla dualnego peelingu
	glBindFramebuffer(GL_FRAMEBUFFER, dualDepthFBOID);

	//przyłączanie sześciu tekstur do tego FBO
	for(int i=0;i<2;i++) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachID[i], GL_TEXTURE_RECTANGLE, depthTexID[i], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachID[i]+1, GL_TEXTURE_RECTANGLE, texID[i], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachID[i]+2, GL_TEXTURE_RECTANGLE, backTexID[i], 0);
	}

	//ustawianie tekstury dla mieszania kolorów jako 7. przyłącza 
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_RECTANGLE, colorBlenderTexID, 0);
		
	//sprawdzian kompletności FBO
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status == GL_FRAMEBUFFER_COMPLETE )
		printf("Ustawienie FBO powiodlo sie !!! \n");
	else
		printf("Problem z ustawieniem FBO");

	//odwiązanie FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//zwalnianie zasobów związanych z FBO
void shutdownFBO() {
	glDeleteFramebuffers(1, &dualDepthFBOID);
	glDeleteFramebuffers(1, &colorBlenderFBOID);
	glDeleteTextures (2, texID);
	glDeleteTextures (2, backTexID);
	glDeleteTextures (2, depthTexID);
	glDeleteTextures(1, &colorBlenderTexID);
}

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
		dist += (y - oldY)/5.0f;
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

	//inicjalizacja FBO
	initFBO();

	//generowanie kwerendy sprzętowej
	glGenQueries(1, &queryId);

	//tworzenie regularnej siatki o wymiarach 20x20 w płaszczyźnie XZ
	grid = new CGrid(20,20);

	GL_CHECK_ERRORS

	//generowanie wierzchołków czworokąta
	glm::vec2 quadVerts[4];
	quadVerts[0] = glm::vec2(0,0);
	quadVerts[1] = glm::vec2(1,0);
	quadVerts[2] = glm::vec2(1,1);
	quadVerts[3] = glm::vec2(0,1);

	//generowanie indeksów czworokąta 
	GLushort quadIndices[]={ 0,1,2,0,2,3};

	//generowanie tablicy wierzchołków i obiektów bufora dla czworokąta
	glGenVertexArrays(1, &quadVAOID);
	glGenBuffers(1, &quadVBOID);
	glGenBuffers(1, &quadIndicesID);

	glBindVertexArray(quadVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, quadVBOID);
		//przekazanie wierzchołków czworokąta do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, sizeof(quadVerts), &quadVerts[0], GL_STATIC_DRAW);

		GL_CHECK_ERRORS

		//włączenie tablicy atrybutów wierzchołka dla położenia
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,0,0);

		//przekazanie indeksów czworokąta do bufora tablicy elementów
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, quadIndicesID);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), &quadIndices[0], GL_STATIC_DRAW);

	//generowanie tablicy wierzchołków i obiektów bufora dla kostki
	glGenVertexArrays(1, &cubeVAOID);
	glGenBuffers(1, &cubeVBOID);
	glGenBuffers(1, &cubeIndicesID);

	//wierzchołki kostki 
	glm::vec3 vertices[8]={ glm::vec3(-0.5f,-0.5f,-0.5f),
							glm::vec3( 0.5f,-0.5f,-0.5f),
							glm::vec3( 0.5f, 0.5f,-0.5f),
							glm::vec3(-0.5f, 0.5f,-0.5f),
							glm::vec3(-0.5f,-0.5f, 0.5f),
							glm::vec3( 0.5f,-0.5f, 0.5f),
							glm::vec3( 0.5f, 0.5f, 0.5f),
							glm::vec3(-0.5f, 0.5f, 0.5f)};

	//indeksy kostki
	GLushort cubeIndices[36]={0,5,4,
							  5,0,1,
							  3,7,6,
							  3,6,2,
							  7,4,6,
							  6,4,5,
							  2,1,3,
							  3,1,0,
							  3,0,7,
							  7,0,4,
							  6,5,2,
							  2,5,1};
	glBindVertexArray(cubeVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, cubeVBOID);
		//przekazanie wierzchołków kostki do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), &(vertices[0].x), GL_STATIC_DRAW);

		GL_CHECK_ERRORS

		//włączenie tablicy atrybutów wierzchołka dla położenia
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,0,0);

		//przekazanie indeksów kostki do bufora tablicy elementów
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, cubeIndicesID);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), &cubeIndices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

	//wczytanie shaderów kostki
	cubeShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/cube_shader.vert");
	cubeShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/cube_shader.frag");
	//kompilacja i konsolidacja programu shaderowego
	cubeShader.CreateAndLinkProgram();
	cubeShader.Use();
		//dodawanie atrybutów i uniformów
		cubeShader.AddAttribute("vVertex");
		cubeShader.AddUniform("MVP");
		cubeShader.AddUniform("vColor");
	cubeShader.UnUse();

	GL_CHECK_ERRORS

	//wczytanie shadera inicjalizującego
	initShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/cube_shader.vert");
	initShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/dual_init.frag");
	//kompilacja i konsolidacja programu shaderowego
	initShader.CreateAndLinkProgram();
	initShader.Use();
		//dodawanie atrybutów i uniformów
		initShader.AddAttribute("vVertex");
		initShader.AddUniform("MVP");
	initShader.UnUse();

	GL_CHECK_ERRORS

	//wczytanie shaderów dualnego peelingu
	dualPeelShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/dual_peel.vert");
	dualPeelShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/dual_peel.frag");
	//kompilacja i konsolidacja programu shaderowego
	dualPeelShader.CreateAndLinkProgram();
	dualPeelShader.Use();
		//dodawanie atrybutów i uniformów
		dualPeelShader.AddAttribute("vVertex");
		dualPeelShader.AddUniform("MVP");
		dualPeelShader.AddUniform("vColor");
		dualPeelShader.AddUniform("alpha");
		dualPeelShader.AddUniform("depthBlenderTex");
		dualPeelShader.AddUniform("frontBlenderTex");
		//ustalanie wartości stałych uniformów
		glUniform1i(dualPeelShader("depthBlenderTex"), 0);
		glUniform1i(dualPeelShader("frontBlenderTex"), 1);
	dualPeelShader.UnUse();

	GL_CHECK_ERRORS

	//wczytanie shaderów mieszania
	blendShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/blend.vert");
	blendShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/blend.frag");
	//kompilacja i konsolidacja programu shaderowego
	blendShader.CreateAndLinkProgram();
	blendShader.Use();
		//dodawanie atrybutów i uniformów
		blendShader.AddAttribute("vVertex");
		blendShader.AddUniform("tempTexture");
		//ustalanie wartości stałych uniformów
		glUniform1i(blendShader("tempTexture"), 0);
	blendShader.UnUse();

	GL_CHECK_ERRORS

	//wczytanie shadera finalnego
	finalShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/blend.vert");
	finalShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/final.frag");
	//kompilacja i konsolidacja programu shaderowego
	finalShader.CreateAndLinkProgram();
	finalShader.Use();
		//dodawanie atrybutów i uniformów
		finalShader.AddAttribute("vVertex");
		finalShader.AddUniform("depthBlenderTex");
		finalShader.AddUniform("frontBlenderTex");
		finalShader.AddUniform("backBlenderTex");
		//ustalanie wartości stałych uniformów
		glUniform1i(finalShader("depthBlenderTex"), 0);
		glUniform1i(finalShader("frontBlenderTex"), 1);
		glUniform1i(finalShader("backBlenderTex"), 2);
	finalShader.UnUse();

	GL_CHECK_ERRORS

	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {
	cubeShader.DeleteShaderProgram();
	initShader.DeleteShaderProgram();
	dualPeelShader.DeleteShaderProgram();
	blendShader.DeleteShaderProgram();
	finalShader.DeleteShaderProgram();

	shutdownFBO();
	glDeleteQueries(1, &queryId);

	glDeleteVertexArrays(1, &quadVAOID);
	glDeleteBuffers(1, &quadVBOID);
	glDeleteBuffers(1, &quadIndicesID);

	glDeleteVertexArrays(1, &cubeVAOID);
	glDeleteBuffers(1, &cubeVBOID);
	glDeleteBuffers(1, &cubeIndicesID);


	delete grid;
	cout<<"Zamkniecie powiodlo sie"<<endl;
}
//obsługa zmiany wymiarów okna
void OnResize(int w, int h) {
	//ustawienie wymiarów okna widokowego
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//wyznaczanie macierzy rzutowania
	P = glm::perspective(60.0f,(float)w/h, 0.1f,1000.0f);
}


// funkcja zwrotna bezczynności
void OnIdle() {
	//tworzenie nowej macierzy obrotu wokół osi Y
	R = glm::rotate(glm::mat4(1), glm::radians(angle+=5), glm::vec3(0,1,0));
	//wywołanie funkcji zwrotnej wyświetlania
	glutPostRedisplay();
}

//funkcja renderująca scenę przy zadanej połączonej macierzy modelu, widoku i rzutowania 
//oraz z użyciem shadera
void DrawScene(const glm::mat4& MVP, GLSLShader& shader, bool useColor=false, bool useAlphaMultiplier=false) {
	//mieszanie alfa z nakładaniem
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//wiązanie obiektu tablicy wierzchołków kostki
	glBindVertexArray(cubeVAOID);

	//wiązanie shadera
	shader.Use();

	//dla wszystkich kostek
	for(int k=-1;k<=1;k++) {
		for(int j=-1;j<=1;j++) {
			int index =0;
			for(int i=-1;i<=1;i++) {
				GL_CHECK_ERRORS
				//ustawianie uniformów shadera i przekształceń modelu
				glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(i*2,j*2,k*2));
				if(useColor)
					glUniform4fv(shader("vColor"),1, &(box_colors[index++].x));
				if(useAlphaMultiplier)
					glUniform1f(shader("alpha"), alpha);

				glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP*R*T));
				
				//rysowanie kostki
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
				GL_CHECK_ERRORS
			}
		}
	}
	//odwiązanie shadera
	shader.UnUse();
	//odwiązanie obiektu tablicy wierzchołków 
	glBindVertexArray(0);
	//diable alpha blending
	glDisable(GL_BLEND);
}

//funkcja rysująca pełnoekranowy czworokąt
void DrawFullScreenQuad() {
	//wiązanie obiektu tablicy wierzchołków czworokąta
	glBindVertexArray(quadVAOID);
	//rysowanie 2 trójkątów
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

//funkcja zwrotna wyświetlania
void OnRender() {
	GL_CHECK_ERRORS

	//zmienne transformacyjne kamery
	glm::mat4 Tr	= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(Tr,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//czyszczenie buforów koloru i głębi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//wyznaczanie połączonej macierzy modelu, widoku i rzutowania
    glm::mat4 MVP	= P*MV;
	
	//jeśli ma być użyty peeling głębi 
	if(bShowDepthPeeling) {
		
		//włączenie mieszania i wyłączenie testowania głębi
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);

		//wiązanie FBO dla dualnego peelingu
		glBindFramebuffer(GL_FRAMEBUFFER, dualDepthFBOID);

		//cele renderingu 1 i 2 przechowują kolory przodu i tyłu
		//wypełnij je zerami, a potem stosuj mieszanie MAX w celu przefiltrowania zapisanego koloru
		//w każdym przejściu może być zapisany co najwyżej jeden kolor przodu i jeden tyłu
		glDrawBuffers(2, &drawBuffers[1]);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		GL_CHECK_ERRORS

		//cel renderingu nr 0 przechowuje (-minDepth, maxDepth)
		glDrawBuffer(drawBuffers[0]);
		//wypełnianie pozaekranowej tekstury wartością -MAX_DEPTH
		glClearColor(-MAX_DEPTH, -MAX_DEPTH, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		//mieszanie typu max
		glBlendEquation(GL_MAX);
		//renderowanie sceny przy użyciu shadera inicjalizującego
		DrawScene(MVP, initShader);

		//2.  //Piling głębi + przebieg mieszający
		glDrawBuffer(drawBuffers[6]);
		//wypełnianie bufora koloru kolorem tła
		glClearColor(bg.x, bg.y, bg.z, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		 
		int currId = 0;
		//dla każdego przebiegu
		for (int layer = 1; bUseOQ || layer < NUM_PASSES; layer++) {
			currId = layer % 2;
			int prevId = 1 - currId;
			int bufId = currId * 3;

			//renderowanie do 2 przyłączy koloru jednocześnie
			glDrawBuffers(2, &drawBuffers[bufId+1]);
			//ustawienie czarnego koloru czyszczącego i czyszczenie bufora koloru
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);

			//zmiana przyłącza koloru dla bufora rysowania
			glDrawBuffer(drawBuffers[bufId+0]);
			//ustawienie koloru czyszczącego na -MAX_DEPTH i czyszczenie bufora koloru
			glClearColor(-MAX_DEPTH, -MAX_DEPTH, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);

			//Render to three draw buffers simultaneously
			// Render target 0: RG32F MAX blending
			// Render target 1: RGBA MAX blending
			//Cel renderingu nr 2 RGBA MAX blending
			glDrawBuffers(3, &drawBuffers[bufId+0]);	
			//mieszanie typu max
			glBlendEquation(GL_MAX);

			//przyłączanie tekstury głębi do jednostki 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_RECTANGLE, depthTexID[prevId]);

			//przyłączanie tekstury koloru do jednostki 1
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_RECTANGLE, texID[prevId]);
			
			//rysowanie sceny przy użyciu shadera dualnego peelingu 
			DrawScene(MVP, dualPeelShader, true,true);

			//pełnoekranowe przejście z mieszaniem alfa koloru czarnego
			glDrawBuffer(drawBuffers[6]);

			//mieszanie nakładkowe 
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			 
			//tu można włączyć kwerendę
			if (bUseOQ) {
				glBeginQuery(GL_SAMPLES_PASSED_ARB, queryId);
			}

			GL_CHECK_ERRORS

			//przyłączanie tylnej tekstury koloru do jednostki 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_RECTANGLE, backTexID[currId]);

			//uruchamianie shadera mieszającego i rysowanie pełnoekranowego czworokąta
			blendShader.Use();
				 DrawFullScreenQuad();
			blendShader.UnUse();

			//tu należy wyłączyć kwerendę
			//i odczytać jej wynik czyli całkowitą liczbę próbek
			//będących wynikiem mieszania
			if (bUseOQ) {
				glEndQuery(GL_SAMPLES_PASSED);
				GLuint sample_count;
				glGetQueryObjectuiv(queryId, GL_QUERY_RESULT, &sample_count);
				if (sample_count == 0) {
					break;
				}
			}
			GL_CHECK_ERRORS
		}

		GL_CHECK_ERRORS

		//wyłączenie mieszania alfa
		glDisable(GL_BLEND);

		//3.  Finalny przebieg renderingu
		//usunięcie FBO 
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//przywrócenie domyślnego bufora ekranu
		glDrawBuffer(GL_BACK_LEFT);
		 
		//przyłączanie tekstury głębi do jednostki 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_RECTANGLE, depthTexID[currId]);

		//przyłączanie tekstury głębi do jednostki 1
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_RECTANGLE, texID[currId]);
		
		//przyłączanie tekstury mieszania kolorów do jednostki 2
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_RECTANGLE, colorBlenderTexID);

		//uruchamianie shadera finalnego i rysowanie pełnoekranowego czworokąta
		finalShader.Use();
			DrawFullScreenQuad();
		finalShader.UnUse();

	} else {
		//renderowanie sceny ze zwykłym mieszaniem alfa, bez peelingu głębi
		glEnable(GL_DEPTH_TEST);
		DrawScene(MVP, cubeShader, true,false);
	}

	//renderowanie siatki
	grid->Render(glm::value_ptr(MVP));

	//zamiana buforów w celu wyświetlenia wyrenderowanego obrazu
	glutSwapBuffers();
}

//obsługa klawiatury w celu przełączania dualnego peelingu głębi
void OnKey(unsigned char key, int x, int y) {
	switch(key) {
		case ' ':
		bShowDepthPeeling = !bShowDepthPeeling;
			break;
	}
	if(bShowDepthPeeling)
		glutSetWindowTitle("Dualny peeling włączony");
	else
		glutSetWindowTitle("Dualny peeling wyłączony");
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	//inicjalizacja freeglut 
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Dualny peeling głębi - OpenGL 3.3");

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
	glutKeyboardFunc(OnKey);
	glutIdleFunc(OnIdle);

	//wywołanie pętli głównej
	glutMainLoop();

	return 0;
}
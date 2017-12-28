
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/freeglut.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include "../src/GLSLShader.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cassert>

using namespace std;
#pragma comment(lib, "glew32.lib")

#define CHECK_GL_ERRORS assert(glGetError()==GL_NO_ERROR);

const int width = 1024, height = 1024;

//maksymalna liczba cząsteczek w symulacji
const int TOTAL_PARTICLES = 10000;

//liczba iteracji transformacyjnego sprzężenia zwrotnego
const int NUM_ITER = 1;

//zmienne transformacyjne kamery
int oldX=0, oldY=0;
float rX=10, rY=0;
int state =1 ;
float dist=-11;

//rozmiar siatki
const int GRID_SIZE=10;

//komunikat informacyjny
char info[MAX_PATH]={0};

//zmienne związane z obliczaniem czasu
float timeStep =  1.0f/60.0f;
float currentTime = 0;
double accumulator = timeStep;
 
glm::mat4 mMVP;	//połączona macierz modelu, widoku i rzutowania
glm::mat4 mMV;	//macierz modelu i widoku
glm::mat4 mP;	//macierz rzutowania
 

//zmienne związane z tempem animacji
LARGE_INTEGER frequency;        //liczba tyknięć w ciągu sekundy
LARGE_INTEGER t1, t2;           //tyknięcia
double frameTimeQP=0;
float frameTime =0 ;

//wyznaczanie liczby klatek na sekundę (FPS)
float startTime =0, fps=0 ;
int totalFrames=0;

//do kwerendy sprzężenia zwrotnego
GLuint primitives_written=0;

//rozmiar cząstki
GLfloat pointSize = 10;

//stałe kolory
GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
GLfloat vBeige[] = { 1.0f, 0.8f, 0.7f, 1.0f };
GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat vGray[] = { .25f, .25f, .25f, 1.0f };

//przyrost czasu
float delta_time=0;

//identyfikatory ścieżek odczytu i zapisu
int readID=0, writeID = 1;

//identyfikatory obiektów buforowych dla położeń bieżącego i poprzedniego
GLuint vboID_Pos[2],
		vboID_PrePos[2];

//identyfikatory obiektów tablic wierzchołków dla cykli aktualizacyjnych i renderujących 
GLuint vaoUpdateID[2], vaoRenderID[2];
GLuint vboID_Direction[2];
size_t i=0;

//shadery cząsteczkowy, przejściowy i renderingu
GLSLShader particleShader, passShader,
			renderShader;

//identyfikatory kwerend czasowych
GLuint t_query, query;

//czas trwania klatki
GLuint64 elapsed_time;

//zmienne renderowania siatki
GLuint gridVAOID, gridVBOVerticesID, gridVBOIndicesID;
vector<glm::vec3> grid_vertices;
vector<GLushort> grid_indices;

//identyfikator transformacyjnego sprzężenia zwrotnego
GLuint tfID;

//czas bieżący
float t=0; 

const int HALF_RAND = (RAND_MAX / 2);

//zmienne związane z wyznaczaniem życia cząsteczki
int max_life = 60;
int emitterLife = max_life;
int   emitterLifeVar = 15;

//przeliczanie stopni na radiany
float DEGTORAD(const float f) {
	return f*(float)M_PI/180.0f;
}

//parametry emitera
float emitterYaw = DEGTORAD(0.0f);
float emitterYawVar	= DEGTORAD(360.0f);
float emitterPitch	= DEGTORAD(90.0f);
float emitterPitchVar = DEGTORAD(40.0f);
float emitterSpeed = 0.05f;
float emitterSpeedVar = 0.01f;

//generator liczb pseudolosowych
float RandomNum()
{
	int rn;
	rn = rand();
	return ((float)(rn - HALF_RAND) / (float)HALF_RAND);
}

//zamiana wartości pochylenia i odchylenia na jednostkowy wektor kierunku 
void RotationToDirection(float pitch,float yaw,glm::vec3 *direction)
{
	direction->x = -sin(yaw) * cos(pitch);
	direction->y = sin(pitch);
	direction->z = cos(pitch) * cos(yaw);
}

//creates buffer objects for particles
void createVBO()
{
	//wypełnij wierzchołki
	int count = 0;

    //utwórz obiekty tablic wierzchołków 
	glGenVertexArrays(2, vaoUpdateID);
	glGenVertexArrays(2, vaoRenderID);

	//utwórz obiekty buforowe
	glGenBuffers( 2, vboID_Pos);
	glGenBuffers( 2, vboID_PrePos);
  	glGenBuffers( 2, vboID_Direction);

	//ustaw aktualizacyjne VAO
	for(int i=0;i<2;i++) {
		glBindVertexArray(vaoUpdateID[i]);
		//przekaż bieżące położenia 
		glBindBuffer( GL_ARRAY_BUFFER, vboID_Pos[i]);
		glBufferData( GL_ARRAY_BUFFER, TOTAL_PARTICLES* sizeof(glm::vec4), 0, GL_DYNAMIC_COPY);

		//xyz -> położenie
		//w-> prędkość
		//włącz tablicę atrybutów wierzchołka 
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,  4, GL_FLOAT, GL_FALSE, 0, 0);

		CHECK_GL_ERRORS

		//przekaż poprzednie położenia 
		glBindBuffer( GL_ARRAY_BUFFER, vboID_PrePos[i]);
		glBufferData( GL_ARRAY_BUFFER, TOTAL_PARTICLES*sizeof(glm::vec4), 0, GL_DYNAMIC_COPY);

		//xyz -> położenie poprzednie
		//w-> życie
		//włącz tablicę atrybutów wierzchołka 
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1,  4, GL_FLOAT, GL_FALSE, 0,0);

		CHECK_GL_ERRORS;

		//xyz-> kierunek początkowy
		//w-> życie
		//przekaż początkowe kierunki cząsteczek 
		glBindBuffer( GL_ARRAY_BUFFER, vboID_Direction[i]);
		glBufferData( GL_ARRAY_BUFFER, TOTAL_PARTICLES*sizeof(glm::vec4), 0, GL_DYNAMIC_COPY);

		//włącz tablicę atrybutów wierzchołka 
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2,  4, GL_FLOAT, GL_FALSE, 0,0);

	}

	CHECK_GL_ERRORS;

	//ustaw VAO renderingowy, aby używał wypełnionego wcześniej obiektu bufora położeń
	for(int i=0;i<2;i++) {
		glBindVertexArray(vaoRenderID[i]);
		glBindBuffer( GL_ARRAY_BUFFER, vboID_Pos[i]);
		//włącz tablicę atrybutów wierzchołka 
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,  4, GL_FLOAT, GL_FALSE, 0, 0);
	}

	glBindVertexArray(0);

	//ustawienie wierzchołków siatki
	for(int i=-GRID_SIZE;i<=GRID_SIZE;i++)
	{
		grid_vertices.push_back(glm::vec3((float)i,0,(float)-GRID_SIZE));
		grid_vertices.push_back(glm::vec3((float)i,0,(float)GRID_SIZE));

		grid_vertices.push_back(glm::vec3((float)-GRID_SIZE,0,(float)i));
		grid_vertices.push_back(glm::vec3((float)GRID_SIZE,0,(float)i));
	}

	//wypełnij indeksy siatki
	for(int i=0;i<GRID_SIZE*GRID_SIZE;i+=4) {
		grid_indices.push_back(i);
		grid_indices.push_back(i+1);
		grid_indices.push_back(i+2);
		grid_indices.push_back(i+3);
	}

	//utwórz VAO i VBO dla siatki
	glGenVertexArrays(1, &gridVAOID);
	glGenBuffers (1, &gridVBOVerticesID);
	glGenBuffers (1, &gridVBOIndicesID);
	glBindVertexArray(gridVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, gridVBOVerticesID);
		//przekazanie wierzchołków siatki do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, sizeof(float)*3*grid_vertices.size(), &grid_vertices[0].x, GL_STATIC_DRAW);
		//włącz tablicę atrybutów wierzchołka 
		glEnableVertexAttribArray(0);
		glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE,0,0);

		CHECK_GL_ERRORS
		//przekazanie indeksów siatki do obiektu bufora tablicy elementów
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridVBOIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*grid_indices.size(), &grid_indices[0], GL_STATIC_DRAW);


	glBindVertexArray(0);

    CHECK_GL_ERRORS;
}


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

//procedura renderowania siatki 
//używa shadera przejściowego, który przypisuje fragmentowi kolor zapisany w uniformie  
void DrawGrid()
{
	passShader.Use();
		glUniform4fv(passShader("vColor"),1, vGray);
		glBindVertexArray(gridVAOID);
		glUniformMatrix4fv(passShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
			glDrawElements(GL_LINES, grid_indices.size(),GL_UNSIGNED_SHORT,0);
		glBindVertexArray(0);
	passShader.UnUse();
}

//procedura renderowania cząsteczek
//angażuje shader renderingowy, który przypisuje fragmentowi kolor otrzymany z shadera wierzchołków
void RenderParticles()
{
	glBindVertexArray(vaoRenderID[readID]);
	renderShader.Use();
		glUniformMatrix4fv(renderShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
			glDrawArrays(GL_POINTS, 0, TOTAL_PARTICLES);
	renderShader.UnUse();
	glBindVertexArray(0);
}

void InitGL() {
	//generowanie kwerend sprzętowych
	glGenQueries(1, &query);
	glGenQueries(1, &t_query);

	CHECK_GL_ERRORS
	//pobierz czas początkowy
	startTime = (float)glutGet(GLUT_ELAPSED_TIME);
	//pobierz częstotliwość tyknięć
    QueryPerformanceFrequency(&frequency);

    //uruchom stoper
    QueryPerformanceCounter(&t1);

	//włącz mieszanie alfa z nakładaniem
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//wczytywanie shaderów
	particleShader.LoadFromFile(GL_VERTEX_SHADER,"shadery/Particle.vert");

	renderShader.LoadFromFile(GL_VERTEX_SHADER,"shadery/Render.vert");
	renderShader.LoadFromFile(GL_FRAGMENT_SHADER,"shadery/Render.frag");

	passShader.LoadFromFile(GL_VERTEX_SHADER,"shadery/Passthrough.vert");
	passShader.LoadFromFile(GL_FRAGMENT_SHADER,"shadery/Passthrough.frag");

	//kompilacja i konsolidacja programu shaderowego dla cząstek
	particleShader.CreateAndLinkProgram();
	particleShader.Use();
		//dodawanie atrybutów i uniformów
		particleShader.AddAttribute("position");
		particleShader.AddAttribute("prev_position");
		particleShader.AddAttribute("direction");
		particleShader.AddUniform("MVP");
		particleShader.AddUniform("time");
	particleShader.UnUse();

	//kompilacja i konsolidacja programu shaderowego renderShader
	renderShader.CreateAndLinkProgram();
	renderShader.Use();
		//dodawanie atrybutów i uniformów
		renderShader.AddAttribute("position");
		renderShader.AddUniform("MVP");
	renderShader.UnUse();

	CHECK_GL_ERRORS

	//kompilacja i konsolidacja programu shaderowego przejściowego
	passShader.CreateAndLinkProgram();
	passShader.Use();
		//dodawanie atrybutów i uniformów
		passShader.AddAttribute("position");
		passShader.AddUniform("MVP");
		passShader.AddUniform("vColor");
	passShader.UnUse();

	CHECK_GL_ERRORS

	//utwórz VBO
	createVBO();

	//ustaw atrybuty transformacyjnego sprzężenia zwrotnego
	glGenTransformFeedbacks(1, &tfID);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfID);
	//przekaż wyjścia shadera wierzchołków do sprzężenia zwrotnego
	const char* varying_names[]={"out_position", "out_prev_position", "out_direction"};
	glTransformFeedbackVaryings(particleShader.GetProgram(), 3, varying_names, GL_SEPARATE_ATTRIBS);
	//ponownie skonsoliduj program shadera cząsteczkowego
	glLinkProgram(particleShader.GetProgram());

	//ustaw rozmiar cząsteczki
	glPointSize(pointSize); 
}

//obsługa zmiany wymiarów okna
void OnReshape(int nw, int nh) {
	//przywrócenie domyślnych wymiarów okna widokowego
	glViewport(0,0,nw, nh);

	//ustaw macierz rzutowania z perspektywą
	mP = glm::perspective(60.0f, (GLfloat)nw/nh, 1.0f, 100.f);
  
}

//aktualizowanie położenia cząsteczki
void UpdateParticlesGPU() {
	//pobieranie czasu, jaki upłynął
	t = (float)glutGet(GLUT_ELAPSED_TIME)/1000.0f;
	
	//włącz shader cząsteczek
	particleShader.Use();
		//ustawianie uniformów shadera
		glUniformMatrix4fv(particleShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
		glUniform1f(particleShader("time"), t);
		CHECK_GL_ERRORS
		//uruchom pętlę symulacyjną
		for(int i=0;i<NUM_ITER;i++) {
			//zwiąż aktualizacyjny obiekt tablicy wierzchołków 
			glBindVertexArray( vaoUpdateID[readID]);
				//zwiąż bufory transformacyjnego sprzężenia zwrotnego
				//indeks 0 -> położenie bieżące
				//indeks 1 -> położenie poprzednie
				//indeks 2 -> kierunek ruchu
				glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vboID_Pos[writeID]);
				glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, vboID_PrePos[writeID]);
				glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, vboID_Direction[writeID]);
				//wyłącz rasteryzację
				glEnable(GL_RASTERIZER_DISCARD);    
					//uruchom sprzętową kwerendę czasową
					glBeginQuery(GL_TIME_ELAPSED,t_query);
						//zainicjuj transformacyjne sprzężenie zwrotne
						glBeginTransformFeedback(GL_POINTS);
							//wyrenderuj punkty, co spowoduje przekazanie wszystkich atrybutów do GPU
							glDrawArrays(GL_POINTS, 0, TOTAL_PARTICLES);
						//zakończ transformacyjne sprzężenie zwrotne
						glEndTransformFeedback();
					//zakończ kwerendę czasową
					glEndQuery(GL_TIME_ELAPSED);
					glFlush();
				//włącz rasteryzację
				glDisable(GL_RASTERIZER_DISCARD);

			//przełącz ścieżki odczytu i zapisu
			int tmp = readID;
			readID  = writeID;
			writeID = tmp;
		}
		CHECK_GL_ERRORS
		//pobierz wynik kwerendy
		glGetQueryObjectui64v(t_query, GL_QUERY_RESULT, &elapsed_time);
		//odczytaj czas trwania transformacyjnego sprzężenia zwrotnego
		delta_time = elapsed_time / 1000000.0f;
	//wyłącz shader cząsteczek
	particleShader.UnUse();

	CHECK_GL_ERRORS
}

//funkcja wyświetlania
void OnRender() {

	CHECK_GL_ERRORS

	//wywołania funkcji związanych obliczaniem czasu
	float newTime = (float) glutGet(GLUT_ELAPSED_TIME);
	frameTime = newTime-currentTime;
	currentTime = newTime; 

	//licznik o dużej precyzji
    QueryPerformanceCounter(&t2);
	 // wyznacz i wyświetl czas w milisekundach
    frameTimeQP = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
	t1=t2;
	accumulator += frameTimeQP;

	++totalFrames;

	//wyznaczanie liczby klatek na sekundę (FPS)
	if((newTime-startTime)>1000)
	{
		float elapsedTime = (newTime-startTime);
		fps = (totalFrames/ elapsedTime)*1000 ;
		startTime = newTime;
		totalFrames=0;
		sprintf_s(info, "FPS: %3.2f, Frame time (GLUT): %3.4f msecs, Frame time (QP): %3.3f, TF Time: %3.3f", fps , frameTime, frameTimeQP, delta_time);
	}
	
	glutSetWindowTitle(info);

	//czyszczenie buforów koloru i głębi
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

	//transformacje kamery
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	mMV	= glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	mMVP = mP*mMV;
	 
	//rysuj siatkę
 	DrawGrid();

	//aktualizowanie cząsteczek na GPU
	UpdateParticlesGPU();

	//renderowanie cząsteczek
	RenderParticles();

	//zamiana buforów w celu wyświetlenia wyrenderowanego obrazu
	glutSwapBuffers();
}

//usuwanie wszystkich obiektów
void OnShutdown() {

	glDeleteQueries(1, &query);
	glDeleteQueries(1, &t_query);

	glDeleteVertexArrays(2, vaoUpdateID);
	glDeleteVertexArrays(2, vaoRenderID);

	glDeleteVertexArrays(1, &gridVAOID);
	glDeleteBuffers( 1, &gridVBOVerticesID);
	glDeleteBuffers( 1, &gridVBOIndicesID);

    glDeleteBuffers( 2, vboID_Pos);
	glDeleteBuffers( 2, vboID_PrePos);
	glDeleteBuffers( 2, vboID_Direction);

	glDeleteTransformFeedbacks(1, &tfID);
	renderShader.DeleteShaderProgram();
	particleShader.DeleteShaderProgram();
	passShader.DeleteShaderProgram();

	printf("Shutdown successful.");
}

void OnIdle() {
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	//inicjalizacja freeglut 
	glutInit(&argc, argv);
	glutInitContextVersion(3,3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow("Prosty system cząsteczkowy z transformacyjnym sprzężeniem zwrotnym");

	//rejestracja funkcji zwrotnych
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnReshape);
	glutIdleFunc(OnIdle);

	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutCloseFunc(OnShutdown);

	//inicjalizacja glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
	}

	GLuint error = glGetError();

	//kontynuuj pod warunkiem, że obsługiwana jest wersja 3.3 biblioteki OpenGL
	if (!glewIsSupported("GL_VERSION_3_3"))
	{
  		puts("OpenGL 3.3 not supported.");
		exit(EXIT_FAILURE);
	} else {
		puts("OpenGL 3.3 supported.");
	}
	if(!glewIsExtensionSupported("GL_ARB_transform_feedback2"))
	{
		puts("Your hardware does not support a required extension [GL_ARB_transform_feedback2].");
		exit(EXIT_FAILURE);
	} else {
		puts("GL_ARB_transform_feedback2 supported.");
	}
	//wypisz informacje na ekranie
	printf("Using GLEW %s\n",glewGetString(GLEW_VERSION));
	printf("Vendor: %s\n",glGetString (GL_VENDOR));
	printf("Renderer: %s\n",glGetString (GL_RENDERER));
	printf("Version: %s\n",glGetString (GL_VERSION));
	printf("GLSL: %s\n",glGetString (GL_SHADING_LANGUAGE_VERSION));

	//inicjalizacja OpenGL
	InitGL();

	//wywołanie pętli głównej
	glutMainLoop();

	return 0;
}


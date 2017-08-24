
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/freeglut.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include "..\src\GLSLShader.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cassert> 


using namespace std;
#pragma comment(lib, "glew32.lib")

#define CHECK_GL_ERRORS assert(glGetError()==GL_NO_ERROR);

//wymiary okna
const int width = 1024, height = 1024;

//liczba punktów wzdłuż osi X i Y
int numX = 21, numY=21;
const int total_points = (numX+1)*(numY+1);

//wymiary tkaniny w przestrzeni swiata
int sizeX = 4,
	sizeY = 4;
float hsize = sizeX/2.0f;

//liczba iteracji transformacyjnego sprzężenia zwrotnego
const int NUM_ITER = 10;

//indeks wybranego wierzchołka
int selected_index = -1;

//znacznik włączający i wyłączający wyświetlanie mas
bool bDisplayMasses=true;

//struktura do przechowywania sił sprężystości tkaniny
struct Spring {
	int p1, p2;			//indeksy dwóch wierzchołków
	float rest_length;	//długość spoczynkowa
	float Ks, Kd;		//stałymi sprężystości i tłumienia
};

//indeksy tkaniny
vector<GLushort> indices;

//siły sprężystości tkaniny
vector<Spring> springs;

vector<glm::vec4> X;		//bieżące położenie wierzchołka tkaniny
vector<glm::vec4> X_last;	//poprzednie położenie wierzchołka tkaniny
vector<glm::vec3> F;		//siła działająca na wierzchołek

//zmienne transformacyjne kamery
int oldX=0, oldY=0;
float rX=10, rY=-45;
int state =1 ;
float dist=-4;

//rozmiar siatki
const int GRID_SIZE=10;

//całkowita liczba sił sprężystosci
int spring_count=0;

//komunikat informacyjny
char info[MAX_PATH]={0};

//domyślne wartości sił sprężystości
const float DEFAULT_DAMPING =  -0.05f;
float	KsStruct = 10.5f,KdStruct = -10.5f;
float	KsShear = 0.25f,KdShear =-0.25f;
float	KsBend = 0.25f,KdBend = -0.25f;

//domyślna grawitacja i masa 
glm::vec3 gravity=glm::vec3(0.0f,-0.00981f,0.0f);
float mass = 1.0f;

//domyślny przyrost czasu i inne zmienne związane z czasem
float timeStep =  1.0f/60.0f;
float currentTime = 0;
double accumulator = timeStep;

//bieżące okno widokowe i macierze rzutowania oraz modelu i widoku
//required for picking
GLint viewport[4];
GLdouble MV[16];
GLdouble P[16];

glm::mat4 mMVP;		//połączona macierz modelu, widoku i rzutowania
glm::mat4 mMV;		//macierz modelu i widoku
glm::mat4 mP;		//macierz rzutowania

//wektory wyznaczające kierunki w górę, w prawo i w przód dla kamery
glm::vec3 Up=glm::vec3(0,1,0), Right, viewDir;

//zmienne związane z tempem animacji
LARGE_INTEGER frequency;        //liczba tyknięć w ciągu sekundy
LARGE_INTEGER t1, t2;           //tyknięcia
double frameTimeQP=0;
float frameTime =0 ;
int texture_size_x=0;
int texture_size_y=0;

//wyznaczanie liczby klatek na sekundę (FPS)
float startTime =0, fps=0 ;
int totalFrames=0;

//do kwerendy sprzężenia zwrotnego
GLuint primitives_written=0;

//rozmiar cząstki
GLfloat pointSize = 30;

//stałe kolory
GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
GLfloat vBeige[] = { 1.0f, 0.8f, 0.7f, 1.0f };
GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat vGray[] = { .25f, .25f, .25f, 1.0f };

//przyrost czasu
float delta_time=0;

//identyfikatory pścieżek odczytu i zapisu
int readID=0, writeID = 1;

//identyfikatory obiektów buforowych dla położeń bieżącego i poprzedniego
GLuint vboID_Pos[2],
		vboID_PrePos[2];

//identyfikatory obiektów tablic wierzchołków dla cykli aktualizacyjnych i renderujących 
GLuint vaoUpdateID[2], vaoRenderID[2], vboIndices;

//identyfikatory buforów teksturowych
GLuint texPosID[2];
GLuint texPrePosID[2];

size_t i=0;

//shadery wierzchołków tkaniny, cząsteczek i renderingu
GLSLShader massSpringShader,
			particleShader,
			renderShader;

//identyfikatory kwerend czasowych
GLuint t_query, query;

//czas trwania klatki
GLuint64 elapsed_time;

//zmienne renderowania siatki
GLuint gridVAOID, gridVBOVerticesID, gridVBOIndicesID;
vector<glm::vec3> grid_vertices;
vector<GLushort> grid_indices;

//obiekty tablicy i bufora wierzchołków tkaniny
GLuint clothVAOID, clothVBOVerticesID, clothVBOIndicesID;

//obiekty tablicy i bufora wierzchołków sfery
GLuint sphereVAOID, sphereVerticesID, sphereIndicesID;

//identyfikator transformacyjnego sprzężenia zwrotnego
GLuint tfID;

//macierz transformacji elipsoidy i jej odwrotność 
glm::mat4 ellipsoid, inverse_ellipsoid;

//promień, równoleżniki i południki sfery
int iStacks = 30;
int iSlices = 30;
float fRadius = 1;

//liczba indeksów sfery
int total_sphere_indices=0;

//konwersja vec4 na vec3
glm::vec3 vec3(glm::vec4 v) {
	return glm::vec3(v.x, v.y, v.z);
}

// tworzy nową siłę sprężystości pomiędzy danymi wierzchołkami (a i b) z zadanymi 
//stałymi sprężystości i tłumienia
void AddSpring(int a, int b, float ks, float kd ) {
	Spring spring;
	spring.p1=a;
	spring.p2=b;
	spring.Ks=ks;
	spring.Kd=kd;
	glm::vec3 deltaP = vec3(X[a]-X[b]);
	spring.rest_length = sqrt(glm::dot(deltaP, deltaP));
	springs.push_back(spring);
}

//tworzy obiekty bufora dla tkaniny, siatki i sfery
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
	glGenBuffers(1, &vboIndices);
	glGenTextures(2, texPosID);
	glGenTextures(2, texPrePosID);

	//ustaw aktualizacyjne VAO
	for(int i=0;i<2;i++) {
		glBindVertexArray(vaoUpdateID[i]);
		//przekaż bieżące położenia 
		glBindBuffer( GL_ARRAY_BUFFER, vboID_Pos[i]);
		glBufferData( GL_ARRAY_BUFFER, X.size()* sizeof(glm::vec4), &(X[0].x), GL_DYNAMIC_COPY);
		//włącz tablicę atrybutów wierzchołka 
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,  4, GL_FLOAT, GL_FALSE, 0, 0);

		CHECK_GL_ERRORS

		//przekaż poprzednie położenia 
		glBindBuffer( GL_ARRAY_BUFFER, vboID_PrePos[i]);
		glBufferData( GL_ARRAY_BUFFER, X_last.size()*sizeof(glm::vec4), &(X_last[0].x), GL_DYNAMIC_COPY);
		//włącz tablicę atrybutów wierzchołka 
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1,  4, GL_FLOAT, GL_FALSE, 0,0);

		CHECK_GL_ERRORS;
	}

	CHECK_GL_ERRORS;

	//ustaw VAO renderingowy, aby używał wypełnionego wcześniej obiektu bufora położeń
	for(int i=0;i<2;i++) {
		glBindVertexArray(vaoRenderID[i]);
		glBindBuffer( GL_ARRAY_BUFFER, vboID_Pos[i]);
		//włącz tablicę atrybutów wierzchołka 
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,  4, GL_FLOAT, GL_FALSE, 0, 0);

		//ustaw bufor tablicy indeksów
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
		if(i==0)
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);
	}

	glBindVertexArray(0);

	//ustaw dwa bufory teksturowe pobierające wartości z  
	//obiektów buforowych dla położeń bieżącego i poprzedniego
	for(int i=0;i<2;i++) {
		glBindTexture( GL_TEXTURE_BUFFER, texPosID[i]);
		glTexBuffer( GL_TEXTURE_BUFFER, GL_RGBA32F, vboID_Pos[i]);

		glBindTexture( GL_TEXTURE_BUFFER, texPrePosID[i]);
		glTexBuffer( GL_TEXTURE_BUFFER, GL_RGBA32F, vboID_PrePos[i]);
	}
	 
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

	//utwórz obiekty tablicy i bufora wierzchołków tkaniny
	glGenVertexArrays(1, &clothVAOID);
	glGenBuffers (1, &clothVBOVerticesID);
	glGenBuffers (1, &clothVBOIndicesID);
	glBindVertexArray(clothVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, clothVBOVerticesID);
		//przekaż położenia wierzchołków tkaniny 
		glBufferData (GL_ARRAY_BUFFER, sizeof(float)*4*X.size(), &X[0].x, GL_STATIC_DRAW);
		//włącz tablicę atrybutów wierzchołka 
		glEnableVertexAttribArray(0);
		glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE,0,0);

		CHECK_GL_ERRORS
		//przekaż indeksy tkaniny do bufora tablicy elementów 
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, clothVBOIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*indices.size(), &indices[0], GL_STATIC_DRAW);

	//wierzchołki i indeksy sfery
	vector<glm::vec4> sphere_vertices;
	vector<GLushort>  sphere_indices;

	//ten fragment kodu generuje zestaw wierzchołków na sferze
	//a także ich triangulację 

	GLfloat drho = (GLfloat)(M_PI) / (GLfloat) iStacks;
    GLfloat dtheta = 2.0f * (GLfloat)(M_PI) / (GLfloat) iSlices;
	GLfloat ds = 1.0f / (GLfloat) iSlices;
	GLfloat dt = 1.0f / (GLfloat) iStacks;
    GLint i, j;     //zmienne do sterowania pętlą
    total_sphere_indices = iSlices * iStacks * 6;
	count=0;
	 
	for (i = 0; i < iStacks; i++)
	{
		GLfloat rho = (GLfloat)i * drho;
		GLfloat srho = (GLfloat)(sin(rho));
		GLfloat crho = (GLfloat)(cos(rho));
		GLfloat srhodrho = (GLfloat)(sin(rho + drho));
		GLfloat crhodrho = (GLfloat)(cos(rho + drho));

        for ( j = 0; j < iSlices; j++)
		{
			GLfloat theta = (j == iSlices) ? 0.0f : j * dtheta;
			GLfloat stheta = (GLfloat)(-sin(theta));
			GLfloat ctheta = (GLfloat)(cos(theta));

			GLfloat x = stheta * srho;
			GLfloat y = ctheta * srho;
			GLfloat z = crho;
			//zapisz wierzchołki sfery
			sphere_vertices.push_back(glm::vec4(x,y,z,1) * fRadius);

            x = stheta * srhodrho;
			y = ctheta * srhodrho;
			z = crhodrho;
			//zapisz wierzchołki sfery
			sphere_vertices.push_back(glm::vec4(x,y,z,1) * fRadius);

			theta = ((j+1) == iSlices) ? 0.0f : (j+1) * dtheta;
			stheta = (GLfloat)(-sin(theta));
			ctheta = (GLfloat)(cos(theta));

			x = stheta * srho;
			y = ctheta * srho;
			z = crho;
			//zapisz wierzchołki sfery
			sphere_vertices.push_back(glm::vec4(x,y,z,1) * fRadius);

            x = stheta * srhodrho;
			y = ctheta * srhodrho;
			z = crhodrho;

			//zapisz wierzchołki sfery
			sphere_vertices.push_back(glm::vec4(x,y,z,1) * fRadius);

			//zapisz indeksy sfery
			sphere_indices.push_back(count);
			sphere_indices.push_back(count+1);
			sphere_indices.push_back(count+2);
			sphere_indices.push_back(count+1);
			sphere_indices.push_back(count+3);
			sphere_indices.push_back(count+2);
			count+=4;
		}
	}


	//obiekty tablicy i bufora wierzchołków sfery
	glGenVertexArrays(1, &sphereVAOID);
	glGenBuffers (1, &sphereVerticesID);
	glGenBuffers (1, &sphereIndicesID);
	glBindVertexArray(sphereVAOID);
		glBindBuffer (GL_ARRAY_BUFFER, sphereVerticesID);
		//pass sphere vertices into buffer object
		glBufferData (GL_ARRAY_BUFFER, sizeof(float)*4*sphere_vertices.size(), &sphere_vertices[0].x, GL_STATIC_DRAW);
		//włącz tablicę atrybutów wierzchołka 
		glEnableVertexAttribArray(0);
		glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE,0,0);

		CHECK_GL_ERRORS
		//przekaż indeksy sfery do obiektu tablicy elementów
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*total_sphere_indices, &sphere_indices[0], GL_STATIC_DRAW);


	glBindVertexArray(0);

    CHECK_GL_ERRORS;
}

void OnMouseDown(int button, int s, int x, int y)
{
	if (s == GLUT_DOWN)
	{
		oldX = x;
		oldY = y;
		//przekaż położenie punktu, w którym kliknięto na ekranie
		//zwróć uwagę na odwrócenie wartości y
		int winY = (height - y); 
		int winX = x ; 
		float winZ =0;

		//odczytaj głębię w miejscu kliknięcia
		glReadPixels( winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
		if(winZ==1)
			winZ=0;

		//zastosuj rzutowanie wsteczne, wykorzystując bieżące okno widokowe i macierze rzutowania oraz modelu i widoku 
		//w celu wyznaczania położenia klikniętego punktu w przestrzeni obiektu
		double objX=0, objY=0, objZ=0;
		gluUnProject(winX, winY, winZ,  MV,  P, viewport, &objX, &objY, &objZ);
	
		glm::vec3 pt(objX,objY, objZ);
	 
		//zwiąż renderingowy obiekt tablicy wierzchołków 
		glBindVertexArray(vaoRenderID[readID]);
		glBindBuffer(GL_ARRAY_BUFFER, vboID_Pos[writeID]);
		//wykonaj takie mapowanie danych, które pozwoli na odczytanie położenia każdego wierzchołka
		//bo będzie potrzebny dostęp do wierzchołków sąsiadujących ze wskazanym przez użytkownika
		GLfloat* pData = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

		//pętla po wszystkich wierzchołkach tkaniny
		for(int i=0;i<total_points*4;i+=4) {
			//jeśli wierzchołki są dostatecznie blisko 
			if( abs(pData[i]-pt.x)<0.1 &&
				abs(pData[i+1]-pt.y)<0.1  &&
				abs(pData[i+2]-pt.z)<0.1 ) {
				//uznaj je za wskazane
				selected_index = i/4;
				printf("Intersected at %d\n",i);
				break;
			}
		}
		//zlikwiduj mapowanie bufora, aby zwolnić uzyskany wskaźnik do pamięci GPU
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindVertexArray(0);

	}

	if(button == GLUT_MIDDLE_BUTTON)
		state = 0;
	else
		state = 1;

	if(s==GLUT_UP) {
		selected_index= -1;
		glutSetCursor(GLUT_CURSOR_INHERIT);
	}
}

void OnMouseMove(int x, int y)
{
	if(selected_index == -1) {
		if (state == 0)
			dist *= (1 + (y - oldY)/60.0f);
		else
		{
			rY += (x - oldX)/5.0f;
			rX += (y - oldY)/5.0f;
		}
	} else {

		//jeśli wskazano jakiś wierzchołek, zmienna selected_index nie będzie równa -1
		float delta = 1500/abs(dist);
		float valX = (x - oldX)/delta;
		float valY = (oldY - y)/delta;
		//zmiana kursora w zależności od kierunku przeciągania myszy
		if(abs(valX)>abs(valY))
			glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);
		else
			glutSetCursor(GLUT_CURSOR_UP_DOWN);

		//zwiąż renderingowy obiekt tablicy wierzchołków 
		glBindVertexArray(vaoRenderID[readID]);
		//zwiąż obiekt buforowy położeń
		glBindBuffer(GL_ARRAY_BUFFER, vboID_Pos[writeID]);
			//wykonaj mapowanie obiektu bufora, aby uzyskać dostęp do położeń wierzchołków wskazanych
			GLfloat* pData = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
				//przypisz nową wartość wierzchołkowi wskazanemu
				pData[selected_index*4]	  += Right[0]*valX ;
				float newValue = pData[selected_index*4+1]+Up[1]*valY;

				//sprawdź, czy nowa współrzędna Y nie jest większa od 0, bo to oznaczałoby,
				//że wierzchołek jest poniżej podłoża
				if(newValue>0)
					pData[selected_index*4+1] = newValue;
				pData[selected_index*4+2] += Right[2]*valX + Up[2]*valY;
			//zlikwiduj mapowanie bufora, aby zwolnić uzyskany wskaźnik do pamięci GPU
			glUnmapBuffer(GL_ARRAY_BUFFER);
		//zrób to samo z obiektem bufora położeń poprzednich
		glBindBuffer(GL_ARRAY_BUFFER, vboID_PrePos[writeID]);
			pData = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
				pData[selected_index*4]	  += Right[0]*valX ;
				newValue = pData[selected_index*4+1]+Up[1]*valY;
				if(newValue>0)
					pData[selected_index*4+1] = newValue;
				pData[selected_index*4+2] += Right[2]*valX + Up[2]*valY;
			glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindVertexArray(0);
	}

	oldX = x;
	oldY = y;

	glutPostRedisplay();
}


//procedura renderowania siatki 
//używa shadera przejściowego, który przypisuje fragmentowi kolor zapisany w uniformie  
void DrawGrid()
{
	renderShader.Use();
		glBindVertexArray(gridVAOID);
		glUniformMatrix4fv(renderShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
			glDrawElements(GL_LINES, grid_indices.size(),GL_UNSIGNED_SHORT,0);
		glBindVertexArray(0);
	renderShader.UnUse();
}

//renderowanie tkaniny przy użyciu shadera 
void DrawCloth()
{
	renderShader.Use();
		glBindVertexArray(clothVAOID);
		glUniformMatrix4fv(renderShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
			glDrawElements(GL_TRIANGLES, indices.size(),GL_UNSIGNED_SHORT,0);
		//glBindVertexArray(0);
	renderShader.UnUse();
}

//renderowanie sfery przy użyciu shadera renderingu
void DrawSphere(glm::mat4 mvp) {
	renderShader.Use();
		glBindVertexArray(sphereVAOID);
		glUniformMatrix4fv(renderShader("MVP"), 1, GL_FALSE, glm::value_ptr(mvp));
			glDrawElements(GL_TRIANGLES, total_sphere_indices,GL_UNSIGNED_SHORT,0); 
	renderShader.UnUse();
}

//renderowanie wierzchołków tkaniny przy użyciu shadera cząstek
//(przy założeniu, że obiekt tablicy wierzchołków tkaniny jest aktualnie związany)
void DrawClothPoints()
{
	particleShader.Use();
		//glBindVertexArray(clothVAOID);
		glUniform1i(particleShader("selected_index"), selected_index);
		glUniformMatrix4fv(particleShader("MV"), 1, GL_FALSE, glm::value_ptr(mMV));
		glUniformMatrix4fv(particleShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
			//narysuj jeszcze masy
			glDrawArrays(GL_POINTS, 0, total_points);
		glBindVertexArray(0);
	particleShader.UnUse();
}

//inicjalizacja OpenGL
void InitGL() {
	//ustawienie białego koloru tła
	glClearColor(1,1,1,1);

	//generowanie kwerendy sprzętowej
	glGenQueries(1, &query);
	glGenQueries(1, &t_query);

	//ustal rozmiar tekstur dla shaderów
	texture_size_x =  numX+1;
	texture_size_y =  numY+1;

	CHECK_GL_ERRORS
	
	//pobierz czas początkowy
	startTime = (float)glutGet(GLUT_ELAPSED_TIME);
	//pobierz częstotliwość tyknięć
    QueryPerformanceFrequency(&frequency);

    //uruchom stoper
    QueryPerformanceCounter(&t1);

	//zmienne lokalne
	size_t i=0, j=0, count=0;
	int l1=0, l2=0;
	int v = numY+1;
	int u = numX+1;

	printf("Total triangles: %3d\n",numX*numY*2);
	
	//zmień rozmiary wektorów zawierających indeksy, położenia, poprzednie położenia i siły
	indices.resize( numX*numY*2*3);
	X.resize(total_points);
	X_last.resize(total_points);
	F.resize(total_points);

	//wypełnij wektory położeń
	for(int j=0;j<=numY;j++) {
		for(int i=0;i<=numX;i++) {
			X[count] = glm::vec4( ((float(i)/(u-1)) *2-1)* hsize, sizeX+1, ((float(j)/(v-1) )* sizeY),1);
			X_last[count] = X[count];
			count++;
		}
	}

	//wypełnij wektory indeksów
	GLushort* id=&indices[0];
	for (int i = 0; i < numY; i++) {
		for (int j = 0; j < numX; j++) {
			int i0 = i * (numX+1) + j;
			int i1 = i0 + 1;
			int i2 = i0 + (numX+1);
			int i3 = i2 + 1;
			if ((j+i)%2) {
				*id++ = i0; *id++ = i2; *id++ = i1;
				*id++ = i1; *id++ = i2; *id++ = i3;
			} else {
				*id++ = i0; *id++ = i2; *id++ = i3;
				*id++ = i0; *id++ = i3; *id++ = i1;
			}
		}
	}
	//ustaw renderowanie wielokątów jako linii
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//włącz ukrywanie ścianek tylnych
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//ustaw wygładzanie linii
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	 
	//zezwól shaderowi wierzchołków na zmianę rozmiaru punktu
	//przez odpowiedni wpis w rejestrze gl_PointSize
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	//ustaw siły sprężystości tkaniny
	//poziome
	for (l1 = 0; l1 < v; l1++)	// v
		for (l2 = 0; l2 < (u - 1); l2++) {
			AddSpring((l1 * u) + l2,(l1 * u) + l2 + 1,KsStruct,KdStruct);
		}

	//pionowe
	for (l1 = 0; l1 < (u); l1++)
		for (l2 = 0; l2 < (v - 1); l2++) {
			AddSpring((l2 * u) + l1,((l2 + 1) * u) + l1,KsStruct,KdStruct);
		}

	//ścinające
	for (l1 = 0; l1 < (v - 1); l1++)
		for (l2 = 0; l2 < (u - 1); l2++) {
			AddSpring((l1 * u) + l2,((l1 + 1) * u) + l2 + 1,KsShear,KdShear);
			AddSpring(((l1 + 1) * u) + l2,(l1 * u) + l2 + 1,KsShear,KdShear);
		}

	//zginające
	for (l1 = 0; l1 < (v); l1++) {
		for (l2 = 0; l2 < (u - 2); l2++) {
			AddSpring((l1 * u) + l2,(l1 * u) + l2 + 2,KsBend,KdBend);
		}
		AddSpring((l1 * u) + (u - 3),(l1 * u) + (u - 1),KsBend,KdBend);
	}
	for (l1 = 0; l1 < (u); l1++) {
		for (l2 = 0; l2 < (v - 2); l2++) {
			AddSpring((l2 * u) + l1,((l2 + 2) * u) + l1,KsBend,KdBend);
		}
		AddSpring(((v - 3) * u) + l1,((v - 1) * u) + l1,KsBend,KdBend);
	}

	//wczytywanie shaderów
	massSpringShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/Spring.vert");
	particleShader.LoadFromFile(GL_VERTEX_SHADER,"shadery/Basic.vert");
	particleShader.LoadFromFile(GL_FRAGMENT_SHADER,"shadery/Basic.frag");
	renderShader.LoadFromFile(GL_VERTEX_SHADER,"shadery/Passthrough.vert");
	renderShader.LoadFromFile(GL_FRAGMENT_SHADER,"shadery/Passthrough.frag");
	
	//kompilacja i konsolidacja programu shaderowego massSpring
	massSpringShader.CreateAndLinkProgram();
	massSpringShader.Use();
		//dodawanie atrybutów i uniformów
		massSpringShader.AddAttribute("position_mass");
		massSpringShader.AddAttribute("prev_position");
		massSpringShader.AddUniform("tex_position_mass");
		massSpringShader.AddUniform("tex_pre_position_mass");
		massSpringShader.AddUniform("MVP");
		massSpringShader.AddUniform("dt");
		massSpringShader.AddUniform("gravity");
		massSpringShader.AddUniform("ksStr");
		massSpringShader.AddUniform("ksShr");
		massSpringShader.AddUniform("ksBnd");
		massSpringShader.AddUniform("kdStr");
		massSpringShader.AddUniform("kdShr");
		massSpringShader.AddUniform("kdBnd");
		massSpringShader.AddUniform("DEFAULT_DAMPING");
		massSpringShader.AddUniform("texsize_x");
		massSpringShader.AddUniform("texsize_y");
		massSpringShader.AddUniform("step");
		massSpringShader.AddUniform("inv_cloth_size");
		massSpringShader.AddUniform("ellipsoid_xform");	
		massSpringShader.AddUniform("inv_ellipsoid");	
		massSpringShader.AddUniform("ellipsoid");		
	massSpringShader.UnUse();

	CHECK_GL_ERRORS

	//kompilacja i konsolidacja programu shaderowego dla cząstek
	particleShader.CreateAndLinkProgram();
	particleShader.Use();
		//dodawanie atrybutów i uniformów
		particleShader.AddAttribute("position_mass");
		particleShader.AddUniform("pointSize");
		particleShader.AddUniform("MV");
		particleShader.AddUniform("MVP");
		particleShader.AddUniform("vColor");
		particleShader.AddUniform("selected_index");
		//ustalanie wartości uniformów
		glUniform1f(particleShader("pointSize"), pointSize);
		glUniform4fv(particleShader("vColor"),1, vRed);
	particleShader.UnUse();

	//kompilacja i konsolidacja programu shaderowego renderShader
	renderShader.CreateAndLinkProgram();
	renderShader.Use();
		//dodawanie atrybutów i uniformów
		renderShader.AddAttribute("position_mass");
		renderShader.AddUniform("MVP");
		renderShader.AddUniform("vColor");
		//ustalanie wartości uniformów
		glUniform4fv(renderShader("vColor"),1, vGray);
	renderShader.UnUse();

	CHECK_GL_ERRORS

	//utwórz VBO
	createVBO();

	//kolizja z elipsoidą
	ellipsoid = glm::translate(glm::mat4(1),glm::vec3(0,2,0));
	ellipsoid = glm::rotate(ellipsoid, 45.0f ,glm::vec3(1,0,0));
	ellipsoid = glm::scale(ellipsoid, glm::vec3(fRadius,fRadius,fRadius/2));
	inverse_ellipsoid = glm::inverse(ellipsoid);

	//ustaw atrybuty transformacyjnego sprzężenia zwrotnego
	glGenTransformFeedbacks(1, &tfID);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfID);
	//przekaż wyjścia shadera wierzchołków do sprzężenia zwrotnego
	const char* varying_names[]={"out_position_mass", "out_prev_position"};
	glTransformFeedbackVaryings(massSpringShader.GetProgram(), 2, varying_names, GL_SEPARATE_ATTRIBS);
	//ponownie skonsoliduj program massSpringShader
	glLinkProgram(massSpringShader.GetProgram());

	//uaktywnij program massSpringShader i ustal wartości stałych uniformów
	massSpringShader.Use();		
		glUniform1f(massSpringShader("dt"),  timeStep);
		glUniform3fv(massSpringShader("gravity"),1,&gravity.x);
		glUniform1i(massSpringShader("tex_position_mass"), 0);
		glUniform1i(massSpringShader("tex_pre_position_mass"), 1);
		glUniform1i(massSpringShader("texsize_x"),texture_size_x);
		glUniform1i(massSpringShader("texsize_y"),texture_size_y);
		glUniformMatrix4fv(massSpringShader("ellipsoid_xform"), 1, GL_FALSE, glm::value_ptr(ellipsoid));
		glUniformMatrix4fv(massSpringShader("inv_ellipsoid"), 1, GL_FALSE, glm::value_ptr(inverse_ellipsoid));
		glUniform4f(massSpringShader("ellipsoid"),0, 0, 0, fRadius);		
		glUniform2f(massSpringShader("inv_cloth_size"),float(sizeX)/numX,float(sizeY)/numY);
		glUniform2f(massSpringShader("step"),1.0f/(texture_size_x-1.0f),1.0f/(texture_size_y-1.0f));
		glUniform1f(massSpringShader("ksStr"),  KsStruct);
		glUniform1f(massSpringShader("ksShr"),  KsShear);
		glUniform1f(massSpringShader("ksBnd"),  KsBend);
		glUniform1f(massSpringShader("kdStr"),  KdStruct/1000.0f);
		glUniform1f(massSpringShader("kdShr"),  KdShear/1000.0f);
		glUniform1f(massSpringShader("kdBnd"),  KdBend/1000.0f);
		glUniform1f(massSpringShader("DEFAULT_DAMPING"),  DEFAULT_DAMPING);
	massSpringShader.UnUse();

	

	//wyłącz synchronizację pionową
	wglSwapIntervalEXT(0);
}
//obsługa zmiany wymiarów okna
void OnReshape(int nw, int nh) {
	//ustawienie wymiarów okna widokowego
	glViewport(0,0,nw, nh);

	//ustaw macierz rzutowania
	mP = glm::perspective(60.0f, (GLfloat)nw/nh, 1.0f, 100.f);
	for(int j=0;j<4;j++)
		for(int i=0;i<4;i++)
			P[i+j*4] = mP[j][i] ;

	//pobierz wymiary okna widokowego
	glGetIntegerv(GL_VIEWPORT, viewport);
}
//uaktualnij i wyrenderuj cząstki tkaniny
void RenderGPU_TF() {
	CHECK_GL_ERRORS

	//ustaw shader wierzchołków tkaniny
	massSpringShader.Use();
	CHECK_GL_ERRORS
		//ustawianie uniformów shadera
		glUniformMatrix4fv(massSpringShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
	 
		CHECK_GL_ERRORS
		//uruchom pętlę symulacyjną
		for(int i=0;i<NUM_ITER;i++) {
			//ustaw bufor teksturowy dla położenia bieżącego 
			glActiveTexture( GL_TEXTURE0);
			glBindTexture( GL_TEXTURE_BUFFER, texPosID[writeID]);

			//ustaw bufor teksturowy dla położenia poprzedniego
			glActiveTexture( GL_TEXTURE1);
			glBindTexture( GL_TEXTURE_BUFFER, texPrePosID[writeID]);

			//zwiąż aktualizacyjny obiekt tablicy wierzchołków 
			glBindVertexArray( vaoUpdateID[writeID]);
				//zwiąż bufory transformacyjnego sprzężenia zwrotnego
				//indeks 0 -> położenie bieżące
				//indeks 1 -> położenie poprzednie
				glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vboID_Pos[readID]);
				glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, vboID_PrePos[readID]);
				//wyłącz rasteryzację
				glEnable(GL_RASTERIZER_DISCARD);  
					//uruchom sprzętową kwerendę czasową
					glBeginQuery(GL_TIME_ELAPSED,t_query);
						//zainicjuj transformacyjne sprzężenie zwrotne
						glBeginTransformFeedback(GL_POINTS);
							//wyrenderuj punkty, co spowoduje przekazanie wszystkich atrybutów do GPU
							glDrawArrays(GL_POINTS, 0, total_points);
						//zakończ transformacyjne sprzężenie zwrotne
						glEndTransformFeedback();
					//zakończ kwerendę czasową
					glEndQuery(GL_TIME_ELAPSED);
					glFlush();
				//włącz rasteryzację
				glDisable(GL_RASTERIZER_DISCARD);

			//przełącz ścieżki odczytu i zapisu
			int tmp = readID;
			readID=writeID;
			writeID = tmp;
		}
		CHECK_GL_ERRORS
		//pobierz wynik kwerendy
		glGetQueryObjectui64v(t_query, GL_QUERY_RESULT, &elapsed_time);
		//odczytaj czas trwania transformacyjnego sprzężenia zwrotnego
		delta_time = elapsed_time / 1000000.0f;
	//wyłącz shader wierzchołków tkaniny
	massSpringShader.UnUse();

	CHECK_GL_ERRORS;

	//zwiąż renderingowy obiekt tablicy wierzchołków 
	glBindVertexArray(vaoRenderID[writeID]);
		//wyłącz testowanie głębi
		glDisable(GL_DEPTH_TEST);
			//włącz shader renderingu
			renderShader.Use();
				//ustaw uniformy shadera
				glUniformMatrix4fv(renderShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
					//rysuj geometrię tkaniny
					glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT,0);
			//wyłącz shader renderingu
			renderShader.UnUse();
		//włącz testowanie głębi
		glEnable(GL_DEPTH_TEST);

		//jeśli chcesz wyświetlić masy
		if(bDisplayMasses) {
			//włącz shader cząsteczek
			particleShader.Use();
				//ustawianie uniformów shadera
				glUniform1i(particleShader("selected_index"), selected_index);
				glUniformMatrix4fv(particleShader("MV"), 1, GL_FALSE, glm::value_ptr(mMV));
				glUniformMatrix4fv(particleShader("MVP"), 1, GL_FALSE, glm::value_ptr(mMVP));
					//narysuj jeszcze masy
			  		glDrawArrays(GL_POINTS, 0, total_points);
					//i ewentualnie cząstki
					//glDrawTransformFeedbackStream(GL_POINTS, tfID, 0);
			//wyłącz shader cząsteczek
			particleShader.UnUse();
		}
	//odwiąż aktualnie związany obiekt tablicy wierzchołków 
	glBindVertexArray( 0);

	CHECK_GL_ERRORS
}

//funkcja wyświetlania
void OnRender() {

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

	//wyznaczanie liczby klatek na sekundę (FPS)
	++totalFrames;
	if((newTime-startTime)>1000)
	{
		float elapsedTime = (newTime-startTime);
		fps = (totalFrames/ elapsedTime)*1000 ;
		startTime = newTime;
		totalFrames=0;
		sprintf_s(info, "FPS: %3.2f, Frame time (GLUT): %3.4f msecs, Frame time (QP): %3.3f, TF Time: %3.3f", fps, frameTime, frameTimeQP, delta_time);
	}

	glutSetWindowTitle(info);

	//czyszczenie buforów koloru i głębi
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

	//transformacje kamery
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, -2.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	mMV	= glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));
	mMVP = mP*mMV;

	for(int j=0;j<4;j++)
		for(int i=0;i<4;i++)
			MV[i+j*4] = mMV[j][i] ;

	//wyznacz wektory widoku i prawej strony
	viewDir.x = (float)-MV[2];
	viewDir.y = (float)-MV[6];
	viewDir.z = (float)-MV[10];
	Right = glm::cross(viewDir, Up);

	//rysuj siatkę
 	DrawGrid();

	//rysuj elipsoidę
	DrawSphere(mP*(mMV*ellipsoid));

	//deformowanie i renderowanie tkaniny 
	RenderGPU_TF(); 

	//zamiana buforów w celu wyświetlenia wyrenderowanego obrazu
	glutSwapBuffers();
}

//usuwanie wszystkich obiektów
void OnShutdown() {
	X.clear();
	X_last.clear();
	F.clear();
	indices.clear();
	springs.clear();

	glDeleteQueries(1, &query);
	glDeleteQueries(1, &t_query);

	glDeleteTextures( 2, texPosID);
	glDeleteTextures( 2, texPrePosID);

	glDeleteVertexArrays(2, vaoUpdateID);
	glDeleteVertexArrays(2, vaoRenderID);
	glDeleteVertexArrays(1, &clothVAOID);
	glDeleteVertexArrays(1, &gridVAOID);
	glDeleteVertexArrays(1, &sphereVAOID);

	glDeleteBuffers( 1, &gridVBOVerticesID);
	glDeleteBuffers( 1, &gridVBOIndicesID);
	glDeleteBuffers( 1, &clothVBOVerticesID);
	glDeleteBuffers( 1, &clothVBOIndicesID);
	glDeleteBuffers( 1, &sphereVerticesID);
	glDeleteBuffers( 1, &sphereIndicesID);

    glDeleteBuffers( 2, vboID_Pos);
	glDeleteBuffers( 2, vboID_PrePos);
	glDeleteBuffers( 1, &vboIndices);

	glDeleteTransformFeedbacks(1, &tfID);
	renderShader.DeleteShaderProgram();
	massSpringShader.DeleteShaderProgram();
	particleShader.DeleteShaderProgram();
	printf("Shutdown successful");
}


void OnIdle() {
	glutPostRedisplay();
}
 
//obsługa klawiatury
void OnKey(unsigned char key, int , int) {
	switch(key) {
		case 'm': bDisplayMasses=!bDisplayMasses; break;
	}

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
	glutCreateWindow("Symulacja tkaniny z transformacyjnym sprzężeniem zwrotnym");

	//rejestracja funkcji zwrotnych
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnReshape);
	glutIdleFunc(OnIdle);

	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutKeyboardFunc(OnKey);
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


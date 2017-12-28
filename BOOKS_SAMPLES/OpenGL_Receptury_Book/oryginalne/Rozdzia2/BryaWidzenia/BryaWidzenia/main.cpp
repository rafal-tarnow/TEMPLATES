#include <GL/glew.h>

#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "..\..\src\GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")

using namespace std;

//rozmiar okna
const int WIDTH  = 1280;
const int HEIGHT = 960;

//niezbêdne shadery
GLSLShader shader;
//shader renderuj¹cy punkty
GLSLShader pointShader;

//tablica i bufory wierzcho³ków
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

//tablica i bufory wierzcho³ków bry³y widzenia
GLuint vaoFrustumID;
GLuint vboFrustumVerticesID;
GLuint vboFrustumIndicesID;

const int NUM_X = 40; //liczba czworok¹tów wzd³u¿ osi X
const int NUM_Z = 40; //liczba czworok¹tów wzd³u¿ osi Z

const float SIZE_X = 100; //rozmiar p³aszczyzny w przestrzeni œwiata
const float SIZE_Z = 100;
const float HALF_SIZE_X = SIZE_X/2.0f;
const float HALF_SIZE_Z = SIZE_Z/2.0f;

//ca³kowita liczba wierzcho³ków i indeksów
glm::vec3 vertices[(NUM_X+1)*(NUM_Z+1)];
const int TOTAL_INDICES = NUM_X*NUM_Z*2*3;
GLushort indices[TOTAL_INDICES];

//do ustalania dok³adnoœci obliczeñ
const float EPSILON = 0.001f;
const float EPSILON2 = EPSILON*EPSILON;

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=-135, rY=45, fov = 45;

#include "..\..\src\FreeCamera.h"
 
//kody klawiszy
const int VK_W = 0x57;
const int VK_S = 0x53;
const int VK_A = 0x41;
const int VK_D = 0x44;
const int VK_Q = 0x51;
const int VK_Z = 0x5a;

//przyrost czasu
float dt = 0;

//zmienne zwi¹zane z up³ywem czasu
float last_time=0, current_time =0;


//dwie kamery swobodne i wskaŸnik do aktywnej
CFreeCamera cam;
CFreeCamera world;
CFreeCamera* pCurrentCam;

//zmienne zwi¹zane z wyg³adzaniem ruchów myszy
const float MOUSE_FILTER_WEIGHT=0.75f;
const int MOUSE_HISTORY_BUFFER_SIZE = 10;

//bufor historii po³o¿eñ myszy
glm::vec2 mouseHistory[MOUSE_HISTORY_BUFFER_SIZE];

float mouseX=0, mouseY=0; //filtered mouse values

//wy³¹cznik wyg³adzania ruchów myszy
bool useFiltering = true;

//wierzcho³ki bry³y widzenia
glm::vec3 frustum_vertices[8];

//sta³e kolory
GLfloat white[4] = {1,1,1,1};
GLfloat red[4] = {1,0,0,0.5};
GLfloat cyan[4] = {0,1,1,0.5};

//punkty
const int PX = 100;
const int PZ = 100;
const int MAX_POINTS=PX*PZ;

//wierzcho³ki punktów, tablica wierzcho³ków obiekty bufora wierzcho³ków
glm::vec3 pointVertices[MAX_POINTS];
GLuint pointVAOID, pointVBOID;


//liczba punktów widocznych
int total_visible=0;

//zapytanie  
GLuint query;

//zmienne zwi¹zane z szybkoœci¹ animacji
float start_time = 0;
float fps=0;
int total_frames;
float last_time_fps=0;

//bufor tekstowy do wyœwietlania komunikatów
char buffer[MAX_PATH]={'\0'};

//funkcja wyg³adzaj¹ca ruchy myszy
void filterMouseMoves(float dx, float dy) {
    for (int i = MOUSE_HISTORY_BUFFER_SIZE - 1; i > 0; --i) {
        mouseHistory[i] = mouseHistory[i - 1];
    }

    // zapisywanie bie¿¹cego po³o¿enia myszy do bufora historii.
    mouseHistory[0] = glm::vec2(dx, dy);

    float averageX = 0.0f;
    float averageY = 0.0f;
    float averageTotal = 0.0f;
    float currentWeight = 1.0f;

    // wyg³adzanie
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
	else if(button == GLUT_RIGHT_BUTTON)
		state = 2;
	else
		state = 1;
}

//obs³uga ruchów myszy
void OnMouseMove(int x, int y)
{
	bool changed = false;
	if (state == 0) {
		fov += (y - oldY)/5.0f;
		pCurrentCam->SetFOV(fov);
		changed = true;
	} else if(state == 1) {
		rY += (y - oldY)/5.0f;
		rX += (oldX-x)/5.0f;
		if(useFiltering)
			filterMouseMoves(rX, rY);
		else {
			mouseX = rX;
			mouseY = rY;
		}
		if(pCurrentCam == &world) {
			cam.Rotate(mouseX, mouseY,0);
			cam.CalcFrustumPlanes();
		} else {
			pCurrentCam->Rotate(mouseX,mouseY, 0);
		}
		changed = true;
	}
	oldX = x;
	oldY = y;

	if(changed) {
		pCurrentCam->CalcFrustumPlanes();
		frustum_vertices[0] = cam.farPts[0];
		frustum_vertices[1] = cam.farPts[1];
		frustum_vertices[2] = cam.farPts[2];
		frustum_vertices[3] = cam.farPts[3];

		frustum_vertices[4] = cam.nearPts[0];
		frustum_vertices[5] = cam.nearPts[1];
		frustum_vertices[6] = cam.nearPts[2];
		frustum_vertices[7] = cam.nearPts[3];

		//aktualizacja wierzcho³ków bry³y widzenia
		glBindVertexArray(vaoFrustumID);
			glBindBuffer (GL_ARRAY_BUFFER, vboFrustumVerticesID);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(frustum_vertices), &frustum_vertices[0]);
		glBindVertexArray(0);
	}

	glutPostRedisplay();
}
void OnInit() {
	//generowanie zapytania
	glGenQueries(1, &query);

	//w³¹czenie trybu rysowania konturów wielok¹tów
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//w³¹czenie testu g³êbi
	glEnable(GL_DEPTH_TEST);

	//rozmiar punktu
	glPointSize(10);

	GL_CHECK_ERRORS

	//wczytanie shaderów
	shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/shader.frag");
	//kompilacja i konsolidacja programu shaderowego
	shader.CreateAndLinkProgram();
	shader.Use();
		//atrybuty i uniformy
		shader.AddAttribute("vVertex");
		shader.AddUniform("MVP");
		shader.AddUniform("color");
		//inicjalizacja uniformów
		glUniform4fv(shader("color"),1,white);
	shader.UnUse();

	GL_CHECK_ERRORS

	//wczytanie schaderów dla punktów
	pointShader.LoadFromFile(GL_VERTEX_SHADER, "shadery/points.vert");
	pointShader.LoadFromFile(GL_GEOMETRY_SHADER, "shadery/points.geom");
	pointShader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/points.frag");
	//kompilacja i konsolidacja programu shaderowego
	pointShader.CreateAndLinkProgram();
	pointShader.Use();
		//atrybuty i uniformy
		pointShader.AddAttribute("vVertex");
		pointShader.AddUniform("MVP");
		pointShader.AddUniform("t");
		pointShader.AddUniform("FrustumPlanes");
	pointShader.UnUse();

	GL_CHECK_ERRORS

	//przygotowanie geometrii pod³o¿a
	//przygotowanie wierzcho³ków
	int count = 0;
	int i=0, j=0;
	for( j=0;j<=NUM_Z;j++) {
		for( i=0;i<=NUM_X;i++) {
			vertices[count++] = glm::vec3( ((float(i)/(NUM_X-1)) *2-1)* HALF_SIZE_X, 0, ((float(j)/(NUM_Z-1))*2-1)*HALF_SIZE_Z);
		}
	}

	//wype³nienie tablicy indeksów
	GLushort* id=&indices[0];
	for (i = 0; i < NUM_Z; i++) {
		for (j = 0; j < NUM_X; j++) {
			int i0 = i * (NUM_X+1) + j;
			int i1 = i0 + 1;
			int i2 = i0 + (NUM_X+1);
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

	GL_CHECK_ERRORS

	//VAO iVBO dla pod³o¿a
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);

	glBindVertexArray(vaoID);

		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		//przekazanie wierzcho³ków do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//tablica atrybutów wierzcho³ka dla po³o¿enia
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);
		GL_CHECK_ERRORS
		//przekazanie indeksów do bufora tablicy elementów
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS

	//ustalenie po³o¿enia i orientacji kamery
	cam.SetPosition(glm::vec3(2,2,2));
	cam.Rotate(rX,rY,0);
	//ustawienie parametrów rzutowania dla kamery i aktualizacja macierzy
	cam.SetupProjection(fov, ((GLfloat)WIDTH)/HEIGHT,1.f,10);
	cam.Update();

	//wyznaczenie œcian bry³y widzenia dla kamery
	cam.CalcFrustumPlanes();

	//ustalenie po³o¿enia i orientacji kamery w przestrzeni œwiata
	world.SetPosition(glm::vec3(10,10,10));
	world.Rotate(rX,rY,0);
	//ustawienie rzutowania i aktualizacja parametrów kamery
	world.SetupProjection(fov,(GLfloat)WIDTH/HEIGHT, 0.1f, 100.0f);
	world.Update();

	//obiekt cam kamer¹ aktywn¹
	pCurrentCam = &cam;

	//przygotowanie geometrii bry³y widzenia
	glGenVertexArrays(1, &vaoFrustumID);
	glGenBuffers(1, &vboFrustumVerticesID);
	glGenBuffers(1, &vboFrustumIndicesID);

	//zapisywanie wierzcho³ków bry³y
	frustum_vertices[0] = cam.farPts[0];
	frustum_vertices[1] = cam.farPts[1];
	frustum_vertices[2] = cam.farPts[2];
	frustum_vertices[3] = cam.farPts[3];

	frustum_vertices[4] = cam.nearPts[0];
	frustum_vertices[5] = cam.nearPts[1];
	frustum_vertices[6] = cam.nearPts[2];
	frustum_vertices[7] = cam.nearPts[3];

	GLushort frustum_indices[36]={0,4,3,3,4,7, //górna
								  6,5,1,6,1,2, //dolna
								  0,1,4,4,1,5, //lewa
								  7,6,3,3,6,2, //prawa
								  4,5,6,4,6,7, //bliska
								  3,2,0,0,2,1, //daleka
								  };
	glBindVertexArray(vaoFrustumID);

		glBindBuffer (GL_ARRAY_BUFFER, vboFrustumVerticesID);
		//przekazanie wierzcho³ków bry³y do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, sizeof(frustum_vertices), &frustum_vertices[0], GL_DYNAMIC_DRAW);
		GL_CHECK_ERRORS
		//tablica atrybutów wierzcho³ka dla po³o¿enia
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);
		GL_CHECK_ERRORS
		//przekazanie indeksów do bufora tablicy elementów
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboFrustumIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(frustum_indices), &frustum_indices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
			 

	//przygotowanie zestawu punktów
	for(int j=0;j<PZ;j++) {
		for(int i=0;i<PX;i++) {
			float   x = i/(PX-1.0f);
			float   z = j/(PZ-1.0f); 
			pointVertices[j*PX+i] = glm::vec3(x,0,z);
		}
	}
	//VAO i VBO dla punktów
	glGenVertexArrays(1, &pointVAOID);
	glGenBuffers(1, &pointVBOID);
	glBindVertexArray(pointVAOID);
	glBindBuffer (GL_ARRAY_BUFFER, pointVBOID);
	//przekazanie wierzcho³ków do obiektu bufora
	glBufferData (GL_ARRAY_BUFFER, sizeof(pointVertices), pointVertices, GL_STATIC_DRAW);

	//tablica atrybutów wierzcho³ka dla po³o¿enia
	glEnableVertexAttribArray(pointShader["vVertex"]);
	glVertexAttribPointer(pointShader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);

	// pobranie kierunku patrzenia kamery w celu okreœlenia wartoœci parametrów yaw i pitch
	glm::vec3 look =  glm::normalize(cam.GetPosition());
	float yaw = glm::degrees(float(atan2(look.z, look.x)+M_PI));
	float pitch = glm::degrees(asin(look.y));
	rX = yaw;
	rY = pitch;

	//wype³nianie bufora historii po³o¿eñ myszy, jeœli wyg³adzanie jest w³¹czone 
	if(useFiltering) {
		for (int i = 0; i < MOUSE_HISTORY_BUFFER_SIZE ; ++i) {
			mouseHistory[i] = glm::vec2(rX, rY);
		}
	}

	cout<<"Inicjalizacja powiodla sie"<<endl;

	//czas pocz¹tkowy
	start_time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {
	glDeleteQueries(1, &query);

	//likwidacja programu shaderowego
	shader.DeleteShaderProgram();
	pointShader.DeleteShaderProgram();

	//likwidacja vao i vbo dla wierzcho³ków
	glDeleteBuffers(1, &vboVerticesID);
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID);

	//likwidacja vao i vbo dla bry³y widzenia
	glDeleteVertexArrays(1, &vaoFrustumID);
	glDeleteBuffers(1, &vboFrustumVerticesID);
	glDeleteBuffers(1, &vboFrustumIndicesID);

	//likwidacja vao i vbo dla punktów
	glDeleteVertexArrays(1, &pointVAOID);
	glDeleteBuffers(1, &pointVBOID);

	cout<<"Zamkniecie powiodlo sie"<<endl;
}

//obs³uga zmiany wymiarów okna
void OnResize(int w, int h) {
	//wymiary okna widokowego
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
}

//reakcja na sygna³ bezczynnoœci procesora
void OnIdle() {

	//obs³uga klawiszy W,S,A,D,Q i Z s³u¿¹cych do poruszania kamer¹
	if( GetAsyncKeyState(VK_W) & 0x8000) {
		pCurrentCam->Walk(dt);
	}

	if( GetAsyncKeyState(VK_S) & 0x8000) {
		pCurrentCam->Walk(-dt);
	}
	if( GetAsyncKeyState(VK_A) & 0x8000) {
		pCurrentCam->Strafe(-dt);
	}

	if( GetAsyncKeyState(VK_D) & 0x8000) {
		pCurrentCam->Strafe(dt);
	}

	if( GetAsyncKeyState(VK_Q) & 0x8000) {
		pCurrentCam->Lift(dt);
	}

	if( GetAsyncKeyState(VK_Z) & 0x8000) {
		pCurrentCam->Lift(-dt);
	}

	glm::vec3 t = pCurrentCam->GetTranslation(); 

	if(glm::dot(t,t)>EPSILON2) {
		pCurrentCam->SetTranslation(t*0.95f);
	}

	//aktualizacja bry³y widzenia
	if(pCurrentCam == &cam) {
		//ustalenie nowych wierzcho³ków bry³y widzenia
		pCurrentCam->CalcFrustumPlanes();
		frustum_vertices[0] = cam.farPts[0];
		frustum_vertices[1] = cam.farPts[1];
		frustum_vertices[2] = cam.farPts[2];
		frustum_vertices[3] = cam.farPts[3];

		frustum_vertices[4] = cam.nearPts[0];
		frustum_vertices[5] = cam.nearPts[1];
		frustum_vertices[6] = cam.nearPts[2];
		frustum_vertices[7] = cam.nearPts[3];

		//przekazanie wierzcho³ków bry³y widzenia do GPU
		glBindVertexArray(vaoFrustumID);
			glBindBuffer (GL_ARRAY_BUFFER, vboFrustumVerticesID);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(frustum_vertices), &frustum_vertices[0]);
		glBindVertexArray(0);
	}

	//wywo³anie funkcji wyœwietlaj¹cej
	glutPostRedisplay();
}

//funkcja zwrotna wyœwietlania
void OnRender() {
	//obliczenia zwi¹zane z szybkoœci¹ animacji
	++total_frames;
	last_time = current_time;
	current_time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
	dt = current_time-last_time;
	if( (current_time-last_time_fps) >1) {
		fps = total_frames/(current_time-last_time_fps);
		last_time_fps = current_time;
		total_frames = 0;
	}

	//czyszczenie buforów koloru i g³êbi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//transformacje kamery
	glm::mat4 MV	= pCurrentCam->GetViewMatrix();
	glm::mat4 P     = pCurrentCam->GetProjectionMatrix();
    glm::mat4 MVP	= P*MV;

	//pobieranie œcian bry³y widzenia
	glm::vec4 p[6];
	pCurrentCam->GetFrustumPlanes(p);

	//pocz¹tek zapytania
	glBeginQuery(GL_PRIMITIVES_GENERATED, query);

	//wi¹zanie shadera punktów
	pointShader.Use();
		//ustalanie uniformów shadera
		glUniform1f(pointShader("t"), current_time);
		glUniformMatrix4fv(pointShader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform4fv(pointShader("FrustumPlanes"), 6, glm::value_ptr(p[0]));

		//wi¹zanie obiektu tablicy wierzcho³ków dla punktów
		glBindVertexArray(pointVAOID);
			//rysowanie punktów
			glDrawArrays(GL_POINTS,0,MAX_POINTS);

	//odwi¹zywanie shadera punktów
	pointShader.UnUse();

	//koniec zapytania
	glEndQuery(GL_PRIMITIVES_GENERATED);

	//ustalenie liczby widocznych punktów na podstawie wyniku zapytania
	GLuint res;
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &res);
	sprintf_s(buffer, "FPS: %3.3f :: Liczba widocznych punktów: %3d",fps, res);
	glutSetWindowTitle(buffer);

	//shader pod³o¿a
	shader.Use();
	//wi¹zanie obiektu tablicy wierzcho³ków dla pod³o¿a
	glBindVertexArray(vaoID);
		//ustalanie uniformów shadera
		glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform4fv(shader("color"),1,white);
			//rysowanie trójk¹tów
			glDrawElements(GL_TRIANGLES, TOTAL_INDICES, GL_UNSIGNED_SHORT, 0);
			
	//rysowanie bry³y widzenia kamery lokalnej, gdy aktywna jest kamera globalna
	if(pCurrentCam == &world) {
		//ustalanie uniformów shadera
		glUniform4fv(shader("color"),1,red);

		//wi¹zanie obiektu tablicy wierzcho³ków dla bry³y widzenia
	 	glBindVertexArray(vaoFrustumID);

		//w³¹czenie mieszania alfa
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//w³¹czenie trybu rysowania pe³nych wielok¹tów
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			//rysowanie trójk¹tów
	 		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
		//przywrócenie trybu rysowania konturów
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//wy³¹czenie mieszania
		glDisable(GL_BLEND);
	}

	//odwi¹zanie shadera
	shader.UnUse();

	//zamiana buforów w celu wyœwietlenia obrazu
	glutSwapBuffers();
}

//obs³uga klawiszy s³u¿¹cych do prze³¹czania kamer 
void OnKey(unsigned char key, int x, int y) {
	switch(key) {
		case '1':
			pCurrentCam = &cam;
		break;

		case '2':
			pCurrentCam = &world;
		break;
	}
	glutPostRedisplay();
}


int main(int argc, char** argv) {
	//inicjalizacja freeglut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Free Camera - OpenGL 3.3");

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
	
	//callback hooks
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutKeyboardFunc(OnKey);
	glutIdleFunc(OnIdle);

	//wywo³anie pêtli g³ównej
	glutMainLoop();

	return 0;
}
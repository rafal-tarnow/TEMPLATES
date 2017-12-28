#include <GL/glew.h>

#define _USE_MATH_DEFINES
#include <cmath>

#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "../src/GLSLShader.h"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

#pragma comment(lib, "glew32.lib")

using namespace std;

//rozmiar okna
const int WIDTH  = 1024;
const int HEIGHT = 768;

//shader oœwietlania wierzcho³ków
GLSLShader shader;
GLSLShader* pFlatShader; // shadera rysuj¹cego siatkê u¿yjemy równie¿ do narysowania wektora kieunku œwiat³a

//struktura dla wierzcho³ka z jego po³o¿eniem i wektorem normalnym
struct Vertex {
	glm::vec3 pos, normal;
};

//tablica wierzcho³ków sfery oraz ID obiektu bufora wiercho³ków
GLuint sphereVAOID;
GLuint sphereVerticesVBO;
GLuint sphereIndicesVBO;

//tablica wierzcho³ków kostki oraz ID obiektu bufora wiercho³ków
GLuint cubeVAOID;
GLuint cubeVerticesVBO;
GLuint cubeIndicesVBO;

//tablica wierzcho³ków odcinków wskazuj¹ceych po³o¿enie œwiat³a oraz ID obiektu bufora wiercho³ków
GLuint lightVAOID;
GLuint lightVerticesVBO;

//macierze rzutowania, modelu i widoku 
glm::mat4  P = glm::mat4(1);
glm::mat4  MV = glm::mat4(1); 

//zmienne transformacyjne kamery
int state = 0, oldX=0, oldY=0;
float rX=25, rY=-40, dist = -10;

//siatka
#include "../src/Grid.h"
CGrid* grid;

glm::vec3 lightPosOS=glm::vec3(0, 2,0); //po³o¿enie œwiat³a w przestrzeni obiektu
 
#include <vector>

//vwierzcho³ki i indeksy sfery (szeœcianu)
std::vector<Vertex> vertices;
std::vector<GLushort> indices;
int totalSphereTriangles = 0;

//Wspó³rzêdne sferyczne po³o¿enia œwiat³a
float theta = 1.36f;
float phi = 0.72f;
float radius = 4;

//dodawanie indeksów sfery do wektora indeksów
inline void push_indices(int sectors, int r, int s, std::vector<GLushort>& indices) {
    int curRow = r * sectors;
    int nextRow = (r+1) * sectors;

    indices.push_back(curRow + s);
    indices.push_back(nextRow + s);
    indices.push_back(nextRow + (s+1));

    indices.push_back(curRow + s);
    indices.push_back(nextRow + (s+1));
    indices.push_back(curRow + (s+1));
}

//generowanie prymitywu sfery o zadanym promieniu oraz liczbach po³udników i równole¿ników
void CreateSphere( float radius, unsigned int slices, unsigned int stacks, std::vector<Vertex>& vertices, std::vector<GLushort>& indices)
{
    float const R = 1.0f/(float)(slices-1);
    float const S = 1.0f/(float)(stacks-1);

    for(size_t r = 0; r < slices; ++r) {
        for(size_t s = 0; s < stacks; ++s) {
            float const y = (float)(sin( -M_PI_2 + M_PI * r * R ));
            float const x = (float)(cos(2*M_PI * s * S) * sin( M_PI * r * R ));
            float const z = (float)(sin(2*M_PI * s * S) * sin( M_PI * r * R ));

			Vertex v;
			v.pos = glm::vec3(x,y,z)*radius;
			v.normal = glm::normalize(v.pos);
            vertices.push_back(v);
            push_indices(stacks, r, s, indices);
        }
    }

}

//generowanie szeœcianu o zadanym rozmiarze
void CreateCube(const float& size, std::vector<Vertex>& vertices, std::vector<GLushort>& indices) {
	float halfSize = size/2.0f;
	glm::vec3 positions[8];
	positions[0]=glm::vec3(-halfSize,-halfSize,-halfSize);
	positions[1]=glm::vec3( halfSize,-halfSize,-halfSize);
	positions[2]=glm::vec3( halfSize, halfSize,-halfSize);
	positions[3]=glm::vec3(-halfSize, halfSize,-halfSize);
	positions[4]=glm::vec3(-halfSize,-halfSize, halfSize);
	positions[5]=glm::vec3( halfSize,-halfSize, halfSize);
	positions[6]=glm::vec3( halfSize, halfSize, halfSize);
	positions[7]=glm::vec3(-halfSize, halfSize, halfSize);

	glm::vec3 normals[6];
	normals[0]=glm::vec3(-1.0,0.0,0.0);
	normals[1]=glm::vec3(1.0,0.0,0.0);
	normals[2]=glm::vec3(0.0,1.0,0.0);
	normals[3]=glm::vec3(0.0,-1.0,0.0);
	normals[4]=glm::vec3(0.0,0.0,1.0);
	normals[5]=glm::vec3(0.0,0.0,-1.0);

	indices.resize(36);
	vertices.resize(36);

	//wype³nianie tablicy indeksów
	GLushort* id=&indices[0];
	//left face
	*id++ = 7; 	*id++ = 3; 	*id++ = 4;
	*id++ = 3; 	*id++ = 0; 	*id++ = 4;

	//œciana prawa
	*id++ = 2; 	*id++ = 6; 	*id++ = 1;
	*id++ = 6; 	*id++ = 5; 	*id++ = 1;

	//œciana górna
	*id++ = 7; 	*id++ = 6; 	*id++ = 3;
	*id++ = 6; 	*id++ = 2; 	*id++ = 3;
	//œciana dolna
	*id++ = 0; 	*id++ = 1; 	*id++ = 4;
	*id++ = 1; 	*id++ = 5; 	*id++ = 4;

	//œciana przednia
	*id++ = 6; 	*id++ = 4; 	*id++ = 5;
	*id++ = 6; 	*id++ = 7; 	*id++ = 4;
	//back face
	*id++ = 0; 	*id++ = 2; 	*id++ = 1;
	*id++ = 0; 	*id++ = 3; 	*id++ = 2;


	for(int i=0;i<36;i++) {
		int normal_index = i/6;
		vertices[i].pos=positions[indices[i]];
		vertices[i].normal=normals[normal_index];
		indices[i]=i;
	}
}

//obs³uga klikniêcia mysz¹
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
	if (state == 0)
		dist *= (1 + (y - oldY)/60.0f);
	else if(state ==2) {
		theta += (oldX - x)/60.0f;
		phi += (y - oldY)/60.0f;

		lightPosOS.x = radius * cos(theta)*sin(phi);
		lightPosOS.y = radius * cos(phi);
		lightPosOS.z = radius * sin(theta)*sin(phi);

	} else {
		rY += (x - oldX)/5.0f;
		rX += (y - oldY)/5.0f;
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}

//Inicjalizacja OpenGL
void OnInit() { 

	//wczytywanie shaderów oœwietlenia na poziomie fragmentów
	shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/PointLight.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/PointLight.frag");
	//kompilacja i konsolidacja programu shaderowego
	shader.CreateAndLinkProgram();
	shader.Use();
		//dodawanie atrybutów i uniformów
		shader.AddAttribute("vVertex");
		shader.AddAttribute("vNormal");
		shader.AddUniform("MVP");
		shader.AddUniform("MV");
		shader.AddUniform("N");
		shader.AddUniform("light_position");
		shader.AddUniform("diffuse_color");
	shader.UnUse();

	GL_CHECK_ERRORS

	//ustalanie geometrii sfery
	CreateSphere(1.0f,10,10, vertices, indices);

	//ustawianie vao i vbo dla sfery
	glGenVertexArrays(1, &sphereVAOID);
	glGenBuffers(1, &sphereVerticesVBO);
	glGenBuffers(1, &sphereIndicesVBO);
	glBindVertexArray(sphereVAOID);

		glBindBuffer (GL_ARRAY_BUFFER, sphereVerticesVBO);
		//przekazanie wierzcho³ków do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//w³¹czenie tablicy atrybutów wierzho³ka dla po³o¿enia
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);
		GL_CHECK_ERRORS
		//w³¹czenie tablicy atrybutów wierzho³ka dla wektora normalnego
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, normal)));
		GL_CHECK_ERRORS
		//przekazanie indeksów sfery do bufora tablicy elementów
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, sphereIndicesVBO);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

	//zapisywanie liczby trójk¹tów dla sfery
	totalSphereTriangles = indices.size();

	//czyszczenie wektorów wierzcho³ków i wskaŸników, bo bêd¹ potrzebne do
	//wyznaczania kostki
	vertices.clear();
	indices.clear();

	//ustalanie geometrii kostki
	CreateCube(2,vertices, indices);

	//vao i vbo dla kostki
	glGenVertexArrays(1, &cubeVAOID);
	glGenBuffers(1, &cubeVerticesVBO);
	glGenBuffers(1, &cubeIndicesVBO);
	glBindVertexArray(cubeVAOID);

		glBindBuffer (GL_ARRAY_BUFFER, cubeVerticesVBO);
		//przekazanie wierzcho³ków do obiektu bufora
		glBufferData (GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//w³¹czenie tablicy atrybutów wierzho³ka dla po³o¿enia
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex),0);
		GL_CHECK_ERRORS
		
		//w³¹czenie tablicy atrybutów wierzho³ka dla wektora normalnego
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, normal)));
		GL_CHECK_ERRORS

		//przekazanie indeksów kostki do bufora tablicy elementów
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, cubeIndicesVBO);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

	GL_CHECK_ERRORS

	//vao i vbo dla odcinków wskazuj¹cych po³o¿enie Ÿród³a œwiat³a
	glm::vec3 crossHairVertices[6];
	crossHairVertices[0] = glm::vec3(-0.5f,0,0);
	crossHairVertices[1] = glm::vec3(0.5f,0,0);
	crossHairVertices[2] = glm::vec3(0, -0.5f,0);
	crossHairVertices[3] = glm::vec3(0, 0.5f,0);
	crossHairVertices[4] = glm::vec3(0,0, -0.5f);
	crossHairVertices[5] = glm::vec3(0,0, 0.5f);

	//przygotowanie tablicy wierzcho³ków i obiektu bufora do przechowania gizma œwiat³a
	glGenVertexArrays(1, &lightVAOID);
	glGenBuffers(1, &lightVerticesVBO);
	glBindVertexArray(lightVAOID);

		glBindBuffer (GL_ARRAY_BUFFER, lightVerticesVBO);
		//przekazanie wierzcho³ków odcinka do bufora
		glBufferData (GL_ARRAY_BUFFER, sizeof(crossHairVertices), &(crossHairVertices[0].x), GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//w³¹czenie tablicy atrybutów wierzho³ka dla po³o¿enia
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,0);

		GL_CHECK_ERRORS

	//w³¹czenie testowania g³êbi ukrywania œcianek niewidocznych
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	GL_CHECK_ERRORS

	//tworzenie siatki o wymiarach 10x10 na p³aszczyŸnie XZ
	grid = new CGrid();

	GL_CHECK_ERRORS

	//pobranie wskaŸnika do shadera siatki potrzebnego do wyrenderowania gizma
	pFlatShader = grid->GetShader();

	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	cout<<"Inicjalizacja powiodla sie"<<endl;
}

//zwalnianie wszystkich alokowanych zasobów
void OnShutdown() {
	pFlatShader = NULL;

	//likwidacja programu shaderowego
	shader.DeleteShaderProgram();
	//likwidacja vao i vbo
	glDeleteBuffers(1, &sphereVerticesVBO);
	glDeleteBuffers(1, &sphereIndicesVBO);
	glDeleteVertexArrays(1, &sphereVAOID);

	glDeleteBuffers(1, &cubeVerticesVBO);
	glDeleteBuffers(1, &cubeIndicesVBO);
	glDeleteVertexArrays(1, &cubeVAOID);

	glDeleteVertexArrays(1, &lightVAOID);
	glDeleteBuffers(1, &lightVerticesVBO);

	delete grid;
	cout<<"Zamkniecie powiodlo sie"<<endl;
}

//obs³uga zmiany wymiarów okna
void OnResize(int w, int h) {
	//ustalanie wymiarów okna widokowego
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//wyznaczanie macierzy rzutowania
	P = glm::perspective(45.0f, (GLfloat)w/h, 0.1f, 1000.f);
}

//obs³uga sygna³u bezczynnoœci procesora - wywo³uje tylko funkcjê wyœwietlaj¹c¹
void OnIdle() {
	glutPostRedisplay();
}

//funkcja renderuj¹ca scenê
void DrawScene(glm::mat4 View, glm::mat4 Proj ) {

	GL_CHECK_ERRORS

	//wi¹zanie bie¿¹cego shadera
	shader.Use();

	//wi¹zanie tablicy wierzcho³ków kostki
	glBindVertexArray(cubeVAOID); {
		//okreœlenie przekszta³ceñ dla kostki
		glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(-1,1,0));
		glm::mat4 M = T;			 //macierz modelu
		glm::mat4 MV = View*M;		 //macierz modelu i widoku
		glm::mat4 MVP = Proj*MV;	 //po³¹czona macierz modelu, widoku i rzutowania

		//uniformy shadera
		glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));
		glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
		glUniform3f(shader("diffuse_color"), 1.0f,0.0f,0.0f);
		glUniform3fv(shader("light_position"),1, &(lightPosOS.x));
			//rysowanie trójk¹tów
	 		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
	}
	//wi¹zanie tablicy wierzcho³ków sfery
	glBindVertexArray(sphereVAOID); {
		//set the sphere's transform
		glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(1,1,0));
		glm::mat4 M = T;
		glm::mat4 MV = View*M;
		glm::mat4 MVP = Proj*MV;
		//uniformy shadera
		glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));
		glUniformMatrix3fv(shader("N"), 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
		glUniform3f(shader("diffuse_color"), 0.0f, 0.0f, 1.0f);
			//rysowanie trójk¹tów
		 	glDrawElements(GL_TRIANGLES, totalSphereTriangles, GL_UNSIGNED_SHORT, 0);
	}
	//odwi¹zanie shadera
	shader.UnUse();

	GL_CHECK_ERRORS

	//wi¹zanie obiektu tablicy wierzcho³ków gizma œwiat³a
	glBindVertexArray(lightVAOID); {
		//transformacje œwiat³a
		glm::mat4 T = glm::translate(glm::mat4(1), lightPosOS);
		//wi¹zanie shadera i rysowanier trzech odcinków
		pFlatShader->Use();
			glUniformMatrix4fv((*pFlatShader)("MVP"), 1, GL_FALSE, glm::value_ptr(Proj*View*T));
				glDrawArrays(GL_LINES, 0, 6);
		//odwi¹zanie shadera
		pFlatShader->UnUse();
	}

	//odwi¹zanie obiektu tablicy wierzcho³ków
	glBindVertexArray(0);

	//renderowanie siatki
	grid->Render(glm::value_ptr(Proj*View));
}

//funkcja zwrotna wyœwietlania
void OnRender() {

	GL_CHECK_ERRORS

	//czyszczenie buforów koloru i g³êbi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//transformacje kamery
	glm::mat4 T		= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(T,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//renderowanie sceny
	DrawScene(MV, P);

	//swap front and back buffers to show the rendered result
	glutSwapBuffers();
}

//obs³uga rolki przewijania zmienia wspó³rzêdn¹ radialn¹ Ÿród³a œwiat³a
//jako ¿e po³o¿enie Ÿród³a jest wyra¿one we wspó³rzêdnych sferycznych promieñ
//okreœla odleg³oœæ Ÿród³a od œrodka uk³adu.
void OnMouseWheel(int button, int dir, int x, int y) {

	if (dir > 0)
    {
        radius += 0.1f;
    }
    else
    {
        radius -= 0.1f;
    }

	//wyznaczanie nowego po³o¿enia Ÿród³a œwiat³a
	lightPosOS.x = radius * cos(theta)*sin(phi);
	lightPosOS.y = radius * cos(phi);
	lightPosOS.z = radius * sin(theta)*sin(phi);

	//wywo³anie funkcji wyœwietlaj¹cej
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	//inicjalizacja freeglut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Œwiat³o punktowe - OpenGL 3.3");

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
	glutMouseWheelFunc(OnMouseWheel);
	glutIdleFunc(OnIdle);

	//wywo³anie pêtli g³ównej
	glutMainLoop();

	return 0;
}
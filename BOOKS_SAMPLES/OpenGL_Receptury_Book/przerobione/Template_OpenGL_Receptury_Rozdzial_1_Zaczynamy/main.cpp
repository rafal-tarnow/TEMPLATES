#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#pragma comment(lib, "glew32.lib")

using namespace std;

//wymiary obrazu
const int WIDTH  = 1280;
const int HEIGHT = 960;

//inicjalizacja obiektu OpenGL 
void OnInit() {
	//ustawienie koloru czerwonego
	glClearColor(1,0,0,0);
	cout<<"Inicjalizacja powiodĹ‚a siÄ™"<<endl;
}

//zwalnianie przydzielonych zasobĂłw
void OnShutdown() {
	cout<<"ZamkniÄ™cie udaĹ‚o siÄ™"<<endl;
}

//obsĹ‚uga zdarzenia zmiany wymiarĂłw
void OnResize(int nw, int nh) {

}

//funkcja zwrotna wyÂświetlania
void OnRender() {
	//czyszczenie buforĂłw koloru i gĹ‚Ä™bi
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//zamiana buforĂłw przedniego i tylnego w celu wyÂświetlenia wynikĂłw renderingu
	glutSwapBuffers();
}

int main(int argc, char** argv) {
	//inicjujĹˇce wywoĹ‚ania biblioteki freeglut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitContextVersion (3, 0);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitContextProfile(GLUT_FORWARD_COMPATIBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Zaczynamy pracÄ™ z OpenGL 3.3");

	//inicjalizacja biblioteki GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)	{
		cerr<<"BĹ‚Ĺˇd: "<<glewGetErrorString(err)<<endl;
	} else {
		if (GLEW_VERSION_3_3)
		{
			cout<<"Sterownik obsĹ‚uguje OpenGL 3.3\nSczegĂłĹ‚y:"<<endl;
		}
	}

	//wyÂświetlanie informacji
	cout<<"\tBiblioteka GLEW "<<glewGetString(GLEW_VERSION)<<endl;
	cout<<"\tProducent: "<<glGetString (GL_VENDOR)<<endl;
	cout<<"\tRenderer: "<<glGetString (GL_RENDERER)<<endl;
	cout<<"\tWersja: "<<glGetString (GL_VERSION)<<endl;
	cout<<"\tGLSL: "<<glGetString (GL_SHADING_LANGUAGE_VERSION)<<endl;

	//inicjalizacja OpenGL
	OnInit();
		
	//rejestracja funkcji zwrotnych
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);

	//uruchomienie gĹ‚Ăłwnej pÄ™tli
	glutMainLoop();

	return 0;
}

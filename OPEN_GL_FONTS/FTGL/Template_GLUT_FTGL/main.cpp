#include <math.h> // sin(), cos()
#include <stdlib.h> // exit()

#include <GL/glut.h>
#include <FTGL/ftgl.h>
#include <FTGL/FTFont.h>
#include <FTGL/FTGLTextureFont.h>

#include <iostream>

using namespace std;

static FTFont *font[5];
static int fontindex = 0;

#define FONT_FILE "./data/font/arial.ttf"


static void RenderScene(void)
{
    int now = glutGet(GLUT_ELAPSED_TIME);

    float n = (float)now / 20.;


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(1.0,0.5,0.0);


    glEnable(GL_DEPTH_TEST);


    glPushMatrix();
    glRotatef(n / 1.11, 1.0, 1.0, 1.0);
    glTranslatef(0.0, 00.0, 0.0);
    glColor3f(1.0, 0.0, 0.0);
    font[fontindex]->Render("ZMT");
    glPopMatrix();

    glutSwapBuffers();
}


static void ProcessKeys(unsigned char key, int x, int y)
{
    switch(key)
    {
    case 27:
        delete font[0];
        delete font[1];
        delete font[2];
        delete font[3];
        exit(EXIT_SUCCESS);
        break;
    case '\t':
        fontindex = (fontindex + 1) % 4;
        cout << "fontindex = " << fontindex << endl;
        break;
    }
}


int main(int argc, char **argv)
{
    char const *file = NULL;
    file = FONT_FILE;


    // Initialise GLUT stuff
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(640, 480);

    glutCreateWindow("simple FTGL C++ demo");

    glutDisplayFunc(RenderScene);
    glutIdleFunc(RenderScene);
    glutKeyboardFunc(ProcessKeys);



    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-150, 150, -150, 150, -1000.0, 2000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // gluLookAt(0.0, 0.0, 640.0f / 2.0f, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    // Initialise FTGL stuff
    font[0] = new FTExtrudeFont(file);
    font[1] = new FTBufferFont(file);
    font[2] = new FTTextureFont(file);
    font[3] = new FTOutlineFont(file);

    font[0]->FaceSize(160);
    font[0]->Depth(20);
    font[0]->Outset(0, 5);
    font[0]->CharMap(ft_encoding_unicode);

    font[1]->FaceSize(160);
    font[1]->CharMap(ft_encoding_unicode);

    font[2]->FaceSize(160);
    font[2]->CharMap(ft_encoding_unicode);

    font[3]->FaceSize(160);
    font[3]->CharMap(ft_encoding_unicode);


    // Run GLUT loop
    glutMainLoop();

    return EXIT_SUCCESS;
}


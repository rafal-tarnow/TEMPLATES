#include <math.h> // sin(), cos()
#include <stdlib.h> // exit()

#include <GL/glut.h>
#include <FTGL/ftgl.h>
#include <FTGL/FTFont.h>
#include <FTGL/FTGLTextureFont.h>

#include <iostream>

using namespace std;

static FTFont *font[2];
static int fontindex = 0;

#define FONT_FILE "./data/font/design_graffiti_agentorange_www_myfontfree_com.ttf"





static void RenderScene(void)
{
    int now = glutGet(GLUT_ELAPSED_TIME);

    float n = (float)now / 20.;


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(1.0,0.5,0.0);


    glEnable(GL_DEPTH_TEST);


    glPushMatrix();
    {
        glTranslatef(-100,-100,0);
       // glRotatef(n / 1.11, 1.0, 1.0, 1.0);
        glColor3f(0.2, 1.0, 0.0);

        font[fontindex]->Render("ZMT: 123 Tarnow");
    }
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
        exit(EXIT_SUCCESS);
        break;
    case '\t':
        fontindex = (fontindex + 1) % 2;
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
    glOrtho(-150, 150, -150, 150, -2000.0, 2000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();



    font[0] = new FTBufferFont(file);
    font[1] = new FTTextureFont(file);



    font[0]->FaceSize(40);
    font[0]->CharMap(ft_encoding_unicode);

    font[1]->FaceSize(40);
    font[1]->CharMap(ft_encoding_unicode);




    // Run GLUT loop
    glutMainLoop();

    return EXIT_SUCCESS;
}


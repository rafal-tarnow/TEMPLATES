#include <math.h> // sin(), cos()
#include <stdlib.h> // exit()

#include <GL/glut.h>
#include <FTGL/ftgl.h>
#include <FTGL/FTFont.h>
#include <FTGL/FTGLTextureFont.h>

#include <iostream>
using namespace std;


static FTFont *font;
static int fontindex = 0;

#define FONT_FILE "./data/font/arial.ttf"




class FTCustomHaloGlyph : public FTPolygonGlyph
{
public:
    FTCustomHaloGlyph(FT_GlyphSlot glyph) : FTPolygonGlyph(glyph, 0, true)
    {
        for(int i = 0; i < 5; i++)
        {
            subglyph[i] = new FTOutlineGlyph(glyph, i, true);
        }
    }

private:
    const FTPoint& Render(const FTPoint& pen, int renderMode)
    {
        glPushMatrix();
        for(int i = 0; i < 5; i++)
        {
            glTranslatef(0.0, 0.0, -2.0);
            subglyph[i]->Render(pen, renderMode);
        }
        glPopMatrix();

        return FTPolygonGlyph::Render(pen, renderMode);
    }

    FTGlyph *subglyph[5];
};


class FTCustomHaloFont : public FTFont
{
public:
    FTCustomHaloFont(char const *fontFilePath) : FTFont(fontFilePath) {}

private:
    virtual FTGlyph* MakeGlyph(FT_GlyphSlot slot)
    {
        return new FTCustomHaloGlyph(slot);
    }
};


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
    font->Render("ZMT");
    glPopMatrix();

    glutSwapBuffers();
}


static void ProcessKeys(unsigned char key, int x, int y)
{
    switch(key)
    {
    case 27:
        delete font;

        exit(EXIT_SUCCESS);
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
    font = new FTCustomHaloFont(file);


    font->FaceSize(160);
    font->CharMap(ft_encoding_unicode);



    // Run GLUT loop
    glutMainLoop();

    return EXIT_SUCCESS;
}

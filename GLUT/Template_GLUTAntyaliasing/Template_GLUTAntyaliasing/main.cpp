#include <GL/glew.h>
#include <iostream>
#include <GL/freeglut.h>
#include <assert.h>

using namespace std;
#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);
const int width = 800;
const int height = 600;

void  printOpenGLVersion();

/* reshaped window */
void reshape(int width, int height) {

    GLfloat fieldOfView = 90.0f;
    glViewport (0, 0, (GLsizei) width, (GLsizei) height);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    //gluPerspective(fieldOfView, (GLfloat) width/(GLfloat) height, 0.1, 500.0);

    glMatrixMode(GL_PROJECTION_MATRIX);
    glOrtho(0.0f,640,0.0f,480,1000.0f,-1000.0f);


    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void enableMultisample(int msaa)
{
    if (msaa)
    {
        glEnable(GL_MULTISAMPLE);
        glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);

        // detect current settings
        GLint iMultiSample = 0;
        GLint iNumSamples = 0;
        glGetIntegerv(GL_SAMPLE_BUFFERS, &iMultiSample);
        glGetIntegerv(GL_SAMPLES, &iNumSamples);
        printf("MSAA on, GL_SAMPLE_BUFFERS = %d, GL_SAMPLES = %d\n", iMultiSample, iNumSamples);
    }
    else
    {
        glDisable(GL_MULTISAMPLE);
        printf("MSAA off\n");
    }
}

static float angle = 0.0f;

/* render the scene */
void draw() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* render the scene here */

    glTranslatef(width/2,height/2,0);
    glRotatef(angle,0.0f,0.0f,1.0f);

    glBegin(GL_TRIANGLES);
    {
        glColor3f(1.0f,0.0f,1.0f);
        glVertex2f(-100.0f, -50.0f);

        glColor3f(0.0f,1.0f,0.0f);
        glVertex2f( 100.0f, -50.0f);

        glColor3f(0.0f,0.0f,1.0f);
        glVertex2f( 0.0f,150.0f);
    }
    glEnd();

    glFlush();
    glutSwapBuffers();
}


void timer(int value){
    glutTimerFunc(20,timer,0);
    angle += 1.0f;
}

void idle(){
    glutPostRedisplay();
}


/* initialize GLUT settings, register callbacks, enter main loop */
int main(int argc, char** argv) {

    glutInit(&argc, argv);

    glutSetOption(GLUT_MULTISAMPLE, 8);  //ANTYALIASIG
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE); //ANTYALIASING
    glutInitContextVersion (2, 0);
    glutInitContextFlags (GLUT_COMPATIBILITY_PROFILE | GLUT_DEBUG);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Perspective's GLUT Template");
    printOpenGLVersion(); //this function must be invoked after Create Window

    //KEYBOARD
    glutIgnoreKeyRepeat(true); // ignore keys held down

    glutReshapeFunc(reshape);
    glutDisplayFunc(draw);
    glutTimerFunc(100,timer,0);
    glutIdleFunc(idle);

    enableMultisample(0);

    glutMainLoop();
    return 0;
}

void printOpenGLVersion(){
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
    err = glGetError(); //w celu ignorowania błędu 1282 INVALID ENUM
    GL_CHECK_ERRORS

    //wyprowadzanie informacji na ekran
    cout<<"\tWersja GLEW "<<glewGetString(GLEW_VERSION)<<endl;
    cout<<"\tProducent: "<<glGetString (GL_VENDOR)<<endl;
    cout<<"\tRenderer: "<<glGetString (GL_RENDERER)<<endl;
    cout<<"\tWersja OpenGL: "<<glGetString (GL_VERSION)<<endl;
    cout<<"\tGLSL: "<<glGetString (GL_SHADING_LANGUAGE_VERSION)<<endl;
}


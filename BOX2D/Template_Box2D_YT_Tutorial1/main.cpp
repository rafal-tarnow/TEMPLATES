#define SDL 1
#define GLUT 2

#define WINDOW_ENGINE GLUT


#include <iostream>
#include <GL/glew.h>

#if WINDOW_ENGINE == SDL
    #include <SDL/SDL.h>
    #include <SDL/SDL_image.h>
#elif  WINDOW_ENGINE == GLUT
    #include <GL/freeglut.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <Box2D/Box2D.h>
#include <assert.h>
#include "texture_factory.hpp"

using namespace std;

const int WIDTH=640;
const int HEIGHT=480;
const float M2P=20;
const float P2M=1/M2P;
b2World* world;

GLint texture;

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

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


b2Body* addRect(int x,int y,int w,int h,bool dyn=true)
{
    b2BodyDef bodydef;
    bodydef.position.Set(x*P2M,y*P2M);
    if(dyn)
        bodydef.type=b2_dynamicBody;
    b2Body* body=world->CreateBody(&bodydef);

    b2PolygonShape shape;
    shape.SetAsBox(P2M*w/2,P2M*h/2);

    b2FixtureDef fixturedef;
    fixturedef.shape=&shape;
    fixturedef.density=1.0;
    body->CreateFixture(&fixturedef);

}

void drawSquare(b2Vec2* points,b2Vec2 center,float angle)
{
    glColor3f(1,1,1);
    glPushMatrix();
    glTranslatef(center.x*M2P,center.y*M2P,0);
    glRotatef(angle*180.0/M_PI,0,0,1);

    glBindTexture(GL_TEXTURE_2D, texture);

    glBegin(GL_QUADS);
    {

        glTexCoord2f(0.0f, 0.0f); glVertex2f(points[0].x*M2P,points[0].y*M2P);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(points[1].x*M2P,points[1].y*M2P);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(points[2].x*M2P,points[2].y*M2P);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(points[3].x*M2P,points[3].y*M2P);
    }
    glEnd();
    glPopMatrix();
}

void init()
{
    cout << "init()" << endl;
    glEnable(GL_MULTISAMPLE);
    glMatrixMode(GL_PROJECTION);
    glOrtho(0,WIDTH,HEIGHT,0,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_TEXTURE_2D);
    glClearColor(143.0/255.0,236.0/255.0,254.0/255.0,1);
    world=new b2World(b2Vec2(0.0,9.81));

    texture = GenerateTexture("./Data/images/skrzynka.png");

    addRect(WIDTH/6,HEIGHT-50,WIDTH,30,false);
}

void display()
{
    cout << "display()" << endl;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    b2Body* tmp=world->GetBodyList();
    b2Vec2 points[4];
    while(tmp)
    {
        for(int i=0;i<4;i++)
            points[i]=((b2PolygonShape*)tmp->GetFixtureList()->GetShape())->m_vertices[i];
        drawSquare(points,tmp->GetWorldCenter(),tmp->GetAngle());
        tmp=tmp->GetNext();
    }
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



#if WINDOW_ENGINE == SDL

int main(int argc,char** argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetVideoMode(WIDTH,HEIGHT,32,SDL_OPENGL);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);
    Uint32 start;
    SDL_Event event;
    bool running=true;
    init();
    while(running)
    {
        start=SDL_GetTicks();
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_QUIT:
                running=false;
                break;
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    running=false;
                    break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                addRect(event.button.x,event.button.y,40,40,true);
                break;

            }
        }
        display();
        world->Step(1.0/30.0,8,3);	//update
        SDL_GL_SwapBuffers();
        if(1000.0/30>SDL_GetTicks()-start)
            SDL_Delay(1000.0/30-(SDL_GetTicks()-start));
    }
    SDL_Quit();
}

#elif  WINDOW_ENGINE == GLUT




void reshape(int width, int height) {
    cout << "reshape()" << endl;
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

static float angle = 0.0f;

/* render the scene */
void draw() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* render the scene here */

    glTranslatef(WIDTH/2,HEIGHT/2,0);
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

void timer(int timer_id){
    glutTimerFunc(30,timer,0);
      angle += 1.0f;
     world->Step(1.0/30.0,8,3);
    glutPostRedisplay();
}

int main(int argc, char** argv) {

    glutInit(&argc, argv);

    glutSetOption(GLUT_MULTISAMPLE, 8);  //ANTYALIASIG
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE); //ANTYALIASING
    glutInitContextVersion (2, 0);
    glutInitContextFlags (GLUT_COMPATIBILITY_PROFILE | GLUT_DEBUG);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Perspective's GLUT Template");
    printOpenGLVersion(); //this function must be invoked after Create Window

    //KEYBOARD
    glutIgnoreKeyRepeat(true); // ignore keys held down

    glutReshapeFunc(reshape);
    glutDisplayFunc(draw);
    glutTimerFunc(100,timer,0);
    //glutIdleFunc(idle);

    enableMultisample(0);
    //init();

    glutMainLoop();
    return 0;
}
#endif

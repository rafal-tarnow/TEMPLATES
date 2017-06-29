//http://www.youtube.com/user/thecplusplusguy
//The Box2D main program with OpenGL and SDL
#include <iostream>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <Box2D/Box2D.h>
#include "texture_factory.hpp"

const int WIDTH=640;
const int HEIGHT=480;
const float M2P=20;
const float P2M=1/M2P;
b2World* world;

GLint texture;


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
    glClear(GL_COLOR_BUFFER_BIT);
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

int main(int argc,char** argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetVideoMode(640,480,32,SDL_OPENGL);
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

#pragma once

#include "FreeCamera/GLSLShader.h"
#include "FreeCamera/FreeCamera.h"
#include "FreeCamera/TexturedPlane.h"

#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
using namespace std;

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

//do ustalania dokładnoci obliczeń
const float EPSILON = 0.001f;
const float EPSILON2 = EPSILON*EPSILON;


class FreeCameraWrapper{

public:

    typedef enum{
        UP,
        DOWN
    }KeyEvent;

    FreeCameraWrapper();
    ~FreeCameraWrapper();

    void systemInput_OnResize(int w, int h);
    void systemInput_OnIdle();
    void systemInput_OnKeyEvent(unsigned char key, KeyEvent event, int x, int y);
    void systemInput_Render(float dt);
    void systemInput_OnMouseMove(int x, int y);
    void systemInput_OnMouseButtonEvent(int button, int event, int x, int y);

private:
    void filterMouseMoves(float dx, float dy);
    void prepareChessboard();
    void prepareCamera();

    const float MOUSE_FILTER_WEIGHT=0.75f;
#define MOUSE_HISTORY_BUFFER_SIZE 10
    glm::vec2 mouseHistory[MOUSE_HISTORY_BUFFER_SIZE];
    float mouseX=0, mouseY=0; //wygładzone wpółrzędne myszy
    bool useMouseFiltering = true;

    CFreeCamera cam;

    bool Key_w_Pressed = false;
    bool Key_s_Pressed = false;
    bool Key_a_Pressed = false;
    bool Key_d_Pressed = false;
    bool Key_q_Pressed = false;
    bool Key_e_Pressed = false;

    CTexturedPlane* checker_plane;

    //zmienne transformacyjne kamery
    int state = 0, oldX=0, oldY=0;
    float rX=0, rY=0, fov = 45;

    GLuint checkerTextureID;
    float m_dt = 0;


};

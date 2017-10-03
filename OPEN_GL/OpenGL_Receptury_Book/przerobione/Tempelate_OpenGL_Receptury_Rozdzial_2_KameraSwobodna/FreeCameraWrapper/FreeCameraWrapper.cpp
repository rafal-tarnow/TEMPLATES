#include "FreeCameraWrapper.hpp"

#include <iostream>

using namespace std;


FreeCameraWrapper::FreeCameraWrapper(){
    prepareChessboard();
    prepareCamera();
}

FreeCameraWrapper::~FreeCameraWrapper(){
    delete checker_plane;
    glDeleteTextures(1, &checkerTextureID);
    cout<<"Shutdown successfull"<<endl;
}

void FreeCameraWrapper::systemInput_OnResize(int w, int h) {
    cam.SetupProjection(45, (GLfloat)w/h);
}

void FreeCameraWrapper::systemInput_OnIdle() {

    //obsługa klawiszy W,S,A,D,Q i Z służšcych do poruszania kamerš
    if( Key_w_Pressed) {
        cam.Walk(m_dt);
    }

    if( Key_s_Pressed) {
        cam.Walk(-m_dt);
    }

    if(Key_a_Pressed) {
        cam.Strafe(-m_dt);
    }

    if(Key_d_Pressed) {
        cam.Strafe(m_dt);
    }

    if(Key_q_Pressed) {
        cam.Lift(m_dt);
    }

    if(Key_e_Pressed) {
        cam.Lift(-m_dt);
    }

    glm::vec3 t = cam.GetTranslation();
    if(glm::dot(t,t)>EPSILON2) {
        cam.SetTranslation(t*0.95f);
    }

}

void FreeCameraWrapper::systemInput_OnKeyEvent(unsigned char key, KeyEvent event,  int x, int y) {

    cout << "Key Down" << endl;

    if(event == DOWN){

        switch(key) {
        case ' ':
            useMouseFiltering = !useMouseFiltering;
            break;
        case 'w':
            Key_w_Pressed = true;
            break;
        case 's':
            Key_s_Pressed = true;
            break;
        case 'a':
            Key_a_Pressed = true;
            break;
        case 'd':
            Key_d_Pressed = true;
            break;
        case 'q':
            Key_q_Pressed = true;
            break;
        case 'e':
            Key_e_Pressed = true;
            break;
        }
    }else if(event == UP){
        switch(key) {
        case 'w':
            Key_w_Pressed = false;
            break;
        case 's':
            Key_s_Pressed = false;
            break;
        case 'a':
            Key_a_Pressed = false;
            break;
        case 'd':
            Key_d_Pressed = false;
            break;
        case 'q':
            Key_q_Pressed = false;
            break;
        case 'e':
            Key_e_Pressed = false;
            break;
        }
    }
}

void FreeCameraWrapper::systemInput_Render(float dt){

    m_dt = dt;

    //transformacje kamery
    glm::mat4 MV	= cam.GetViewMatrix();
    glm::mat4 P     = cam.GetProjectionMatrix();
    glm::mat4 MVP	= P*MV;

    //renderowanie płaskiej szachownicy
    checker_plane->Render(glm::value_ptr(MVP));

}

void FreeCameraWrapper::systemInput_OnMouseMove(int x, int y)
{
    if (state == 0) {
        fov += (y - oldY)/5.0f;
        cam.SetupProjection(fov, cam.GetAspectRatio());
    } else {
        rY -= (y - oldY)/5.0f;
        rX -= (oldX-x)/5.0f;
        if(useMouseFiltering)
            filterMouseMoves(rX, rY);
        else {
            mouseX = rX;
            mouseY = rY;
        }
        cam.Rotate(mouseX,mouseY, 0);
    }
    oldX = x;
    oldY = y;

}

void FreeCameraWrapper::systemInput_OnMouseButtonEvent(int button, int event, int x, int y)
{
    if (event == GLUT_DOWN)
    {
        oldX = x;
        oldY = y;
    }

    if(button == GLUT_MIDDLE_BUTTON)
        state = 0;
    else
        state = 1;
}

void FreeCameraWrapper::filterMouseMoves(float dx, float dy) {
    for (int i = MOUSE_HISTORY_BUFFER_SIZE - 1; i > 0; --i) {
        mouseHistory[i] = mouseHistory[i - 1];
    }

    // zapisywanie bieżšcego położenia myszy do bufora historii
    mouseHistory[0] = glm::vec2(dx, dy);

    float averageX = 0.0f;
    float averageY = 0.0f;
    float averageTotal = 0.0f;
    float currentWeight = 1.0f;

    // wygładzanie
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

void FreeCameraWrapper::prepareChessboard(){
    GL_CHECK_ERRORS
            GLubyte szachownica[128][128]={0};
    for(int j=0;j<128;j++) {
        for(int i=0;i<128;i++) {
            szachownica[i][j]=(i<=64 && j<=64 || i>64 && j>64 )?255:0;
        }
    }
    //generowanie obiektu szachownicy
    glGenTextures(1, &checkerTextureID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, checkerTextureID);
    //ustalanie parametrów szachownicy
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    GL_CHECK_ERRORS
            //ustawianie parametrów anizotropii
            GLfloat largest_supported_anisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);
    //ustawianie poziomów mipmapy, podstawowego i maksymalnego
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
    //alokowanie obiektu tekstury
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED, 128, 128, 0, GL_RED, GL_UNSIGNED_BYTE, szachownica);
    //generowanie mipmapy
    glGenerateMipmap(GL_TEXTURE_2D);
    GL_CHECK_ERRORS
            //tworzenie obiektu teksturowanej płaszczyzny
            checker_plane = new CTexturedPlane();
    GL_CHECK_ERRORS
}

void FreeCameraWrapper::prepareCamera(){
    //przygotowywanie kamery
    //ustalenie położenia i orientacji kamery
    glm::vec3 p = glm::vec3(5);
    cam.SetPosition(p);
    glm::vec3 look =  glm::normalize(p);

    //obracanie kamery we właciwym kierunku
    float yaw = glm::degrees(float(atan2(look.z, look.x)+M_PI));
    float pitch = glm::degrees(asin(look.y));
    rX = yaw;
    rY = pitch;
    if(useMouseFiltering) {
        for (int i = 0; i < MOUSE_HISTORY_BUFFER_SIZE ; ++i) {
            mouseHistory[i] = glm::vec2(rX, rY);
        }
    }
    cam.Rotate(rX,rY,0);
    cout<<"Inicjalizacja powiodla sie"<<endl;
}

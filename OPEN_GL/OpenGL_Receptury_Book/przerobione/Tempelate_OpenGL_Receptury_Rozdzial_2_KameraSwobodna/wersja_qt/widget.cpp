#include "widget.h"
#include <QKeyEvent>

#include <GL/freeglut.h>

Widget::Widget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    idleTimer = new QTimer();
    connect(idleTimer,SIGNAL(timeout()),this,SLOT(onIdleTimer()));
    idleTimer->start(33);
}


Widget::~Widget()
{

}

static const GLfloat g_vertex_buffer_data[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    0.0f,  1.0f, 0.0f,
};


static QString vertex_shader =
        "#version 330 core\r\n"
        "layout(location = 0) in vec3 vertexPosition_modelspace;"
        "void main(){"
        "gl_Position = vec4(vertexPosition_modelspace, 1);"
        "}";

static QString fragment_shader =
        "#version 330 core\r\n"
        "out vec3 color;"
        "void main(){"
        "color = vec3(1,0,0);"
        "}";

GLuint vertexbuffer;
GLuint VertexArrayID;

void Widget::initializeGL()
{
    makeCurrent();

    // To properly initialize all available OpenGL function pointers
    // and stops from segfaulting
    glewExperimental=true;
    GLenum err = glewInit();

    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        std::cerr << glewGetErrorString(err);
    }


    std::cerr << "using OpenGL " << format().majorVersion() << "." << format().minorVersion() << "\n";

   glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
   freeCameraWrapper = new FreeCameraWrapper();

}

void Widget::resizeGL(int w, int h)
{

    glViewport(0, 0, w, h);
    freeCameraWrapper->systemInput_OnResize(w,h);
    update();
}

void Widget::paintGL()
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    freeCameraWrapper->systemInput_Render(0.033);

}

void Widget::keyPressEvent(QKeyEvent *event){
    cout << "Widget::keyPressEvent(QKeyEvent *event)" << endl;
    freeCameraWrapper->systemInput_OnKeyEvent(event->key(), FreeCameraWrapper::DOWN,0,0);
    update();
}

void Widget::keyReleaseEvent(QKeyEvent *event){
    cout << "Widget::keyReleaseEvent(QKeyEvent *event)" << endl;
    freeCameraWrapper->systemInput_OnKeyEvent(event->key(), FreeCameraWrapper::UP,0,0);
    update();
}

void Widget::mouseMoveEvent(QMouseEvent *event){
    freeCameraWrapper->systemInput_OnMouseMove(event->x(),event->y());
}

void Widget::mousePressEvent(QMouseEvent *event){
     freeCameraWrapper->systemInput_OnMouseButtonEvent(GLUT_LEFT_BUTTON, GLUT_DOWN, event->x(), event->y());
}

void Widget::mouseReleaseEvent(QMouseEvent *event){
     freeCameraWrapper->systemInput_OnMouseButtonEvent(GLUT_LEFT_BUTTON ,GLUT_UP, event->x(), event->y());
}

void Widget::onIdleTimer(){
    cout << "On idle Timer" << endl;
    freeCameraWrapper->systemInput_OnIdle();
    update();
}

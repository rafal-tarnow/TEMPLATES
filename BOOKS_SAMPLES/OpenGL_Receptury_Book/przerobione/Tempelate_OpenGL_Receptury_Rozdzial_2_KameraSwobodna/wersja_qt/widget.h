#ifndef WIDGET_H
#define WIDGET_H

#include <GL/glew.h>
#include "FreeCameraWrapper/FreeCameraWrapper.hpp"

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QTimer>

#include <iostream>

class Widget : public QOpenGLWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

public slots:
    void onIdleTimer();

private:
    QTimer * idleTimer;
    QOpenGLShaderProgram program;
    FreeCameraWrapper * freeCameraWrapper;
};

#endif // WIDGET_H

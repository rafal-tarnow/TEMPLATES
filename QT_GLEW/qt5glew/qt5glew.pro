#-------------------------------------------------
#
# Project created by QtCreator 2015-06-17T10:02:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt5glew
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h

LIBS += -lGLEW


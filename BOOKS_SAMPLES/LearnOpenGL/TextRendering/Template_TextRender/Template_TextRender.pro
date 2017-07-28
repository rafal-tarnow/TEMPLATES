TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += /usr/include/freetype2

SOURCES += main.cpp

LIBS += -lfreetype

HEADERS += \
    Shader.h

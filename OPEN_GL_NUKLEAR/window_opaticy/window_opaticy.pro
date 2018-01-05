TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lglfw

SOURCES += main.cpp \
    glad.c

HEADERS += \
    glad.h \
    nuklear.h \
    nuklear_glfw_gl2.h \
    nuklear_glfw_gl3.h

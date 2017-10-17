TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lGLEW
LIBS += -lX11
LIBS += -lXrender
LIBS += -lGL
LIBS += -lm

DEFINES += "USE_GLX_CREATE_CONTEXT_ATTRIB=1"


SOURCES += main.c \
    glx_window.c \
    glx_context.c

HEADERS += \
    glx_window.h \
    glx_context.h

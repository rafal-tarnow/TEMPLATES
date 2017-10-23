TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lX11
LIBS += -lXrender
LIBS += -lGLEW
LIBS += -lm
LIBS += -lGL

DEFINES += "USE_GLX_CREATE_CONTEXT_ATTRIB"

SOURCES += main.c \
    glx_window.c \
    glx_context.c \
    drawing.c

HEADERS += \
    glx_window.h \
    glx_context.h \
    drawing.h

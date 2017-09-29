TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

copydata.commands = $(COPY_DIR) -r $$PWD/shadery $$OUT_PWD
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

LIBS += -lglut
LIBS += -lGL
LIBS += -lGLEW

SOURCES += main.cpp \
    AbstractCamera.cpp \
    FreeCamera.cpp \
    TexturedPlane.cpp \
    RenderableObject.cpp \
    Plane.cpp \
    GLSLShader.cpp

HEADERS += \
    GLSLShader.h \
    FreeCamera.h \
    AbstractCamera.h \
    Plane.h \
    TexturedPlane.h \
    RenderableObject.h

DISTFILES += \
    FreeCamera.cpp\

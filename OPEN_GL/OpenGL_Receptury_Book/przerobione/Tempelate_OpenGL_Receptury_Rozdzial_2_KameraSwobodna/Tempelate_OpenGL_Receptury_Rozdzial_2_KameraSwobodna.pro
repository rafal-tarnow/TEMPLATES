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
    FreeCameraWrapper/FreeCamera/AbstractCamera.cpp \
    FreeCameraWrapper/FreeCamera/FreeCamera.cpp \
    FreeCameraWrapper/FreeCamera/GLSLShader.cpp \
    FreeCameraWrapper/FreeCamera/Plane.cpp \
    FreeCameraWrapper/FreeCamera/RenderableObject.cpp \
    FreeCameraWrapper/FreeCamera/TexturedPlane.cpp \
    FreeCameraWrapper/FreeCameraWrapper.cpp

HEADERS += \
    FreeCameraWrapper/FreeCamera/AbstractCamera.h \
    FreeCameraWrapper/FreeCamera/FreeCamera.h \
    FreeCameraWrapper/FreeCamera/GLSLShader.h \
    FreeCameraWrapper/FreeCamera/Plane.h \
    FreeCameraWrapper/FreeCamera/RenderableObject.h \
    FreeCameraWrapper/FreeCamera/TexturedPlane.h \
    FreeCameraWrapper/FreeCameraWrapper.hpp

DISTFILES += \
    FreeCamera.cpp\
    shadery/checker_shader.frag \
    shadery/shader.frag \
    shadery/checker_shader.vert \
    shadery/shader.vert

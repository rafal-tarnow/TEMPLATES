TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

copydata.commands = $(COPY_DIR) -r $$PWD/../common/shadery $$OUT_PWD
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

LIBS += -lglut
LIBS += -lGL
LIBS += -lGLEW

INCLUDEPATH +=$$PWD/../common/

SOURCES += main.cpp \
    ../common/FreeCameraWrapper/FreeCamera/AbstractCamera.cpp \
    ../common/FreeCameraWrapper/FreeCamera/FreeCamera.cpp \
    ../common/FreeCameraWrapper/FreeCamera/GLSLShader.cpp \
    ../common/FreeCameraWrapper/FreeCamera/Plane.cpp \
    ../common/FreeCameraWrapper/FreeCamera/RenderableObject.cpp \
    ../common/FreeCameraWrapper/FreeCamera/TexturedPlane.cpp \
    ../common/FreeCameraWrapper/FreeCameraWrapper.cpp

HEADERS += \
    ../common/FreeCameraWrapper/FreeCamera/AbstractCamera.h \
    ../common/FreeCameraWrapper/FreeCamera/FreeCamera.h \
    ../common/FreeCameraWrapper/FreeCamera/GLSLShader.h \
    ../common/FreeCameraWrapper/FreeCamera/Plane.h \
    ../common/FreeCameraWrapper/FreeCamera/RenderableObject.h \
    ../common/FreeCameraWrapper/FreeCamera/TexturedPlane.h \
    ../common/FreeCameraWrapper/FreeCameraWrapper.hpp

DISTFILES += \
    FreeCamera.cpp\
    shadery/checker_shader.frag \
    shadery/shader.frag \
    shadery/checker_shader.vert \
    shadery/shader.vert \
    ../common/shadery/checker_shader.frag \
    ../common/shadery/checker_shader.vert

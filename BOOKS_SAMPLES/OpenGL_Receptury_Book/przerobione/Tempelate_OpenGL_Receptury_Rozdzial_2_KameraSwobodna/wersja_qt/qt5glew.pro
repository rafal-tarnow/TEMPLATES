#-------------------------------------------------
#
# Project created by QtCreator 2015-06-17T10:02:53
#
#-------------------------------------------------

QT       += core opengl gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt5glew
TEMPLATE = app

copydata.commands = $(COPY_DIR) -r $$PWD/../common/shadery $$OUT_PWD
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

SOURCES += main.cpp\
	widget.cpp \
    ../common/FreeCameraWrapper/FreeCamera/AbstractCamera.cpp \
    ../common/FreeCameraWrapper/FreeCamera/FreeCamera.cpp \
    ../common/FreeCameraWrapper/FreeCamera/GLSLShader.cpp \
    ../common/FreeCameraWrapper/FreeCamera/Plane.cpp \
    ../common/FreeCameraWrapper/FreeCamera/RenderableObject.cpp \
    ../common/FreeCameraWrapper/FreeCamera/TexturedPlane.cpp \
    ../common/FreeCameraWrapper/FreeCameraWrapper.cpp

HEADERS  += widget.h \
    ../common/FreeCameraWrapper/FreeCamera/AbstractCamera.h \
    ../common/FreeCameraWrapper/FreeCamera/FreeCamera.h \
    ../common/FreeCameraWrapper/FreeCamera/GLSLShader.h \
    ../common/FreeCameraWrapper/FreeCamera/Plane.h \
    ../common/FreeCameraWrapper/FreeCamera/RenderableObject.h \
    ../common/FreeCameraWrapper/FreeCamera/TexturedPlane.h \
    ../common/FreeCameraWrapper/FreeCameraWrapper.hpp

INCLUDEPATH += $$PWD/../common

LIBS += -lGLEW

DISTFILES += \
    shadery/checker_shader.frag \
    shadery/checker_shader.vert


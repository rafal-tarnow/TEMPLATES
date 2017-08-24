TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


copydata.commands = $(COPY_DIR) -r $$PWD/shadery $$OUT_PWD
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

LIBS += -lGLEW
LIBS += -lGL
LIBS += -lglut

SOURCES += main.cpp \
    GLSLShader.cpp

HEADERS += \
    GLSLShader.h

DISTFILES += \
    shadery/shader.frag \
    shadery/shader.vert

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

#COPY DATA FROM ./data FOLDER TO BUILD DIRECOTRY
copydata.commands = $(COPY_DIR) -r $$PWD/data $$OUT_PWD
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata


INCLUDEPATH += /usr/include/freetype2/

LIBS += -lglut
LIBS += -lGL
LIBS += -lGLEW
LIBS += -lGLU
LIBS += -lftgl

SOURCES += main.cpp

DISTFILES += \
    data/font/arial.ttf

HEADERS += \
    config.h

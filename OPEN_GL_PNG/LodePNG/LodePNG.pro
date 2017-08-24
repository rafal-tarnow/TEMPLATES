TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


copydata.commands = $(COPY_DIR) -r $$PWD/data $$OUT_PWD
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata


LIBS += -lSDL
LIBS += -lGL

SOURCES += main.cpp \
    lodepng.cpp

HEADERS += \
    lodepng.h

DISTFILES += \
    data/png/bg.png \
    data/png/car.png \
    data/png/coin_2.png \
    data/png/coin_3.png \
    data/png/hero.png \
    data/png/kolo.png

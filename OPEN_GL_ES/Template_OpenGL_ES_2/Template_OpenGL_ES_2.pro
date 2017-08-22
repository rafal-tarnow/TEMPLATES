TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

copydata.commands = $(COPY_DIR) -r $$PWD/data $$OUT_PWD
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

LIBS += -lGLESv2
LIBS += -lglfw
LIBS += -lSOIL

SOURCES += main.cpp \
    texture_manager.cpp

HEADERS += \
    texture_manager.hpp

DISTFILES += \
    data/font/arial.ttf \
    data/font/dahot_Garfield_www_myfontfree_com.ttf \
    data/font/design_graffiti_agentorange_www_myfontfree_com.ttf \
    data/png/bg.png \
    data/png/car.png \
    data/png/coin_2.png \
    data/png/coin_3.png \
    data/png/hero.png \
    data/png/kolo.png

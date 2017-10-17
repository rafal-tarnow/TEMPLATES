TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

target.path=/opt/$${TARGET}/bin
INSTALLS += target

SOURCES += main.c

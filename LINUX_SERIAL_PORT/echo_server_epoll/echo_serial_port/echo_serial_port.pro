TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    serialPort/serialport.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    serialPort/serialport.hpp


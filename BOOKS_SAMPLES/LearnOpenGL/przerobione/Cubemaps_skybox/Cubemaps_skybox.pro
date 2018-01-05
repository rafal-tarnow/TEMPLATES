TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lglfw
LIBS += -ldl

copydata.commands = $(COPY_DIR) -r $$PWD/data $$OUT_PWD
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

SOURCES += cubemaps_skybox.cpp \
    glad.c \
    stb_image.cpp

HEADERS += \
    glad.h \
    stb_image.h \
    learnopengl/camera.h \
    learnopengl/filesystem.h \
    learnopengl/mesh.h \
    learnopengl/model.h \
    learnopengl/shader.h \
    learnopengl/shader_m.h \
    learnopengl/shader_s.h \
    root_directory.h

DISTFILES += \
    data/shadery/6.1.cubemaps.fs \
    data/shadery/6.1.skybox.fs \
    data/shadery/6.1.cubemaps.vs \
    data/shadery/6.1.skybox.vs

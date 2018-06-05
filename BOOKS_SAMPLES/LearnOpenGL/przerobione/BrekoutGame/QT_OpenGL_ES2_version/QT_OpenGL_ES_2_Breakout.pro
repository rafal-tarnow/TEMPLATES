TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


copyfolder.commands = $(COPY_DIR) -r $$PWD/data $$OUT_PWD
first.depends = $(first) copyfolder
export(first.depends)
export(copyfolder.commands)
QMAKE_EXTRA_TARGETS += first copyfolder

INCLUDEPATH += $$PWD/include
INCLUDEPATH += /usr/include/freetype2/


LIBS += -lglfw
LIBS += -lSOIL
LIBS += -lGLESv2
LIBS += -lfreetype
LIBS += -lpthread

SOURCES += main.cpp \
    Ball.cpp \
    GameController.cpp \
    GameLevel.cpp \
    GameObject.cpp \
    Particle.cpp \
    PostProcessor.cpp \
    PowerUp.cpp \
    Shader.cpp \
    SpriteRenderer.cpp \
    TextRenderer.cpp \
    Texture2D.cpp \
    ResourceManager.cpp

HEADERS += \
    GameController.hpp \
    Particle.hpp \
    Shader.hpp \
    Ball.hpp \
    GameLevel.hpp \
    GameObject.hpp \
    PostProcessor.hpp \
    PowerUp.hpp \
    SpriteRenderer.hpp \
    TextRenderer.hpp \
    Texture2D.hpp \
    ResourceManager.hpp \
    opengl_includes.hpp

DISTFILES += \
    data/shaders/particle.vs \
    data/shaders/post_processing.vs \
    data/shaders/sprite.vs \
    data/shaders/text.vs \
    data/shaders/particle.frag \
    data/shaders/post_processing.frag \
    data/shaders/sprite.frag \
    data/shaders/text.frag \
    data/fonts/arial.ttf \
    data/images/background.jpg \
    data/images/block.png \
    data/images/block_solid.png \
    data/images/paddle.png \
    data/images/particle.png \
    data/images/powerup_chaos.png \
    data/images/powerup_confuse.png \
    data/images/powerup_increase.png \
    data/images/powerup_passthrough.png \
    data/images/powerup_speed.png \
    data/images/powerup_sticky.png \
    data/images/sheep.png \
    data/wavs/bleep.mp3 \
    data/wavs/breakout.mp3 \
    data/wavs/powerup.wav \
    data/wavs/solid.wav \
    data/levels/four.lvl \
    data/levels/one.lvl \
    data/levels/three.lvl \
    data/levels/two.lvl

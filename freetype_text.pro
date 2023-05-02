TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

CONFIG(release, debug|release) {
    #This is a release build
    DEFINES += NDEBUG
    QMAKE_CXXFLAGS += -s
} else {
    #This is a debug build
    DEFINES += DEBUG
    TARGET = $$join(TARGET,,,_d)
}

#QMAKE_CXXFLAGS += -std=c++17 -Wno-unused-parameter -Wconversion -Wold-style-cast

DESTDIR = $$PWD/bin

INCLUDEPATH += $$PWD/include

LIBS += -L$$PWD/lib

win32:{
    INCLUDEPATH += $$PWD/include/freetype
    LIBS += -lopengl32 -lglu32 -lgdi32 -lglew32dll -lglfw3dll
    LIBS += -lfreetype -static-libgcc -static-libstdc++ -static -lpthread
}
unix:{
    INCLUDEPATH += /usr/include/freetype2/
    LIBS += -lglfw -lfreetype -lGL -lGLEW
}

SOURCES += \
    src/AtlasTex.cpp \
    src/Shader.cpp \
    src/TexFont.cpp \
    src/TextRender.cpp \
    src/Tga.cpp \
    src/VertexAttrib.cpp \
    src/VertexBuffer.cpp \
    src/main.cpp \
    src/rect2d.cpp \
    src/utf8_utils.cpp

HEADERS += \
    src/AtlasTex.h \
    src/Shader.h \
    src/TexFont.h \
    src/TextRender.h \
    src/Tga.h \
    src/VertexAttrib.h \
    src/VertexBuffer.h \
    src/rect2d.h \
    src/utf8_utils.h

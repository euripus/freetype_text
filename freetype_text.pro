TEMPLATE = app
CONFIG += console #c++11
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

DESTDIR = $$PWD/bin

INCLUDEPATH += $$PWD/include

LIBS += -L$$PWD/lib

win32:{
    LIBS += -lopengl32 -lglu32 -lgdi32 -lglew32 -lglfw3dll
    LIBS += -lfreetype252
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
    src/main.cpp

HEADERS += \
    src/AtlasTex.h \
    src/Shader.h \
    src/TexFont.h \
    src/TextRender.h \
    src/Tga.h \
    src/VertexAttrib.h \
    src/VertexBuffer.h

TEMPLATE = app
CONFIG += console
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

QMAKE_CXXFLAGS += -std=c++17 -Wno-unused-parameter -Wold-style-cast

DESTDIR = $$PWD/bin

INCLUDEPATH += $$PWD/include

LIBS += -L$$PWD/lib

win32:{
    INCLUDEPATH += $$PWD/include/freetype $$PWD/../libs/boost_1_82_0
    LIBS += -L$$PWD/../libs/boost_1_82_0/stage/lib
    LIBS += -lopengl32 -lglu32 -lgdi32 -lglew32dll -lglfw3dll -lzlibdll
    LIBS += -lfreetype.dll -static-libgcc -static-libstdc++ -static -lpthread
    LIBS += -lboost_json-mgw8-mt-x32-1_82
}
unix:{
    INCLUDEPATH += /usr/include/freetype2/
    LIBS += -lglfw -lfreetype -lGL -lGLEW
    LIBS += -lboost_json
}

SOURCES += \
    src/Shader.cpp \
    src/VertexAttrib.cpp \
    src/VertexBuffer.cpp \
    src/gui/atlastex.cpp \
    src/gui/fontmanager.cpp \
    src/gui/imagedata.cpp \
    src/gui/packer.cpp \
    src/gui/rect2d.cpp \
    src/gui/texfont.cpp \
    src/gui/ui.cpp \
    src/gui/uiimagemanager.cpp \
    src/gui/utf8_utils.cpp \
    src/gui/widget.cpp \
    src/gui/window.cpp \
    src/main.cpp

HEADERS += \
    src/Shader.h \
    src/VertexAttrib.h \
    src/VertexBuffer.h \
    src/gui/atlastex.h \
    src/gui/basic_types.h \
    src/gui/fontmanager.h \
    src/gui/imagedata.h \
    src/gui/packer.h \
    src/gui/rect2d.h \
    src/gui/texfont.h \
    src/gui/ui.h \
    src/gui/uiimagemanager.h \
    src/gui/utf8_utils.h \
    src/gui/widget.h \
    src/gui/window.h

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
    # QMAKE_CXXFLAGS += -fsanitize=address  -fsanitize=leak -g
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
    LIBS += -lfreetype -static-libgcc -static-libstdc++ -static -lpthread
    LIBS += -lboost_json-mgw13-mt-x64-1_82
}
unix:{
    INCLUDEPATH += /usr/include/freetype2/
    LIBS += -lglfw -lfreetype -lGL -lGLEW
    LIBS += -lboost_json -lz
}

SOURCES += \
    src/fs/file.cpp \
    src/fs/file_system.cpp \
    src/fs/memory_stream.cpp \
    src/gui/atlastex.cpp \
    src/gui/button.cpp \
    src/gui/chain.cpp \
    src/gui/fontmanager.cpp \
    src/gui/imagedata.cpp \
    src/gui/packer.cpp \
    src/gui/rect2d.cpp \
    src/gui/texfont.cpp \
    src/gui/text_box.cpp \
    src/gui/text_fitter.cpp \
    src/gui/ui.cpp \
    src/gui/uiconfigloader.cpp \
    src/gui/uiimagemanager.cpp \
    src/gui/utf8_utils.cpp \
    src/gui/widget.cpp \
    src/gui/window.cpp \
    src/input/input.cpp \
    src/main.cpp \
    src/vertex_buffer.cpp

HEADERS += \
    src/fs/file.h \
    src/fs/file_system.h \
    src/fs/memory_stream.h \
    src/fs/zip.h \
    src/gui/atlastex.h \
    src/gui/basic_types.h \
    src/gui/button.h \
    src/gui/chain.h \
    src/gui/fontmanager.h \
    src/gui/imagedata.h \
    src/gui/packer.h \
    src/gui/rect2d.h \
    src/gui/texfont.h \
    src/gui/text_box.h \
    src/gui/text_fitter.h \
    src/gui/ui.h \
    src/gui/uiconfigloader.h \
    src/gui/uiimagemanager.h \
    src/gui/utf8_utils.h \
    src/gui/widget.h \
    src/gui/window.h \
    src/input/input.h \
    src/vertex_buffer.h

DISTFILES += \
    bin/data/ui/jsons/ui_res.json \
    bin/data/ui/jsons/vert_win.json

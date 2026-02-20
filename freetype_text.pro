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

QMAKE_CXXFLAGS += -std=c++17 -Wno-unused-parameter -Wold-style-cast -Wuninitialized -Wpedantic -Wfloat-equal
#-Wdouble-promotion

DESTDIR = $$PWD/bin

INCLUDEPATH += $$PWD/include

LIBS += -L$$PWD/lib

win32:{
    INCLUDEPATH += $$PWD/include/freetype $$PWD/../boost_1_89_0
    LIBS += -L$$PWD/../boost_1_89_0/stage/lib
    LIBS += -lopengl32 -lglu32 -lgdi32 -lglew32dll -lglfw3 -lzlibdll
    LIBS += -lfreetype -static-libgcc -static-libstdc++ -static -lpthread
    LIBS += -lboost_json-mgw17-mt-x64-1_89
}
unix:{
    INCLUDEPATH += /usr/include/freetype2/
    LIBS += -lglfw -lfreetype -lGL -lGLEW
    LIBS += -lboost_json -lz
}

SOURCES +=  \
    src/fs/file.cpp \
    src/fs/file_system.cpp \
    src/fs/memory_stream.cpp \
    src/gui/button.cpp \
    src/gui/imagebox.cpp \
    src/gui/packer.cpp \
    src/gui/text_box.cpp \
    src/gui/text_fitter.cpp \
    src/gui/ui.cpp \
    src/gui/uiconfigloader.cpp \
    src/gui/uiimagemanager.cpp \
    src/gui/uiwindow.cpp \
    src/gui/utils/atlastex.cpp \
    src/gui/utils/chain.cpp \
    src/gui/utils/fontmanager.cpp \
    src/gui/utils/rect2d.cpp \
    src/gui/utils/texfont.cpp \
    src/gui/utils/utf8_utils.cpp \
    src/gui/widget.cpp \
    src/input/input.cpp \
    src/input/inputglfw.cpp \
    src/main.cpp \
    src/render/renderer.cpp \
    src/render/texture.cpp \
    src/render/vertex_buffer.cpp \
    src/res/imagedata.cpp \
    src/window.cpp

HEADERS +=  \
    src/fs/file.h \
    src/fs/file_system.h \
    src/fs/memory_stream.h \
    src/fs/zip.h \
    src/gui/basic_types.h \
    src/gui/button.h \
    src/gui/imagebox.h \
    src/gui/packer.h \
    src/gui/text_box.h \
    src/gui/text_fitter.h \
    src/gui/ui.h \
    src/gui/uiconfigloader.h \
    src/gui/uiimagemanager.h \
    src/gui/uiwindow.h \
    src/gui/utils/atlastex.h \
    src/gui/utils/chain.h \
    src/gui/utils/fontmanager.h \
    src/gui/utils/rect2d.h \
    src/gui/utils/texfont.h \
    src/gui/utils/utf8_utils.h \
    src/gui/widget.h \
    src/input/input.h \
    src/input/inputglfw.h \
    src/input/key_codes.h \
    src/render/AABB.h \
    src/render/render_states.h \
    src/render/renderer.h \
    src/render/texture.h \
    src/render/vertex_buffer.h \
    src/res/imagedata.h \
    src/scene_data.h \
    src/window.h

DISTFILES += \
    bin/data/ui/jsons/ui_res.json \
    bin/data/ui/jsons/vert_win.json

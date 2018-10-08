
QT       += core
QT       += gui
QT       += xml
QT       += widgets
QT       += multimedia
QT       += multimediawidgets
QT       += opengl


include(../qroilib/qroilib.pri)
#include(../serial/qextserialport.pri)

win32 {

DEFINES += Q_OS_WIN
LIBS += -L"..\winlib"
LIBS += -L"..\qroilib\pthread"

CONFIG(debug, debug|release) {
#LIBS += -lopencv_core320d -lopencv_imgproc320d -lopencv_highgui320d -lopencv_videoio320d -lopencv_imgcodecs320d -lopencv_shape320d
LIBS += -lopencv_world340d
#LIBS += "..\winlib\Debug\qroilib.lib"
LIBS += "..\winlib\Debug\jpeg.lib"
LIBS += "..\winlib\Debug\png.lib"
LIBS += "..\winlib\Debug\zlib.lib"
LIBS += "..\winlib\Debug\lcms2.lib"
LIBS += "..\winlib\Debug\QZXing2.lib"
}
CONFIG(release, debug|release) {
#LIBS += -lopencv_core320 -lopencv_imgproc320 -lopencv_highgui320 -lopencv_videoio320  -lopencv_imgcodecs320 -lopencv_shape320
LIBS += -lopencv_world340
LIBS += "..\winlib\Release\qroilib.lib"
LIBS += "..\winlib\Release\jpeg.lib"
LIBS += "..\winlib\Release\png.lib"
LIBS += "..\winlib\Release\zlib.lib"
LIBS += "..\winlib\Release\lcms2.lib"
LIBS += "..\winlib\Release\QZXing2.lib"
}
LIBS += -lws2_32
LIBS += -lpthreadVC1

INCLUDEPATH += "..\winlib\include"
INCLUDEPATH += "..\qroilib\pthread"
#INCLUDEPATH += "c:\opencv340\build\include"
TARGET = ../../bin/qroilcdmeasure
}

linux {
QMAKE_CXXFLAGS += -std=c++11 -pthread
LIBS += -ljpeg -lexiv2 -lpng -lz -llcms2 -lX11 -lGLU
LIBS += -L/usr/local/lib
LIBS += -L/usr/lib
LIBS += -lopencv_core
LIBS += -lopencv_imgproc
LIBS += -lopencv_highgui
LIBS += -lopencv_videoio
LIBS += -lopencv_imgcodecs
LIBS += -lopencv_ml
LIBS += -lopencv_shape
LIBS += -lopencv_features2d
LIBS += -lopencv_xfeatures2d
LIBS += -lopencv_calib3d
LIBS += -lgstreamer-1.0
LIBS += -lgobject-2.0
LIBS += -lglib-2.0
LIBS += -lv4l2
LIBS += -lqroilib
LIBS += -lQZXing

INCLUDEPATH += /usr/local/include
TARGET = ../bin/qroilcdmeasure
}

TEMPLATE = app

INCLUDEPATH += . \
   ./app \
   ../qroilib \
   ../qroilib/engine \
   ../qroilib/qroilib \
   ../qroilib/qroilib/document \
   ../qroilib/qroilib/roilib \
   ../qroilib/qtpropertybrowser/src \
   ../serial \
   ../qzxing/src \
   ../qzxing/src/zxing \

# Input

HEADERS +=  \
        app/mainwindow.h \
        app/viewmainpage.h \
        app/viewoutpage.h \
        app/common.h \
        app/camcapture.h \
        app/imgprocengine.h \
        app/recipedata.h \
        app/roipropertyeditor.h \
        app/dialogcamera.h \
        app/outwidget.h \


FORMS += \
        app/dialogcamera.ui \


SOURCES +=  \
        app/main.cpp \
        app/mainwindow.cpp \
        app/viewmainpage.cpp \
        app/viewoutpage.cpp \
        app/common.cpp \
        app/imgprocengine.cpp \
        app/camcapture.cpp \
        app/recipedata.cpp \
        app/roipropertyeditor.cpp \
        app/dialogcamera.cpp \
        app/outwidget.cpp \


RESOURCES += \
   resources.qrc


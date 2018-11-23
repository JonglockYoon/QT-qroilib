
QT       += core
QT       += gui
QT       += xml
QT       += widgets
QT       += multimedia
QT       += multimediawidgets
QT       += opengl


#include(../qroilib/qroilib.pri)
#include(../serial/qextserialport.pri)

win32 {

DEFINES += Q_OS_WIN
LIBS += -L"..\winlib"
LIBS += -L"..\qroilib\pthread"
#LIBS += -L"c:\opencv340\build\x64\vc15\lib"

CONFIG(debug, debug|release) {
#LIBS += -lopencv_core320d -lopencv_imgproc320d -lopencv_highgui320d -lopencv_videoio320d -lopencv_imgcodecs320d -lopencv_shape320d
LIBS += -lopencv_world340d
LIBS += "..\winlib\Debug\qroilib.lib"
LIBS += "..\winlib\Debug\jpeg.lib"
LIBS += "..\winlib\Debug\png.lib"
LIBS += "..\winlib\Debug\zlib.lib"
LIBS += "..\winlib\Debug\lcms2.lib"
LIBS += "..\winlib\Debug\QZXing2.lib"
TARGET = ../../bin/qroisimulator
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
TARGET = ../../bin/release/qroisimulator
}
LIBS += -lws2_32
LIBS += -lpthreadVC1

INCLUDEPATH += "..\winlib\include"
INCLUDEPATH += "..\qroilib\pthread"
#INCLUDEPATH += "c:\opencv340\build\include"
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
LIBS += -lopencv_stitching
LIBS += -lgstreamer-1.0
LIBS += -lgobject-2.0
LIBS += -lglib-2.0
LIBS += -lv4l2
LIBS += -lqroilib
LIBS += -lQZXing

INCLUDEPATH += /usr/local/include
TARGET = ../bin/qroisimulator
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
        app/recipedata.h \
        app/roipropertyeditor.h \
        app/dialogcamera.h \
        app/outwidget.h \
        app/dialogedge.h \
        app/dialogthreshold.h \
        app/dialogmorphology.h \
        app/dialogcontour.h \
        app/dialogblob.h \
        app/dialogarithmeticlogic.h \
        app/dialogcircle.h \
        app/dialoglinefit.h \
        app/dialogapplication.h \
        app/dialogapitest.h
    app/geomatch.h

FORMS += \
        app/dialogcamera.ui \
        app/dialogedge.ui \
        app/dialogthreshold.ui \
        app/dialogmorphology.ui \
        app/dialogarithmeticlogic.ui \
        app/dialogcontour.ui \
        app/dialogblob.ui \
        app/dialoglinefit.ui \
        app/dialogcircle.ui \
        app/dialogapplication.ui \
    app/dialogapitest.ui


SOURCES +=  \
        app/main.cpp \
        app/mainwindow.cpp \
        app/viewmainpage.cpp \
        app/viewoutpage.cpp \
        app/common.cpp \
        app/camcapture.cpp \
        app/recipedata.cpp \
        app/roipropertyeditor.cpp \
        app/dialogcamera.cpp \
        app/outwidget.cpp \
        app/dialogedge.cpp \
        app/dialogthreshold.cpp \
        app/dialogmorphology.cpp \
        app/dialogcontour.cpp \
        app/dialogblob.cpp \
        app/dialogarithmeticlogic.cpp \
        app/dialogcircle.cpp \
        app/dialoglinefit.cpp \
        app/dialogapplication.cpp \
        app/dialogapitest.cpp \
    app/geomatch.cpp

RESOURCES += \
   resources.qrc


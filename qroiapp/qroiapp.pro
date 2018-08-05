
QT       += core
QT       += gui
QT       += xml
QT       += widgets
QT       += multimedia
QT       += multimediawidgets
QT       += opengl
QT       += charts

#include(../qroilib/qroilib.pri)
#include(../serial/qextserialport.pri)

win32 {

DEFINES += Q_OS_WIN
LIBS += -L"..\winlib"

CONFIG(debug, debug|release) {
#LIBS += -lopencv_core320d -lopencv_imgproc320d -lopencv_highgui320d -lopencv_videoio320d -lopencv_imgcodecs320d
LIBS += -L..\winlib -lopencv_world340d
LIBS += "..\winlib\Debug\qroilib.lib"
LIBS += "..\winlib\Debug\qextserial.lib"
LIBS += "..\winlib\Debug\jpeg.lib"
LIBS += "..\winlib\Debug\png.lib"
LIBS += "..\winlib\Debug\zlib.lib"
LIBS += "..\winlib\Debug\lcms2.lib"
}
CONFIG(release, debug|release) {
#LIBS += -lopencv_core320 -lopencv_imgproc320 -lopencv_highgui320 -lopencv_videoio320  -lopencv_imgcodecs320
LIBS += -L..\winlib -lopencv_world340
LIBS += "..\winlib\Release\qroilib.lib"
LIBS += "..\winlib\Release\qextserial.lib"
LIBS += "..\winlib\Release\jpeg.lib"
LIBS += "..\winlib\Release\png.lib"
LIBS += "..\winlib\Release\zlib.lib"
LIBS += "..\winlib\Release\lcms2.lib"
}
LIBS += -lws2_32

INCLUDEPATH += "..\winlib\include"
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
LIBS += -lgstreamer-1.0
LIBS += -lgobject-2.0
LIBS += -lglib-2.0
LIBS += -lv4l2
LIBS += -lqroilib
LIBS += -lqextserial

INCLUDEPATH += /usr/local/include
}

LDFLAGS=-L # is this even necessary?

TEMPLATE = app
TARGET = qroiapp

TRANSLATIONS = qroiapp_ko.ts qroiapp_en.ts

INCLUDEPATH += . \
   ./app \
   ./qtsingleapplication/src \
   ../qroilib \
   ../qroilib/engine \
   ../qroilib/qroilib \
   ../qroilib/qroilib/document \
   ../qroilib/qroilib/roilib \
   ../qroilib/qtpropertybrowser/src \
   ../serial \

# Input

HEADERS +=  \
        app/frminput.h \
        app/frmnum.h \
        app/mainwindow.h \
        app/viewmainpage.h \
        app/camcapture.h \
        app/recipedata.h \
        app/common.h \
        app/dialogmodel.h \
        app/config.h \
        app/roipropertyeditor.h \
        app/interfaceport.h \
        app/lightplustek.h \
        app/imgprocengine.h \
        app/dialogconfig.h \
        app/logviewdock.h \
        app/mlogthread.h \
        app/teachdock.h \
        app/minfoteachmanager.h \
        app/positionsdock.h \
        app/aligngraphdock.h \
        qtsingleapplication/src/qtsingleapplication.h \
        qtsingleapplication/src/qtlocalpeer.h \
        app/mtrsbase.h \
        app/mtrsalign.h \
        app/mticktimer.h \


FORMS += app/advancedconfigpage.ui \
         app/generalconfigpage.ui \
         app/imageviewconfigpage.ui \
         app/dialogmodel.ui \
         app/dialogconfig.ui \
         app/mainwindow.ui \
         app/frminput.ui \
         app/frmnum.ui \
         app/teachdock.ui

SOURCES +=  \
        app/frminput.cpp \
        app/frmnum.cpp \
        app/main.cpp \
        app/mainwindow.cpp \
        app/viewmainpage.cpp \
        app/camcapture.cpp \
        app/recipedata.cpp \
        app/common.cpp \
        app/dialogmodel.cpp \
        app/config.cpp \
        app/roipropertyeditor.cpp \
        app/interfaceport.cpp \
        app/lightplustek.cpp \
        app/imgprocengine.cpp \
        app/dialogconfig.cpp \
        app/logviewdock.cpp \
        app/mlogthread.cpp \
        app/teachdock.cpp \
        app/minfoteachmanager.cpp \
        app/positionsdock.cpp \
        app/aligngraphdock.cpp \
        qtsingleapplication/src/qtsingleapplication.cpp \
        qtsingleapplication/src/qtlocalpeer.cpp \
        app/mtrsbase.cpp \
        app/mtrsalign.cpp \
        app/mticktimer.cpp \

RESOURCES += \
    resources.qrc


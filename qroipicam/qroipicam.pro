
QT       += core
QT       += gui
QT       += xml
QT       += widgets
QT       += multimedia
QT       += multimediawidgets
QT       += opengl


#include(../qroilib/qroilib.pri)
#include(../serial/qextserialport.pri)

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
#LIBS += -lraspicam
#LIBS += -lraspicam_cv

INCLUDEPATH += /usr/local/include
}

#LDFLAGS=-L # is this even necessary?

TEMPLATE = app
TARGET = qroipicam

INCLUDEPATH += . \
   ./app \
   ../qroilib \
   ../qroilib/engine \
   ../qroilib/qroilib \
   ../qroilib/qroilib/document \
   ../qroilib/qroilib/roilib \
   ../qroilib/qtpropertybrowser/src \
   ../serial \


# Input

HEADERS +=  \
        app/mainwindow.h \
        app/viewmainpage.h \
        app/common.h \
        app/leftdock.h \
        app/picapture.h \
        app/outwidget.h \
        app/viewoutpage.h

FORMS += \
        app/leftdock.ui

SOURCES +=  \
        app/main.cpp \
        app/mainwindow.cpp \
        app/viewmainpage.cpp \
        app/common.cpp \
        app/leftdock.cpp \
        app/picapture.cpp \
        app/outwidget.cpp \
        app/viewoutpage.cpp

RESOURCES +=



TEMPLATE = lib
TARGET = ../../bin/qroilib

DEFINES += ROID_LIBRARY

target.path = /usr/lib
INSTALLS += target

include(./qtpropertybrowser/src/qtpropertybrowser.pri)

QT       += core
QT       += gui
QT       += xml
QT       += widgets
QT       += multimedia
QT       += multimediawidgets
QT       += opengl

win32 {
CONFIG(debug, debug|release) {
#LIBS += -L..\winlib -lopencv_core320d -lopencv_imgproc320d -lopencv_highgui320d -lopencv_videoio320d -lopencv_imgcodecs320d
LIBS += -L..\winlib -lopencv_world340d
LIBS += "..\winlib\Debug\jpeg.lib"
LIBS += "..\winlib\Debug\png.lib"
LIBS += "..\winlib\Debug\zlib.lib"
LIBS += "..\winlib\Debug\lcms2.lib"
}
CONFIG(release, debug|release) {
#LIBS += -L..\winlib -lopencv_core320 -lopencv_imgproc320 -lopencv_highgui320 -lopencv_videoio320  -lopencv_imgcodecs320
LIBS += -L..\winlib -lopencv_world340
LIBS += "..\winlib\Release\jpeg.lib"
LIBS += "..\winlib\Release\png.lib"
LIBS += "..\winlib\Release\zlib.lib"
LIBS += "..\winlib\Release\lcms2.lib"
}

LIBS += -lws2_32
LIBS += "..\qroilib\pthread\pthreadVC1.lib"
INCLUDEPATH += "..\winlib\include"
}

linux {
QMAKE_CXXFLAGS += -std=c++11
LIBS += -ljpeg -lexiv2 -lpng -lz -llcms2 -lX11 -lGLU
#-lcap-ng -laudit
INCLUDEPATH += /usr/local/include
}
LDFLAGS=-L # is this even necessary?

INCLUDEPATH += . \
   ./qroilib \
   ./qroilib/document \
   ./qroilib/roilib \
   ./pthread


# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += engine/blob.h \
           engine/blobcontour.h \
           engine/bloblibraryconfiguration.h \
           engine/bloboperators.h \
           engine/blobresult.h \
           engine/componentlabeling.h \
           engine/edgessubpix.h \
           engine/image_contour.h \
           engine/voronoithinner.h \
           engine/imgprocbase.h \
           qroilib/graphicswidgetfloater.h \
           qroilib/gvdebug.h \
           qroilib/imagescaler.h \
           qroilib/imageutils.h \
           qroilib/iodevicejpegsourcemanager.h \
           qroilib/jpegerrormanager.h \
           qroilib/memoryutils.h \
           qroilib/mimetypeutils.h \
           qroilib/mimetypeutils_p.h \
           qroilib/mousewheelbehavior.h \
           qroilib/orientation.h \
           qroilib/paintutils.h \
           qroilib/signalblocker.h \
           qroilib/statusbartoolbutton.h \
           qroilib/thumbnailgroup.h \
           qroilib/variantmanager.h \
           qroilib/widgetfloater.h \
           qroilib/zoommode.h \
           qroilib/zoomslider.h \
           qroilib/zoomwidget.h \
           qroilib/cms/cmsprofile.h \
           qroilib/cms/cmsprofile_png.h \
           qroilib/cms/iccjpeg.h \
           qroilib/document/abstractdocumenteditor.h \
           qroilib/document/abstractdocumentimpl.h \
           qroilib/document/document.h \
           qroilib/document/document_p.h \
           qroilib/document/documentfactory.h \
           qroilib/document/documentjob.h \
           qroilib/documentview/abstractdocumentviewadapter.h \
           qroilib/documentview/abstractimageview.h \
           qroilib/documentview/birdeyeview.h \
           qroilib/documentview/documentview.h \
           qroilib/documentview/documentviewcontainer.h \
           qroilib/documentview/documentviewcontroller.h \
           qroilib/documentview/messageviewadapter.h \
           qroilib/documentview/rasterimageview.h \
           qroilib/documentview/rasterimageviewadapter.h \
           qroilib/jconfig.h \
           qroilib/jmorecfg.h \
           qroilib/jpeglib.h \
           qroilib/lcms2.h \
           qroilib/png.h \
           qroilib/pngconf.h \
           qroilib/zconf.h \
           qroilib/zlib.h \
           qroilib/roilib/abstractobjecttool.h \
           qroilib/roilib/abstracttool.h \
           qroilib/roilib/addremoveroiobject.h \
           qroilib/roilib/changeobjectgroupproperties.h \
           qroilib/roilib/changepolygon.h \
           qroilib/roilib/changeproperties.h \
           qroilib/roilib/changeselectedarea.h \
           qroilib/roilib/compression.h \
           qroilib/roilib/createellipseobjecttool.h \
           qroilib/roilib/createmultipointobjecttool.h \
           qroilib/roilib/createobjecttool.h \
           qroilib/roilib/createpatternobjecttool.h \
           qroilib/roilib/createpointobjecttool.h \
           qroilib/roilib/createpolygonobjecttool.h \
           qroilib/roilib/createpolylineobjecttool.h \
           qroilib/roilib/createrectangleobjecttool.h \
           qroilib/roilib/createscalableobjecttool.h \
           qroilib/roilib/createtextobjecttool.h \
           qroilib/roilib/editpolygontool.h \
           qroilib/roilib/filesystemwatcher.h \
           qroilib/roilib/grouplayer.h \
           qroilib/roilib/hex.h \
           qroilib/roilib/hexagonalrenderer.h \
           qroilib/roilib/imagereference.h \
           qroilib/roilib/isometricrenderer.h \
           qroilib/roilib/layer.h \
           qroilib/roilib/layeritem.h \
           qroilib/roilib/layeroffsettool.h \
           qroilib/roilib/logginginterface.h \
           qroilib/roilib/moveroiobject.h \
           qroilib/roilib/moveroiobjecttogroup.h \
           qroilib/roilib/object.h \
           qroilib/roilib/objectgroup.h \
           qroilib/roilib/objectgroupitem.h \
           qroilib/roilib/objectselectionitem.h \
           qroilib/roilib/objectselectiontool.h \
           qroilib/roilib/objecttypes.h \
           qroilib/roilib/objecttypesmodel.h \
           qroilib/roilib/orthogonalrenderer.h \
           qroilib/roilib/properties.h \
           qroilib/roilib/propertiesdock.h \
           qroilib/roilib/propertybrowser.h \
           qroilib/roilib/qpatternroif.h \
           qroilib/roilib/rangeset.h \
           qroilib/roilib/resizeroiobject.h \
           qroilib/roilib/roid.h \
           qroilib/roilib/roid_global.h \
           qroilib/roilib/roilib_export.h \
           qroilib/roilib/roilib_version.h \
           qroilib/roilib/roimap.h \
           qroilib/roilib/roiobject.h \
           qroilib/roilib/roiobjectitem.h \
           qroilib/roilib/roiobjectmodel.h \
           qroilib/roilib/roireader.h \
           qroilib/roilib/roirenderer.h \
           qroilib/roilib/roiscene.h \
           qroilib/roilib/roitovariantconverter.h \
           qroilib/roilib/roiwriter.h \
           qroilib/roilib/rotateroiobject.h \
           qroilib/roilib/savefile.h \
           qroilib/roilib/selectionrectangle.h \
           qroilib/roilib/snaphelper.h \
           qroilib/roilib/toolmanager.h \
           qroilib/roilib/utils.h \
           qroilib/roilib/variantpropertymanager.h \
           qroilib/roilib/varianttomapconverter.h \
           qroilib/roilib/objectgroup.h \
           qroilib/roilib/roilib_export.h \
           qroilib/document/document.h \
           qroilib/mimetypeutils.h \
           qroilib/cms/cmsprofile.h \
           qroilib/paintutils.h \
           qroilib/orientation.h \
           qroilib/jpeglib.h \
           qroilib/gvdebug.h \
           qroilib/statusbartoolbutton.h \
           qroilib/cms/cmsprofile_png.h \
           qroilib/jpegerrormanager.h \
           qroilib/cms/iccjpeg.h \
           qroilib/lcms2.h \
           qroilib/png.h \
           qroilib/document/documentjob.h \
           qroilib/document/abstractdocumentimpl.h \
           qroilib/documentview/documentview.h \
           qroilib/roilib/roiwriter.h \
           qroilib/document/documentfactory.h \
           qroilib/documentview/birdeyeview.h \
           qroilib/documentview/messageviewadapter.h \
           qroilib/documentview/abstractdocumentviewadapter.h \
           qroilib/documentview/rasterimageview.h \
           qroilib/documentview/abstractimageview.h \
           qroilib/documentview/rasterimageviewadapter.h \
           qroilib/graphicswidgetfloater.h \
           qroilib/signalblocker.h \
           qroilib/roilib/savefile.h \
           qroilib/roilib/roiscene.h \
           qroilib/roilib/roirenderer.h \
           qroilib/roilib/roireader.h \
           qroilib/roilib/addremoveroiobject.h \
           qroilib/roilib/grouplayer.h \
           qroilib/roilib/hexagonalrenderer.h \
           qroilib/roilib/orthogonalrenderer.h \
           qroilib/roilib/isometricrenderer.h \
           qroilib/roilib/layeroffsettool.h \
           qroilib/roilib/abstracttool.h \
           qroilib/roilib/roiobject.h \
           qroilib/roilib/roiobjectmodel.h \
           qroilib/roilib/toolmanager.h \
           qroilib/roilib/roiobjectitem.h \
           qroilib/roilib/objectselectiontool.h \
           qroilib/roilib/createellipseobjecttool.h \
           qroilib/roilib/createobjecttool.h \
           qroilib/roilib/createpolygonobjecttool.h \
           qroilib/roilib/createpolylineobjecttool.h \
           qroilib/roilib/createpointobjecttool.h \
           qroilib/roilib/createrectangleobjecttool.h \
           qroilib/roilib/createpatternobjecttool.h \
           qroilib/roilib/createtextobjecttool.h \
           qroilib/zoomwidget.h \
           qroilib/imagescaler.h \

FORMS += qroilib/documentview/messageview.ui  \
	 qroilib/resize/resizeimagewidget.ui
SOURCES += engine/blob.cpp \
           engine/blobcontour.cpp \
           engine/bloboperators.cpp \
           engine/blobresult.cpp \
           engine/componentlabeling.cpp \
           engine/edgessubpix.cpp \
           engine/voronoithinner.cpp \
           engine/imgprocbase.cpp \
           qroilib/graphicswidgetfloater.cpp \
           qroilib/imagescaler.cpp \
           qroilib/imageutils.cpp \
           qroilib/iodevicejpegsourcemanager.cpp \
           qroilib/memoryutils.cpp \
           qroilib/mimetypeutils.cpp \
           qroilib/paintutils.cpp \
           qroilib/statusbartoolbutton.cpp \
           qroilib/variantmanager.cpp \
           qroilib/widgetfloater.cpp \
           qroilib/zoomslider.cpp \
           qroilib/zoomwidget.cpp \
           qroilib/cms/cmsprofile.cpp \
           qroilib/cms/cmsprofile_png.cpp \
           qroilib/cms/iccjpeg.c \
           qroilib/document/abstractdocumentimpl.cpp \
           qroilib/document/document.cpp \
           qroilib/document/documentfactory.cpp \
           qroilib/document/documentjob.cpp \
           qroilib/documentview/abstractdocumentviewadapter.cpp \
           qroilib/documentview/abstractimageview.cpp \
           qroilib/documentview/birdeyeview.cpp \
           qroilib/documentview/documentview.cpp \
           qroilib/documentview/documentviewcontainer.cpp \
           qroilib/documentview/documentviewcontroller.cpp \
           qroilib/documentview/messageviewadapter.cpp \
           qroilib/documentview/rasterimageview.cpp \
           qroilib/documentview/rasterimageviewadapter.cpp \
           qroilib/roilib/abstractobjecttool.cpp \
           qroilib/roilib/abstracttool.cpp \
           qroilib/roilib/addremoveroiobject.cpp \
           qroilib/roilib/changeobjectgroupproperties.cpp \
           qroilib/roilib/changepolygon.cpp \
           qroilib/roilib/changeproperties.cpp \
           qroilib/roilib/changeselectedarea.cpp \
           qroilib/roilib/compression.cpp \
           qroilib/roilib/createellipseobjecttool.cpp \
           qroilib/roilib/createmultipointobjecttool.cpp \
           qroilib/roilib/createobjecttool.cpp \
           qroilib/roilib/createpatternobjecttool.cpp \
           qroilib/roilib/createpointobjecttool.cpp \
           qroilib/roilib/createpolygonobjecttool.cpp \
           qroilib/roilib/createpolylineobjecttool.cpp \
           qroilib/roilib/createrectangleobjecttool.cpp \
           qroilib/roilib/createscalableobjecttool.cpp \
           qroilib/roilib/createtextobjecttool.cpp \
           qroilib/roilib/editpolygontool.cpp \
           qroilib/roilib/filesystemwatcher.cpp \
           qroilib/roilib/grouplayer.cpp \
           qroilib/roilib/hex.cpp \
           qroilib/roilib/hexagonalrenderer.cpp \
           qroilib/roilib/imagereference.cpp \
           qroilib/roilib/isometricrenderer.cpp \
           qroilib/roilib/layer.cpp \
           qroilib/roilib/layeritem.cpp \
           qroilib/roilib/layeroffsettool.cpp \
           qroilib/roilib/moveroiobject.cpp \
           qroilib/roilib/moveroiobjecttogroup.cpp \
           qroilib/roilib/objectgroup.cpp \
           qroilib/roilib/objectgroupitem.cpp \
           qroilib/roilib/objectselectionitem.cpp \
           qroilib/roilib/objectselectiontool.cpp \
           qroilib/roilib/objecttypes.cpp \
           qroilib/roilib/objecttypesmodel.cpp \
           qroilib/roilib/orthogonalrenderer.cpp \
           qroilib/roilib/properties.cpp \
           qroilib/roilib/propertiesdock.cpp \
           qroilib/roilib/propertybrowser.cpp \
           qroilib/roilib/qpatternroif.cpp \
           qroilib/roilib/resizeroiobject.cpp \
           qroilib/roilib/roimap.cpp \
           qroilib/roilib/roiobject.cpp \
           qroilib/roilib/roiobjectitem.cpp \
           qroilib/roilib/roiobjectmodel.cpp \
           qroilib/roilib/roireader.cpp \
           qroilib/roilib/roirenderer.cpp \
           qroilib/roilib/roiscene.cpp \
           qroilib/roilib/roitovariantconverter.cpp \
           qroilib/roilib/roiwriter.cpp \
           qroilib/roilib/rotateroiobject.cpp \
           qroilib/roilib/savefile.cpp \
           qroilib/roilib/selectionrectangle.cpp \
           qroilib/roilib/snaphelper.cpp \
           qroilib/roilib/toolmanager.cpp \
           qroilib/roilib/utils.cpp \
           qroilib/roilib/variantpropertymanager.cpp \
           qroilib/roilib/varianttomapconverter.cpp \


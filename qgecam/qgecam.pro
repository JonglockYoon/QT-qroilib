
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

INCLUDEPATH += "..\winlib\include"
#INCLUDEPATH += "c:\opencv340\build\include"
}

linux {
QMAKE_CXXFLAGS += -std=c++11 -pthread
LIBS += -ljpeg -lexiv2 -lpng -lz -llcms2 -lX11 -lGLU  -lcap-ng
LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_ml
LIBS += -lgstreamer-1.0
LIBS += -lgobject-2.0
LIBS += -lglib-2.0
LIBS += -lv4l2
LIBS += -lqroilib
LIBS += -lqextserial
LIBS += -L/usr/local/lib
LIBS += -L/usr/lib

INCLUDEPATH += /usr/local/include
}

TEMPLATE = app
TARGET = qgecam

INCLUDEPATH += . \
   ./app \
   ../qroilib \
   ../qroilib/engine \
   ../qroilib/qroilib \
   ../qroilib/qroilib/document \
   ../qroilib/qroilib/roilib \
   ../qroilib/qtpropertybrowser/src \
   ../serial \
   ./genicam \
   ./gvcpdevices \
   ./gvspdevices \
   ./pimpl \

# Input

HEADERS +=  \
        app/mainwindow.h \
        app/viewmainpage.h \
           genicam/boolean.h \
           genicam/boolean_p.h \
           genicam/category.h \
           genicam/category_p.h \
           genicam/command.h \
           genicam/command_p.h \
           genicam/converter.h \
           genicam/converter_p.h \
           genicam/enumentry.h \
           genicam/enumentry_p.h \
           genicam/enumeration.h \
           genicam/enumeration_p.h \
           genicam/floatjgv.h \
           genicam/floatjgv_p.h \
           genicam/floatreg.h \
           genicam/floatreg_p.h \
           genicam/formula.h \
           genicam/formula_p.h \
           genicam/genicam.h \
           genicam/genicamobjectbuilder.h \
           genicam/genicamobjectbuilder_p.h \
           genicam/genicamxmlfile.h \
           genicam/genicamxmlfile_p.h \
           genicam/iboolean.h \
           genicam/icategory.h \
           genicam/icommand.h \
           genicam/ienumeration.h \
           genicam/ifloat.h \
           genicam/iinteger.h \
           genicam/iinterface.h \
           genicam/inode.h \
           genicam/inode_p.h \
           genicam/inodeconverter.h \
           genicam/inodeconverter_p.h \
           genicam/inoderegister.h \
           genicam/inoderegister_p.h \
           genicam/inodeswissknife.h \
           genicam/inodeswissknife_p.h \
           genicam/inodevalue.h \
           genicam/inodevalue_p.h \
           genicam/intconverter.h \
           genicam/intconverter_p.h \
           genicam/integer.h \
           genicam/integer_p.h \
           genicam/intreg.h \
           genicam/intreg_p.h \
           genicam/intswissknife.h \
           genicam/intswissknife_p.h \
           genicam/iport.h \
           genicam/iregister.h \
           genicam/istring.h \
           genicam/maskedintreg.h \
           genicam/maskedintreg_p.h \
           genicam/pvalue.h \
           genicam/register.h \
           genicam/register_p.h \
           genicam/stringreg.h \
           genicam/stringreg_p.h \
           genicam/structentry.h \
           genicam/structentry_p.h \
           genicam/swissknife.h \
           genicam/swissknife_p.h \
           genicam/token.h \
           genicam/xmlhelper.h \
           genicam/gui/genicambooleditor.h \
           genicam/gui/genicambooleditor_p.h \
           genicam/gui/genicambuttoneditor.h \
           genicam/gui/genicambuttoneditor_p.h \
           genicam/gui/genicamcomboboxeditor.h \
           genicam/gui/genicamcomboboxeditor_p.h \
           genicam/gui/genicamdelegate.h \
           genicam/gui/genicamdoubleeditor.h \
           genicam/gui/genicamdoubleeditor_p.h \
           genicam/gui/genicameditor.h \
           genicam/gui/genicamint64editor.h \
           genicam/gui/genicamint64editor_p.h \
           genicam/gui/genicamipv4editor.h \
           genicam/gui/genicamipv4editor_p.h \
           genicam/gui/genicamlineeditor.h \
           genicam/gui/genicamlineeditor_p.h \
           genicam/gui/genicammodel.h \
           genicam/gui/genicammodel_p.h \
           genicam/qtiocompressor/qtiocompressor.h \
           gvcpdevices/bootstrapregisters.h \
           gvcpdevices/bootstrapregisters_p.h \
           gvcpdevices/discoveryhelper.h \
           gvcpdevices/forceiphelper.h \
           gvcpdevices/gvcp.h \
           gvcpdevices/gvcpclient.h \
           gvcpdevices/gvcpclient_p.h \
           gvcpdevices/gvcpdiscoverer.h \
           gvcpdevices/gvcpdiscoverer_p.h \
           gvcpdevices/headerhelper.h \
           gvcpdevices/packethelper.h \
           gvcpdevices/readmemhelper.h \
           gvcpdevices/readreghelper.h \
           gvcpdevices/writememhelper.h \
           gvcpdevices/writereghelper.h \
           gvspdevices/gvsp.h \
           gvspdevices/gvspblock.h \
           gvspdevices/gvspdevices.h \
           gvspdevices/gvspmemoryallocator.h \
           gvspdevices/gvspmemoryallocator_p.h \
           gvspdevices/gvsppacket.h \
           gvspdevices/gvspreceiver.h \
           gvspdevices/gvspreceiver_p.h \
           gvspdevices/timestampdate.h \
           gvspdevices/timestampdate_p.h \
           gvspdevices/ColorConversion.h \
           gvspdevices/bayer.h \
           app/camerasession.h \
           app/camerasession_p.h \
           app/cameravalidationpage.h \
           app/cameravalidationpage_p.h \
           app/genicamtreeview.h \
           app/jgvwizardpage.h \
           app/jgvwizardpage_p.h \
           app/lineseparator.h \
           app/discoverypage.h \
           app/discoverypage_p.h \
           app/networkselectionpage.h \
           app/networkselectionpage_p.h \
           app/gvsppage.h \
           app/gvsppage_p.h \
           app/gvspwidget.h \
           app/gvcputils.h \
           app/gvcputils_p.h \
           app/geviport.h \
           app/htmltable.h \
           app/forceippage.h \
           app/forceippage_p.h \
           app/qtpimpl.hpp \

FORMS += app/advancedconfigpage.ui \
         app/generalconfigpage.ui \
         app/imageviewconfigpage.ui

SOURCES +=  \
        app/main.cpp \
        app/mainwindow.cpp \
        app/viewmainpage.cpp \
           genicam/boolean.cpp \
           genicam/category.cpp \
           genicam/command.cpp \
           genicam/converter.cpp \
           genicam/enumentry.cpp \
           genicam/enumeration.cpp \
           genicam/floatjgv.cpp \
           genicam/floatreg.cpp \
           genicam/formula.cpp \
           genicam/genicam.cpp \
           genicam/genicamobjectbuilder.cpp \
           genicam/genicamxmlfile.cpp \
           genicam/iboolean.cpp \
           genicam/icategory.cpp \
           genicam/icommand.cpp \
           genicam/ienumeration.cpp \
           genicam/ifloat.cpp \
           genicam/iinteger.cpp \
           genicam/iinterface.cpp \
           genicam/inode.cpp \
           genicam/inodeconverter.cpp \
           genicam/inoderegister.cpp \
           genicam/inodeswissknife.cpp \
           genicam/intconverter.cpp \
           genicam/integer.cpp \
           genicam/intreg.cpp \
           genicam/intswissknife.cpp \
           genicam/iport.cpp \
           genicam/iregister.cpp \
           genicam/istring.cpp \
           genicam/maskedintreg.cpp \
           genicam/register.cpp \
           genicam/stringreg.cpp \
           genicam/structentry.cpp \
           genicam/swissknife.cpp \
           genicam/token.cpp \
           genicam/xmlhelper.cpp \
           genicam/gui/genicambooleditor.cpp \
           genicam/gui/genicambuttoneditor.cpp \
           genicam/gui/genicamcomboboxeditor.cpp \
           genicam/gui/genicamdelegate.cpp \
           genicam/gui/genicamdoubleeditor.cpp \
           genicam/gui/genicameditor.cpp \
           genicam/gui/genicamint64editor.cpp \
           genicam/gui/genicamipv4editor.cpp \
           genicam/gui/genicamlineeditor.cpp \
           genicam/gui/genicammodel.cpp \
           genicam/qtiocompressor/qtiocompressor.cpp \
           gvcpdevices/bootstrapregisters.cpp \
           gvcpdevices/discoveryhelper.cpp \
           gvcpdevices/forceiphelper.cpp \
           gvcpdevices/gvcpclient.cpp \
           gvcpdevices/gvcpdiscoverer.cpp \
           gvcpdevices/headerhelper.cpp \
           gvcpdevices/packethelper.cpp \
           gvcpdevices/readmemhelper.cpp \
           gvcpdevices/readreghelper.cpp \
           gvcpdevices/writememhelper.cpp \
           gvcpdevices/writereghelper.cpp \
           gvspdevices/gvspblock.cpp \
           gvspdevices/gvspmemoryallocator.cpp \
           gvspdevices/gvsppacket.cpp \
           gvspdevices/gvspreceiver.cpp \
           gvspdevices/timestampdate.cpp \
           gvspdevices/bayer.cpp \
           app/camerasession.cpp \
           app/cameravalidationpage.cpp \
           app/genicamtreeview.cpp \
           app/jgvwizardpage.cpp \
           app/lineseparator.cpp \
           app/discoverypage.cpp \
           app/gvsppage.cpp \
           app/gvspwidget.cpp \
           app/networkselectionpage.cpp \
           app/gvcputils.cpp \
           app/geviport.cpp \
           app/htmltable.cpp \
           app/forceippage.cpp \

RESOURCES += \
    resources.qrc


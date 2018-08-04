
TEMPLATE = lib
TARGET = qextserial

DEFINES += QEXTSERIALPORT_BUILD_SHARED

target.path = /usr/lib
INSTALLS += target

include(./qextserialport.pri)





set(GVCPDEVICES_SOURCES ${GVCPDEVICES_SOURCES})

list(APPEND GVCPDEVICES_SOURCES
    gvcp.h
    packethelper.cpp
    headerhelper.cpp
    discoveryhelper.cpp
    forceiphelper.cpp
    readreghelper.cpp
    writereghelper.cpp
    writememhelper.cpp
    readmemhelper.cpp
    bootstrapregisters.cpp bootstrapregisters_p.h
    gvcpdiscoverer.cpp gvcpdiscoverer_p.h
    gvcpclient.cpp gvcpclient_p.h
    )

add_library(
    gvcpdevices SHARED
    ${GVCPDEVICES_SOURCES})
   
install(
    TARGETS gvcpdevices
    LIBRARY DESTINATION lib)

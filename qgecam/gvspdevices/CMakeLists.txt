# libcap-ng
include(FindPkgConfig)

pkg_search_module(
    CAPNG REQUIRED libcap-ng)

if (CAPNG_FOUND)
    message(STATUS "cap-ng found")
endif (CAPNG_FOUND)

set(GVSP_SOURCES ${GVSP_SOURCES})

list(APPEND GVSP_SOURCES
    gvsp.h
    gvspdevices.h
    gvsppacket.cpp
    gvspblock.cpp
    gvspreceiver.cpp gvspreceiver_p.h
    gvspmemoryallocator.cpp gvspmemoryallocator_p.h
    timestampdate.cpp timestampdate_p.h)

add_library(
    gvspdevices SHARED
    ${GVSP_SOURCES})

target_include_directories(
    gvspdevices PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    gvspdevices
    ${CAPNG_LIBRARIES})

if (ENDAT)
    target_link_libraries(
        gvspdevices
        endat)
endif()

install(
    TARGETS gvspdevices
    LIBRARY DESTINATION lib)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)


find_package(Qt5Xml REQUIRED)

if (GUI)
    find_package(Qt5Widgets REQUIRED)
endif()

find_package(ZLIB REQUIRED)

set(GENICAM_SOURCES ${GENICAM_SOURCES})

list(APPEND GENICAM_SOURCES
    genicamxmlfile.cpp genicamxmlfile_p.h
    genicamobjectbuilder.cpp genicamobjectbuilder_p.h
    inode.cpp inode_p.h
    inoderegister.cpp inoderegister_p.h
    xmlhelper.cpp
    pvalue.h
    category.cpp category_p.h
    integer.cpp integer_p.h
    intreg.cpp intreg_p.h
    stringreg.cpp stringreg_p.h
    enumeration.cpp enumeration_p.h
    enumentry.cpp enumentry_p.h
    floatjgv.cpp floatjgv_p.h
    floatreg.cpp floatreg_p.h
    inodevalue.h inodevalue_p.h
    inodeswissknife.cpp inodeswissknife_p.h
    inodeconverter.cpp inodeconverter_p.h
    swissknife.cpp swissknife_p.h
    intswissknife.cpp intswissknife_p.h
    converter.cpp converter_p.h
    intconverter.cpp intconverter_p.h
    maskedintreg.cpp maskedintreg_p.h
    structentry.cpp structentry_p.h
    boolean.cpp boolean_p.h
    command.cpp command_p.h
    register.cpp register_p.h
    formula.cpp formula_p.h
    genicam.cpp
    token.cpp


    iinterface.cpp
    icategory.cpp
    istring.cpp
    iinteger.cpp
    ifloat.cpp
    ienumeration.cpp
    iboolean.cpp
    iregister.cpp
    icommand.cpp
    iport.cpp

    qtiocompressor/qtiocompressor.cpp)

if(GUI)
    message(STATUS "enabling Genicam GUI support")
    find_package(Qt5Widgets REQUIRED)
    include_directories(gui)
    list(APPEND GENICAM_SOURCES
        gui/genicammodel.cpp gui/genicammodel_p.h
        gui/genicamdelegate.cpp
        gui/genicameditor.cpp
        gui/genicamint64editor.cpp gui/genicamint64editor_p.h
        gui/genicamdoubleeditor.cpp gui/genicamdoubleeditor_p.h
        gui/genicambuttoneditor.cpp gui/genicambuttoneditor_p.h
        gui/genicamcomboboxeditor.cpp gui/genicamcomboboxeditor_p.h
        gui/genicambooleditor.cpp gui/genicambooleditor_p.h
        gui/genicamlineeditor.cpp gui/genicamlineeditor_p.h
        gui/genicamipv4editor.cpp gui/genicamipv4editor_p.h)

endif()

add_library(
    genicam SHARED
    ${GENICAM_SOURCES}
    resources.qrc)

target_link_libraries(
    genicam
    gvcpdevices
    ${ZLIB_LIBRARIES}
    Qt5::Xml)

if(GUI)
    target_link_libraries(genicam Qt5::Widgets)
endif()

target_include_directories(
    genicam PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(
    TARGETS genicam
    LIBRARY DESTINATION lib)



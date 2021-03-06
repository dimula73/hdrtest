CMAKE_MINIMUM_REQUIRED(VERSION 3.3)

PROJECT("hdrtest")

LIST (APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")


# find and setup Qt5 for this project
set(MIN_QT_VERSION 5.11.0)
set(CMAKE_AUTOMOC ON)

option(USE_OPENGLES "Use OpenGL ES instead of the desktop OpenGL" ON)
configure_file(config-opengles.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-opengles.h)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_CURRENT_BINARY_DIR})

find_package(Qt5 ${MIN_QT_VERSION}
        REQUIRED COMPONENTS
        Core
        Gui
        Widgets
)
# Include the qtmain lib if we are on the windows platform

IF(WIN32)
    SET(LIBS ${LIBS} ${Qt5Core_QTMAIN_LIBRARIES})
ENDIF(WIN32)

FIND_PACKAGE(OpenEXR REQUIRED)
INCLUDE_DIRECTORIES(${OPENEXR_INCLUDE_DIR} "${OPENEXR_INCLUDE_DIR}/OpenEXR")

find_package(Boost 1.55 REQUIRED COMPONENTS system)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

SET(LIBS ${LIBS} ${OPENEXR_LIBRARIES})

set(hdrtest_SRCS 
    glwidget.cpp 
    KisGLImageWidget.cpp
    KisGLImageF16.cpp
    main.cpp
    window.cpp
    ExrReader.cpp
    HdrImageF16.cpp
    kis_debug.cpp
    KisScreenInformationAdapter.cpp
)

set(hdr_QRCS
    ${CMAKE_SOURCE_DIR}/shaders.qrc
    ${CMAKE_SOURCE_DIR}/kis_gl_image_widget.qrc
    CACHE INTERNAL "hdr_QRCS"
)
qt5_add_resources(hdrtest_SRCS ${hdr_QRCS})

add_executable(hdrtest ${hdrtest_SRCS})
target_link_libraries(
    hdrtest
        PRIVATE 
    ${LIBS}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets

)

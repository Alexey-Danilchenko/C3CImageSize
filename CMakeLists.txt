cmake_minimum_required(VERSION 3.17)

# For MacOS only
if (APPLE)
    # TBB 12 on MacOS requires 15.0 and is arm64 only
    set(CMAKE_OSX_DEPLOYMENT_TARGET "15.0" CACHE STRING "Minimum OS X deployment version")
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures")
endif()

project(C3CPlugins VERSION 3.2)

# Search path for cmake modules
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

# Accept shell-style colon separated CMAKE_PREFIX_PATH (CMake lists are ;-separated)
if(NOT WIN32)
    string(REPLACE ":" ";" CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}")
endif()

# Default build type release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Usage of TBB clashes with QT internal emit keyword
add_compile_definitions(QT_NO_KEYWORDS)

# Build self-contained artifacts into bin/<build type>
if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
endif()

message("Looking for Qt...")
# Qt modules (https://doc.qt.io/qt-6/qtmodules.html) you're using in your application
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Xml)
if (${Qt6_FOUND})
    message("Found Qt " ${Qt6_VERSION})
else()
    message("Couldn't find Qt")
endif()

# Photoshop SDK
find_package(PsSdk REQUIRED)
if (${PsSdk_FOUND})
    message("Found Photoshop SDK at " ${PsSdk_INCLUDE_DIR})
else()
    message("Couldn't find Photoshop SDK")
endif()

# TBB - until QT/TBB is sorted
find_package(TBB REQUIRED tbb)
if (${TBB_FOUND})
    message("Found TBB " ${TBB_VERSION})
else()
    message("Couldn't find TBB")
endif()

# Photoshop plugins are application bundles on MacOS and DLLs on Windows
if(WIN32)
    add_library(C3CImageSize SHARED)
    add_library(C3CImageSizeAutomate SHARED)
else()
    add_executable(C3CImageSize)
    add_executable(C3CImageSizeAutomate)
endif()

# Version follows the PiPLs.json version
set(C3C_PLUGINS_VERSION 3.2)

# Add common include
include_directories(common)

# SDK suite acquisition globals (sPSActionDescriptor, sPSUIHooks, ...)
target_sources(C3CImageSize PRIVATE
    ${PsSdk_SAMPLE_SOURCES_DIR}/PIUSuites.cpp
)
target_sources(C3CImageSizeAutomate PRIVATE
    ${PsSdk_SAMPLE_SOURCES_DIR}/PIUSuites.cpp
)

target_link_libraries(C3CImageSize
                      Qt6::Core
                      Qt6::Gui
                      Qt6::Widgets
                      Qt6::Network
                      Qt6::Xml
                      PsSdk::PsSdk
                      TBB::tbb)

target_link_libraries(C3CImageSizeAutomate
                      Qt6::Core
                      Qt6::Gui
                      Qt6::Widgets
                      Qt6::Network
                      Qt6::Xml
                      PsSdk::PsSdk
                      TBB::tbb)

add_subdirectory(common)
add_subdirectory(C3CImageSize)
add_subdirectory(C3CImageSizeAutomate)

if (APPLE)
    add_subdirectory(mac)
elseif(WIN32)
    add_subdirectory(win)
else()
    message(FATAL_ERROR "Platform not supported!")
endif()

# Post build steps - copy libraries and other dependencies
copy_libraries(C3CImageSize
               Qt6::Core
               Qt6::Gui
               Qt6::Widgets
               Qt6::Network
               Qt6::Xml
               TBB::tbb)

copy_qt_plugins(C3CImageSize)

copy_libraries(C3CImageSizeAutomate
               Qt6::Core
               Qt6::Gui
               Qt6::Widgets
               Qt6::Network
               Qt6::Xml
               TBB::tbb)

copy_qt_plugins(C3CImageSizeAutomate)

# The filter script is installed manually into Photoshop's Scripts folder
# (see C3CImageSize/C3CImageSize2FilterScripting.jsx). Ship it next to the
# plugins in bin/<build type> so the release archive contains the plugins
# and the script.
add_custom_target(C3CImageSize_filter_script ALL
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/C3CImageSize/C3CImageSize2FilterScripting.jsx
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    DEPENDS C3CImageSize
    COMMENT "Copying C3CImageSize2FilterScripting.jsx to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

# On MacOS Photoshop plugins are .plugin bundles.
# The rename is a custom target (not a post-build step) so it runs after
# all bundle content has been copied and macdeployqt has finished.
if (APPLE)
    foreach(_plugin C3CImageSize C3CImageSizeAutomate)
        set(_rename_cmds
            COMMAND ${CMAKE_COMMAND} -E rm -rf $<TARGET_BUNDLE_DIR:${_plugin}>/../${_plugin}.plugin
            COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_BUNDLE_DIR:${_plugin}>
                                             $<TARGET_BUNDLE_DIR:${_plugin}>/../${_plugin}.plugin
        )
        add_custom_target(${_plugin}_plugin ALL
            ${_rename_cmds}
            DEPENDS ${_plugin}_macdeployqt
            COMMENT "Renaming ${_plugin}.app to ${_plugin}.plugin"
        )
    endforeach()
endif()

cmake_minimum_required(VERSION 3.16)

project(SerialMaster VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets SerialPort Network)

# Source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    serialportmanager.cpp
    datapipeline.cpp
    configmanager.cpp
    protocolparser.cpp
    plugins/serialplugin.cpp
    ui/receivewidget.cpp
    ui/sendwidget.cpp
    ui/statusbar.cpp
    ui/waveformwidget.cpp
    python/pythonengine.cpp
    websocket/serialbridge.cpp
)

# Header files
set(HEADERS
    mainwindow.h
    serialportmanager.h
    datapipeline.h
    configmanager.h
    protocolparser.h
    plugins/iserialplugin.h
    plugins/serialplugin.h
    ui/receivewidget.h
    ui/sendwidget.h
    ui/statusbar.h
    ui/waveformwidget.h
    python/pythonengine.h
    websocket/serialbridge.h
)

# UI files
set(FORMS
    mainwindow.ui
)

# Resource files
set(RESOURCES
    resources.qrc
)

# Create executable
qt_add_executable(SerialMaster
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
)

# Include directories
target_include_directories(SerialMaster PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/plugins
    ${CMAKE_CURRENT_SOURCE_DIR}/python
    ${CMAKE_CURRENT_SOURCE_DIR}/websocket
)

# Link Qt6 libraries
target_link_libraries(SerialMaster PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::SerialPort
    Qt6::Network
)

# Set output directory
set_target_properties(SerialMaster PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build
)

# Platform-specific configurations
if(UNIX AND NOT APPLE)
    target_compile_options(SerialMaster PRIVATE -Wall -Wextra)
endif()

if(WIN32)
    target_compile_definitions(SerialMaster PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

QT += core gui serialport widgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Output directory
DESTDIR = $$PWD/build

# Source files
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    serialportmanager.cpp \
    datapipeline.cpp \
    configmanager.cpp \
    protocolparser.cpp \
    plugins/serialplugin.cpp \
    ui/receivewidget.cpp \
    ui/sendwidget.cpp \
    ui/statusbar.cpp \
    ui/waveformwidget.cpp \
    python/pythonengine.cpp \
    websocket/serialbridge.cpp

HEADERS += \
    mainwindow.h \
    serialportmanager.h \
    datapipeline.h \
    configmanager.h \
    protocolparser.h \
    plugins/iserialplugin.h \
    plugins/serialplugin.h \
    ui/receivewidget.h \
    ui/sendwidget.h \
    ui/statusbar.h \
    ui/waveformwidget.h \
    python/pythonengine.h \
    websocket/serialbridge.h

FORMS += \
    mainwindow.ui

# Resource files
RESOURCES += \
    resources.qrc

# Unix specific configurations
unix {
    QMAKE_CXXFLAGS += -Wall -Wextra
}

# Windows specific configurations
win32 {
    DEFINES += _CRT_SECURE_NO_WARNINGS
}

# Python integration (pybind11 would be added separately)
# INCLUDEPATH += /path/to/pybind11/include

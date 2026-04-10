/****************************************************************************
**
** Copyright (C) 2024 SerialMaster Team
**
** SerialMaster - Professional Serial Port Debugging Tool
** Based on Qt6 with modern UI and advanced features
**
****************************************************************************/

#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDarkStyle>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application info
    QCoreApplication::setOrganizationName("SerialMaster");
    QCoreApplication::setApplicationName("SerialMaster");
    QCoreApplication::setApplicationVersion("1.0.0");
    
    // Set dark theme
    app.setStyle(QStyleFactory::create("Fusion"));
    
    MainWindow window;
    window.show();
    
    return app.exec();
}

QT += serialbus widgets charts core

TARGET = RobotController
TEMPLATE = app

SOURCES += \
    bitratebox.cpp \
    connectdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    sendframebox.cpp \
    themewidget.cpp

HEADERS += \
    bitratebox.h \
    connectdialog.h \
    mainwindow.h \
    sendframebox.h \
    themewidget.h

FORMS   += mainwindow.ui \
    connectdialog.ui \
    sendframebox.ui \
    themewidget.ui

RESOURCES += \
    robot.qrc

target.path = ../
INSTALLS += target

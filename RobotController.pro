QT += serialbus widgets

TARGET = RobotController
TEMPLATE = app

SOURCES += \
    bitratebox.cpp \
    connectdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    sendframebox.cpp

HEADERS += \
    bitratebox.h \
    connectdialog.h \
    mainwindow.h \
    sendframebox.h

FORMS   += mainwindow.ui \
    connectdialog.ui \
    sendframebox.ui

RESOURCES += \
    robot.qrc

target.path = ../
INSTALLS += target

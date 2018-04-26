QT += serialbus widgets charts core

TARGET = RobotController
TEMPLATE = app

SOURCES += \
    bitratebox.cpp \
    connectdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    sendframebox.cpp \
    themewidget.cpp \
    chartview.cpp

HEADERS += \
    bitratebox.h \
    connectdialog.h \
    mainwindow.h \
    sendframebox.h \
    themewidget.h \
    ECanVci.h \
    chartview.h \
    doubleslider.h \
    receiveworkerthread.h

FORMS   += mainwindow.ui \
    connectdialog.ui \
    sendframebox.ui \
    themewidget.ui

RESOURCES += \
    robot.qrc

target.path = ../
INSTALLS += target

unix|win32: LIBS += -L$$PWD/./ -lECanVci

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

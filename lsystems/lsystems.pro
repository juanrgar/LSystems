#-------------------------------------------------
#
# Project created by QtCreator 2012-11-19T19:57:31
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lsystems
TEMPLATE = app

LIBS += -framework OpenCL

SOURCES += main.cpp\
        mainwindow.cpp \
    centralwidget.cpp \
    maincontroller.cpp \
    lsystem.cpp \
    lrule.cpp \
    lparser.cpp \
    glviewer.cpp

HEADERS  += mainwindow.h \
    centralwidget.h \
    maincontroller.h \
    lsystem.h \
    lrule.h \
    lparser.h \
    glviewer.h

OTHER_FILES += \
    koch.txt \
    derivation.cl \
    draw.cl \
    basic.vert \
    basic.frag

RESOURCES += \
    resources.qrc

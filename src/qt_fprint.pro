#-------------------------------------------------
#
# Project created by QtCreator 2011-02-21T02:57:37
#
#-------------------------------------------------

QT       += core gui

TARGET = qt_fprint
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    cfingerprintscanner.cpp

HEADERS  += mainwindow.h \
    cfingerprintscanner.h

FORMS    += mainwindow.ui

LIBS += -lfprint


#-------------------------------------------------
#
# Project created by QtCreator 2015-06-02T07:35:41
#
#-------------------------------------------------

QT       += core gui
QT += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Dail
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    CContactorsTable.cpp \
    DatabaseTable.cpp

HEADERS  += mainwindow.h \
    CContatorsTable.h \
    DatabaseTable.h

FORMS    += mainwindow.ui

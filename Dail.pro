#-------------------------------------------------
#
# Project created by QtCreator 2015-06-07T06:00:00
#
#-------------------------------------------------

QT       += core gui
QT += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dail
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    CContactorsTable.cpp \
    DatabaseTable.cpp \
    CommRecordTable.cpp \
    ChinesePinyinTable.cpp \
    AddContactorThread.cpp \
    DynamicSelectThread.cpp

HEADERS  += mainwindow.h \
    CContatorsTable.h \
    DatabaseTable.h \
    CommRecordTable.h \
    ChinesePinyinTable.h \
    AddContactorThread.h \
    DynamicSelectThread.h

FORMS    += mainwindow.ui

RESOURCES += \
    icons.qrc

RC_FILE += myapp.rc

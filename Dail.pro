#-------------------------------------------------
#
# Project created by QtCreator 2015-06-07T06:00:00
#
#-------------------------------------------------

QT       += core gui
QT += sql
QT += serialport

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
    DynamicSelectThread.cpp \
    ModemPoolSerialPort.cpp \
    ModemPool.cpp

HEADERS  += mainwindow.h \
    CContatorsTable.h \
    DatabaseTable.h \
    CommRecordTable.h \
    ChinesePinyinTable.h \
    AddContactorThread.h \
    DynamicSelectThread.h \
    ModemPoolSerialPort.h \
    ModemPool.h

FORMS    += mainwindow.ui

RESOURCES += \
    icons.qrc

RC_FILE += myapp.rc

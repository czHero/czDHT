#-------------------------------------------------
#
# Project created by QtCreator 2014-05-15T16:07:03
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cz_DHT
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    bencodeparser.cpp \
    ktable.cpp \
    utils.cpp \
    krpc.cpp

HEADERS  += mainwindow.h \
    bencodeparser.h \
    ktable.h \
    utils.h \
    krpc.h

FORMS    += mainwindow.ui

#-------------------------------------------------
#
# Project created by QtCreator 2017-03-27T13:46:41
#
#-------------------------------------------------

QT       += core gui
QT       += concurrent
QT       += sql
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets



TARGET = S3ClientGUI
TEMPLATE = app

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += aws-cpp-sdk-s3
mac: LIBS += -lcurl


SOURCES += main.cpp\
        mainwindow.cpp \
    ../S3Client/actions.cpp \
    ../S3Client/qs3client.cpp \
    s3treemodel.cpp \
    s3item.cpp \
    qfilesystemview.cpp \
    ../S3Client/qlogs3.cpp \
    qtaskmodel.cpp \
    transferwidget.cpp


HEADERS  += mainwindow.h \
    ../S3Client/qs3client.h \
    ../S3Client/actions.h \
    s3treemodel.h \
    s3item.h \
    qfilesystemview.h \
    ../S3Client/qlogs3.h \
    qtaskmodel.h \
    transferwidget.h

FORMS    += mainwindow.ui

DISTFILES += \
    README.md

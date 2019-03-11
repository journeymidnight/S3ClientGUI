#-------------------------------------------------
#
# Project created by QtCreator 2017-03-27T13:46:41
#
#-------------------------------------------------

QT       += core gui
QT       += concurrent
QT       += sql
win32: QT  += winextras
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets



TARGET = S3ClientGUI
TEMPLATE = app

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += aws-cpp-sdk-s3
mac: LIBS += -lcurl


SOURCES += main.cpp\
        mainwindow.cpp \
    ../s3util/actions.cpp \
    ../s3util/qs3client.cpp \
    s3treemodel.cpp \
    s3item.cpp \
    qfilesystemview.cpp \
    ../s3util/qlogs3.cpp \
    qtaskmodel.cpp \
    transferwidget.cpp \
    filesystemmodel.cpp \
    driveselectwidget.cpp \
    editaccountdialog.cpp \
    qs3config.cpp


HEADERS  += mainwindow.h \
    ../s3util/qs3client.h \
    ../s3util/actions.h \
    s3treemodel.h \
    s3item.h \
    qfilesystemview.h \
    ../s3util/qlogs3.h \
    qtaskmodel.h \
    transferwidget.h \
    filesystemmodel.h \
    driveselectwidget.h \
    editaccountdialog.h \
    qs3config.h

FORMS    += mainwindow.ui \
    editaccountdialog.ui

DISTFILES += \
    README.md

RESOURCES += \
    resource.qrc

RC_ICONS = images/Main.ico

# Single Application implementation
include(SingleApplication/singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication

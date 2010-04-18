# -------------------------------------------------
# Project created by QtCreator 2010-04-03T17:25:49
# -------------------------------------------------
QT += network \
    sql \
    script \
    scripttools \
    xml
QT -= gui
TARGET = ApplicationServer
CONFIG += console
CONFIG -= app_bundle

# CONFIG += qt link_pkgconfig
# PKGCONFIG += soprano
TEMPLATE = app
LIBS += ../RPC/xml_rpc/libxml_rpc.so \
    ../SharedServices/libSharedServices.so \
    /usr/local/lib/libsoprano.so \
    /usr/local/lib/libsopranoclient.so \
    /usr/local/lib/libsopranoserver.so \
    /usr/local/lib/libsopranoindex.so
SOURCES += main.cpp \
    nsclient.cpp \
    appperiodicprocess.cpp \
    filemanager.cpp \
    scriptrunner.cpp \
    scripthelper.cpp \
    server.cpp \
    applicationservicerequest.cpp
HEADERS += nsclient.h \
    appperiodicprocess.h \
    filemanager.h \
    scriptrunner.h \
    scripthelper.h \
    server.h \
    applicationservicerequest.h

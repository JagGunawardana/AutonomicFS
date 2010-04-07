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
CONFIG += qt link_pkgconfig
PKGCONFIG += soprano
TEMPLATE = app
LIBS += ../RPC/xml_rpc/libxml_rpc.so \
	../SharedServices/libSharedServices.so
SOURCES += main.cpp \
    nsclient.cpp \
    appperiodicprocess.cpp \
    filemanager.cpp
HEADERS += nsclient.h \
    appperiodicprocess.h \
    filemanager.h

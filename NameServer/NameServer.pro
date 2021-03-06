# -------------------------------------------------
# Project created by QtCreator 2010-03-29T16:19:22
# -------------------------------------------------
QT += network \
    xml \
    script \
    scripttools
QT -= gui
TARGET = NameServer
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    server.cpp \
    applicationserver.cpp \
    servicerequest.cpp \
    nsscriptrunner.cpp \
    nsscripthelper.cpp \
    nameserver.cpp
LIBS += ../RPC/xml_rpc/libxml_rpc.so \
    ../SharedServices/libSharedServices.so
HEADERS += server.h \
    applicationserver.h \
    servicerequest.h \
    nsscriptrunner.h \
    nsscripthelper.h \
    nameserver.h

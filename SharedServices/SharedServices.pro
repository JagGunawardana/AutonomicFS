# -------------------------------------------------
# Project created by QtCreator 2010-04-04T00:48:36
# -------------------------------------------------
QT += network \
    script \
    scripttools
QT -= gui
TARGET = SharedServices
TEMPLATE = lib
DEFINES += SHAREDSERVICES_LIBRARY
SOURCES += sharedservices.cpp \
    logger.cpp \
    profilemgr.cpp
HEADERS += sharedservices.h \
    SharedServices_global.h \
    logger.h \
    profilemgr.h

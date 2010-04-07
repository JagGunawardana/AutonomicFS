# -------------------------------------------------
# Project created by QtCreator 2010-04-05T16:10:36
# -------------------------------------------------
QT += network \
    xml
QT -= gui
TARGET = xml_rpc
TEMPLATE = lib

DEFINES += XML_RPC_LIBRARY
SOURCES += client.cpp \
    request.cpp \
    server.cpp \
    serverintrospection.cpp \
    variant.cpp \
    response.cpp
HEADERS += xml_rpc_global.h \
    variant.h \
    client.h \
    qxmlrpc_documentation.h \
    request.h \
    server_private.h \
    server.h \
    serverintrospection.h \
    stable.h \
    variant.h \
    response.h


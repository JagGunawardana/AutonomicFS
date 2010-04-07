#ifndef XML_RPC_GLOBAL_H
#define XML_RPC_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(XML_RPC_LIBRARY)
#  define XML_RPCSHARED_EXPORT Q_DECL_EXPORT
#else
#  define XML_RPCSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // XML_RPC_GLOBAL_H

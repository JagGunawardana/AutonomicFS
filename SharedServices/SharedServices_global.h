#ifndef SHAREDSERVICES_GLOBAL_H
#define SHAREDSERVICES_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SHAREDSERVICES_LIBRARY)
#  define SHAREDSERVICESSHARED_EXPORT Q_DECL_EXPORT
#else
#  define SHAREDSERVICESSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // SHAREDSERVICES_GLOBAL_H

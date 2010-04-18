#include "nsscripthelper.h"

NSScriptHelper::NSScriptHelper(QObject* parent) :
	QObject(parent) {
}

int NSScriptHelper::GetValue(int a) {
	return(a);
}

#include "scripthelper.h"
#include <QDebug>
ScriptHelper::ScriptHelper(QObject* parent) :
QObject(parent) {
}

int ScriptHelper::GetValue(int a) {
	return(a*a);
}

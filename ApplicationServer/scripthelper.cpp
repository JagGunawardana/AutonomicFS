#include <QVariant>
#include <QDebug>
#include "filemanager.h"
#include "scripthelper.h"

ScriptHelper::ScriptHelper(QObject* parent) :
QObject(parent) {
}

int ScriptHelper::GetValue(int a) {
	return(a*a);
}


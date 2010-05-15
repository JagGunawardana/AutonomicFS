#ifndef SCRIPTHELPER_H
#define SCRIPTHELPER_H

#include <QObject>

class QVariant;

class ScriptHelper : public QObject {
	Q_OBJECT
public:
	ScriptHelper(QObject* parent = NULL);

public:
public slots:
	int GetValue(int a);
};

#endif // SCRIPTHELPER_H

#ifndef NSSCRIPTHELPER_H
#define NSSCRIPTHELPER_H

#include <QObject>

class NSScriptHelper : public QObject {
	Q_OBJECT
public:
	NSScriptHelper(QObject* parent = NULL);

public slots:
	int GetValue(int a);
};

#endif // NSSCRIPTHELPER_H

#ifndef NSSCRIPTHELPER_H
#define NSSCRIPTHELPER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QVariant>

#include "../RPC/xml_rpc/client.h"

class Server;

class NSScriptHelper : public QObject {
	Q_OBJECT
public:
	NSScriptHelper(QObject* parent, Server* server,
				   QMap<QString, QVariant> params);
	~NSScriptHelper();
private:
	Server* server;
	QMap<QString, QVariant> params;
	xmlrpc::Client* client;
public slots:
	int GetValue(int a);
	QVariant GetParameter(QString param) {return params[param];}
	QVariant GetNumberOfParameters(void) {return params.size();}
	QVariant GetActiveAppServers(void);
};

#endif // NSSCRIPTHELPER_H

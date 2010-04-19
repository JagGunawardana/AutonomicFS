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
	void processReturnValue( int requestId, QVariant value );
	void processFault( int requestId,
					   int errorCode, QString errorString );
	QVariant script_ret_val;
	QSemaphore sem_sync;
public slots:
	int GetValue(int a);
	QVariant GetParameter(QString param) {return params[param];}
	QVariant GetNumberOfParameters(void) {return params.size();}
	QVariant GetActiveAppServers(void);
	QVariant ExtractFile(QVariant file_store);
	QVariant HasFile(QVariant file_store);
	QVariant TryGetFileByName(QVariant server, QString file_name);
};

#endif // NSSCRIPTHELPER_H

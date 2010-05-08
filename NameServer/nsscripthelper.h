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
	QVariant script_ret_val;
	QEventLoop event_loop;
public slots:
	int GetValue(int a);
	QVariant DummyCall(QVariantMap server, QString dummy_string);
	QVariant GetParameter(QString param) {return params[param];}
	QVariant GetNumberOfParameters(void) {return params.size();}
	QVariant GetActiveAppServers(void);
	QVariant GetActiveNameServers(void);
	QVariant ExtractFile(QVariant file_store);
	QVariant HasFile(QVariant file_store);
	QVariant TryGetFileByName(QVariantMap server, QString file_name);
	QVariant GetAllFilesUnderMgt(QVariantMap server);
private slots:
	void processReturnValue( int requestId, QVariant value );
	void processFault( int requestId,
					   int errorCode, QString errorString );

};

#endif // NSSCRIPTHELPER_H

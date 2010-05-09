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
				   QMap<QString, QVariant> params, int requestId);
	~NSScriptHelper();
private:
	Server* server;
	QMap<QString, QVariant> params;
	xmlrpc::Client* client;
	QVariant script_ret_val;
	QEventLoop event_loop;
	int requestId;
protected:
	QVariant MakeAppCall(const char* call_name, QVariantMap& server, QString* str1 = NULL, QString* str2 = NULL);
public slots:
	int GetValue(int a); // Test call
	QVariant DummyCall(QVariantMap server, QString dummy_string); // Test call
	QVariant GetParameter(QString param) {return params[param];}
	QVariant GetNumberOfParameters(void) {return params.size();}
	QVariant GetActiveAppServers(void);
	QVariant GetActiveNameServers(void);
	QVariant TryGetFileByName(QVariantMap server, QString file_name);
	QVariant TryGetFileByHash(QVariantMap server, QString file_hash);
	QVariant GetAllFilesUnderMgt(QVariantMap server);
	QVariant GetIPAddress(void);
private slots:
	void processReturnValue( int requestId, QVariant value );
	void processFault( int requestId,
					   int errorCode, QString errorString );

};

#endif // NSSCRIPTHELPER_H

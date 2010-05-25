#ifndef NSSCRIPTHELPER_H
#define NSSCRIPTHELPER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QVariant>

#include "../RPC/xml_rpc/client.h"
#include "server.h"


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
	QVariant MakeRemoteNSCall(QString ServerIP, const char* call_name, QString* str1 = NULL, QString* str2 = NULL);
	QVariant MakeRemoteNSCall(QString ServerIP, const char* call_name, QString* str1, QByteArray* content, bool force);
public slots:
	int GetValue(int a); // Test call
	void Debug(QVariant str);
	void LogMessage(QVariant str);
	QVariant GetReplicaCount(void);
	QVariant GetAliveTime(void) {return(this->server->GetAliveTime());}
	QVariant DummyCall(QVariantMap server, QString dummy_string); // Test call
	QVariant GetParameter(QString param) {return params[param];}
	QVariant GetNumberOfParameters(void) {return params.size();}
	QVariant GetActiveAppServers(void);
	QVariant GetIdleAppServer(void);
	QVariant GetActiveNameServers(void);
	QVariant GetIdleNameServerFromList(QVariant list);
	QVariant GetLoadOfServer(QVariant IP);
	QVariant GetAllActiveNameServers(void);
	QVariant TryGetFileByName(QVariantMap server, QString file_name);
	QVariant TryGetFileByHash(QVariantMap server, QString file_hash);
	QVariant SaveFile(QVariantMap server, QString file_name, QString file_content);
	QVariant SaveNewFile(QVariantMap server, QString file_name, QString file_content);
	QVariant GetAllLocalFilesUnderMgt(QVariantMap server);
	QVariant GetIPAddress(void);
	QVariant GetLocalFileListForServer(QVariant ServerIP);
	QVariant GetRemoteFileFromServerByName(QVariant ServerIP, QString file_name);
	QVariant GetRemoteFileFromServerByHash(QVariant ServerIP, QString hash);
	QVariant SaveRemoteFile(QVariant ServerIP, QString hash, QString file_content, bool Force=false);
	QVariant DeleteFile(QVariantMap server, QString file_name);
	QVariant DeleteRemoteFile(QVariant ServerIP, QString file_name);
private slots:
	void processReturnValue( int requestId, QVariant value );
	void processFault( int requestId,
					   int errorCode, QString errorString );
	void processNSReturnValue( int requestId, QVariant value );
	void processNSFault( int requestId,
					   int errorCode, QString errorString );

};

#endif // NSSCRIPTHELPER_H

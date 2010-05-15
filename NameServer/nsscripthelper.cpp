#include "nsscripthelper.h"
#include "server.h"

#include "../RPC/xml_rpc/client.h"
#include "../SharedServices/logger.h"

#include <QDebug>

NSScriptHelper::NSScriptHelper(QObject* parent,
							   Server* server,
							   QMap<QString, QVariant> params, int requestId) :
QObject(parent), script_ret_val(), event_loop(parent)
{
	this->server = server;
	this->params = params;
	this->requestId = requestId;
	client = NULL;
}

NSScriptHelper::~NSScriptHelper() {
	if (client != NULL)
		delete(client);
}

int NSScriptHelper::GetValue(int a) {
	return(a);
}

void NSScriptHelper::Debug(QVariant str) {
	qDebug()<<str;
}

QVariant NSScriptHelper::GetActiveAppServers(void) {
	return(server->GetActiveApplicationServers());
}

QVariant NSScriptHelper::GetActiveNameServers(void) {
	return(xmlrpc::Variant(server->GetActiveNameServers()));
}

QVariant NSScriptHelper::GetAllActiveNameServers(void) {
	QList<xmlrpc::Variant> servers = server->GetActiveNameServers();
	if (requestId>0)
		servers.insert(0, xmlrpc::Variant(server->GetIPAddress(requestId)));
	else
		Q_ASSERT("Should you be calling this ??");
	return(xmlrpc::Variant(servers));
}

QVariant NSScriptHelper::GetIPAddress(void) {
	if (requestId<0)
		return(QVariant(QString("127.0.0.1")));
	else
		return(QVariant(server->GetIPAddress(requestId)));
}

void NSScriptHelper::processReturnValue( int requestId, QVariant value ) {
	Q_UNUSED(requestId)
	script_ret_val = value;
	event_loop.processEvents(QEventLoop::AllEvents);
	event_loop.exit();
}

void NSScriptHelper::processFault( int requestId, int errorCode, QString errorString ) {
	Q_UNUSED( requestId );
	Logger("Name Server Scripthelper",
		   "server_log").WriteLogLine(QString("Error"),
			QString("(Script heleper) Error in communication with application server: Code(%1), String(%2).").arg(errorCode).arg(errorString));
	script_ret_val = QVariant();
	event_loop.exit();
}

QVariant NSScriptHelper::DummyCall(QVariantMap server, QString dummy_string) {
	return(MakeAppCall("Dummy", server, &dummy_string));
}

QVariant NSScriptHelper::GetAllLocalFilesUnderMgt(QVariantMap server) {
	QString ip = GetIPAddress().toString();
	QVariant ret = MakeAppCall("Service_GetAllFilesUnderMgt", server, &ip);
	if (ret.isValid())
		return(ret);
	else {
		QList<QVariant> dummy;
		for(int i=0;i<5;i++)
			dummy.append(QString(""));
		QList<QVariant> dummy1;
		dummy1.append(QVariant(dummy));
		return(dummy1);
	}
}

QVariant NSScriptHelper::TryGetFileByName(QVariantMap server, QString file_name) {
	// Ask app server for file (by name)
	QVariant ret = MakeAppCall("Service_FileByName", server, &file_name);
	if (ret.isValid())
		return(ret);
	else {
		QList<QVariant> dummy;
		dummy.append(QVariant(false));
		for(int i=0;i<3;i++)
			dummy.append(QString(""));
		return(dummy);
	}
}

QVariant NSScriptHelper::TryGetFileByHash(QVariantMap server, QString file_hash) {
	// Ask app server for file (by hash)
	QVariant ret = MakeAppCall("Service_FileByHash", server, &file_hash);
	if (ret.isValid())
		return(ret);
	else {
		QList<QVariant> dummy;
		dummy.append(QVariant(false));
		for(int i=0;i<3;i++)
			dummy.append(QString(""));
		return(dummy);
	}
}

QVariant NSScriptHelper::SaveFile(QVariantMap server, QString file_name, QString file_content) {
	// Ask app server to save file if it has it in store
	QVariant ret = MakeAppCall("Service_SaveFile", server, &file_name, &file_content);
	if (ret.isValid())
		return(ret);
	else
		return(QVariant(false));
}

QVariant NSScriptHelper::MakeAppCall(const char* call_name, QVariantMap& server, QString* str1, QString* str2) {
	// Factored out to make an application xml_rpc call
	client = new xmlrpc::Client(this);
	int port_number =server[QString("port")].toInt();
	QString address = "localhost";
	client->setHost(address, port_number);
	connect( client, SIGNAL(done( int, QVariant )),
		this, SLOT(processReturnValue( int, QVariant )) );
	connect( client, SIGNAL(failed( int, int, QString )),
		this, SLOT(processFault( int, int, QString )) );
	if (str1==NULL && str2==NULL)
		client->request(call_name);
	else if (str2==NULL && str1!=NULL)
		client->request(call_name, *str1);
	else if (str2!=NULL && str1 != NULL)
		client->request(call_name, *str1, *str2);
	event_loop.exec();
	QEventLoop().processEvents(QEventLoop::AllEvents, 200000);
	return(script_ret_val);
}

QVariant NSScriptHelper::GetLocalFileListForServer(QVariant ServerIP) {
	// Ask app server for file (by name)
	QVariant ret = MakeRemoteNSCall(ServerIP.toString(), "Service_GetAllLocalFilesUnderMgt");
	if (ret.isValid())
		return(ret);
	else {
		QList<QVariant> dummy;
		for(int i=0;i<5;i++)
			dummy.append(QString(""));
		QList<QVariant> dummy1;
		dummy1.append(QVariant(dummy));
		return(dummy1);
	}
}

QVariant NSScriptHelper::GetRemoteFileFromServerByName(QVariant ServerIP, QString file_name) {
	// Ask app server for file (by name)
	QVariant ret = MakeRemoteNSCall(ServerIP.toString(), "Service_RequestFileByName", &file_name);
	if (ret.isValid())
		return(ret);
	else {
		QList<QVariant> dummy;
		dummy.append(QVariant(false));
		for(int i=0;i<3;i++)
			dummy.append(QString(""));
		return(dummy);
	}
}

QVariant NSScriptHelper::GetRemoteFileFromServerByHash(QVariant ServerIP, QString hash) {
	// Ask app server for file (by hash)
	QVariant ret = MakeRemoteNSCall(ServerIP.toString(), "Service_RequestFileByHash", &hash);
	if (ret.isValid())
		return(ret);
	else {
		QList<QVariant> dummy;
		dummy.append(QVariant(false));
		for(int i=0;i<3;i++)
			dummy.append(QString(""));
		return(dummy);
	}
}

QVariant NSScriptHelper::MakeRemoteNSCall(QString ServerIP, const char* call_name, QString* str1, QString* str2) {
	// Factored out to make an NS xml_rpc call
	Logger("Name Server Scripthelper",
		   "server_log").WriteLogLine(QString("Remote"),
			QString("(Script) Making remote request: call(%1), server(%2).").arg(call_name).arg(ServerIP));
	client = new xmlrpc::Client(this);
	int port_number = QSettings("../NameServer/nameserver_config", QSettings::IniFormat).value("port", 0).toInt();
	QString address = ServerIP;
	client->setHost(address, port_number);
	connect( client, SIGNAL(done( int, QVariant )),
		this, SLOT(processNSReturnValue( int, QVariant )) );
	connect( client, SIGNAL(failed( int, int, QString )),
		this, SLOT(processNSFault( int, int, QString )) );
	if (str1==NULL && str2==NULL) {
		client->request(call_name);
	}
	else if (str2==NULL && str1!=NULL) {
		client->request(call_name, *str1);
	}
	else if (str2!=NULL && str1 != NULL) {
		client->request(call_name, *str1, *str2);
	}
	QEventLoop().processEvents(QEventLoop::AllEvents, 200000);
	event_loop.exec();
	return(script_ret_val);
}

void NSScriptHelper::processNSReturnValue( int requestId, QVariant value ) {
	Q_UNUSED(requestId)
	script_ret_val = value;
	event_loop.processEvents(QEventLoop::AllEvents);
	event_loop.exit();
}

void NSScriptHelper::processNSFault( int requestId, int errorCode, QString errorString ) {
	Q_UNUSED( requestId );
	Logger("Name Server Scripthelper",
		   "server_log").WriteLogLine(QString("Error"),
			QString("(Script helper) Error in communication with name server: Code(%1), String(%2).").arg(errorCode).arg(errorString));
	script_ret_val = QVariant();
	event_loop.exit();
}

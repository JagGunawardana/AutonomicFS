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

QVariant NSScriptHelper::GetActiveAppServers(void) {
	return(server->GetActiveApplicationServers());
}

QVariant NSScriptHelper::GetActiveNameServers(void) {
	return(xmlrpc::Variant(server->GetActiveNameServers()));
}

QVariant NSScriptHelper::GetIPAddress(void) {
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
	// !!! how to return error here?
	event_loop.exit();
}

QVariant NSScriptHelper::DummyCall(QVariantMap server, QString dummy_string) {
	return(MakeAppCall("Dummy", server, &dummy_string));
}

QVariant NSScriptHelper::GetAllFilesUnderMgt(QVariantMap server) {
	return(MakeAppCall("Service_GetAllFilesUnderMgt", server));
}

QVariant NSScriptHelper::TryGetFileByName(QVariantMap server, QString file_name) {
	// Ask app server for file (by name)
	return(MakeAppCall("Service_FileByName", server, &file_name));
}

QVariant NSScriptHelper::TryGetFileByHash(QVariantMap server, QString file_hash) {
	// Ask app server for file (by hash)
	return(MakeAppCall("Service_FileByHash", server, &file_hash));
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

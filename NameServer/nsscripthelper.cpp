#include "nsscripthelper.h"
#include "server.h"

#include "../RPC/xml_rpc/client.h"
#include "../SharedServices/logger.h"

NSScriptHelper::NSScriptHelper(QObject* parent,
							   Server* server,
							   QMap<QString, QVariant> params) :
QObject(parent), script_ret_val(), sem_sync()
{
	this->server = server;
	this->params = params;
	sem_sync.release(0);
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

QVariant NSScriptHelper::HasFile(QVariant file_store) {
	if (file_store.isValid() && file_store.type()==QVariant::List &&
		file_store.toList()[0] == true) {
		return(true);
	}
	else
		return(QVariant(false));
}

QVariant NSScriptHelper::ExtractFile(QVariant file_store) {
	if (file_store.isValid() && file_store.type()==QVariant::List &&
		file_store.toList()[0] == true) {
		return(file_store.toList()[1]);
	}
	else
		return(QVariant(""));
}

QVariant NSScriptHelper::TryGetFileByName(QVariant server, QString file_name) {
	client = new xmlrpc::Client(this);
	client->setHost(QString("127.0.0.1"), server.toMap()["port"].toInt());
	connect( client, SIGNAL(done( int, QVariant )),
		this, SLOT(processReturnValue( int, QVariant )) );
	connect( client, SIGNAL(failed( int, int, QString )),
		this, SLOT(processFault( int, int, QString )) );
	client->request( "Service_FileByName",
		file_name);
	sem_sync.acquire(10); // wait for return ???
	return(script_ret_val);
}

void NSScriptHelper::processReturnValue( int requestId, QVariant value ) {
	Q_UNUSED(requestId)
	script_ret_val = value;
	sem_sync.release(10);
}

void NSScriptHelper::processFault( int requestId, int errorCode, QString errorString ) {
	Q_UNUSED( requestId );
	Logger("Name Server Scripthelper",
		   "server_log").WriteLogLine(QString("Error"),
			QString("Error in communication with application server: Code(%1), String(%2).").arg(errorCode).arg(errorString));
}

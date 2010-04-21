#include "nsscripthelper.h"
#include "server.h"

#include "../RPC/xml_rpc/client.h"
#include "../SharedServices/logger.h"

#include <QDebug>

NSScriptHelper::NSScriptHelper(QObject* parent,
							   Server* server,
							   QMap<QString, QVariant> params) :
QObject(parent), script_ret_val(), event_loop(parent)
{
	this->server = server;
	this->params = params;
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

QVariant NSScriptHelper::TryGetFileByName(QVariantMap server, QString file_name) {
	client = new xmlrpc::Client(this);
	int port_number =server[QString("port")].toInt();
	QString address = "localhost";
	client->setHost(address, port_number);
	connect( client, SIGNAL(done( int, QVariant )),
		this, SLOT(processReturnValue( int, QVariant )) );
	connect( client, SIGNAL(failed( int, int, QString )),
		this, SLOT(processFault( int, int, QString )) );
	int requestId = client->request( "Service_FileByName",
		file_name);
	qDebug()<<"Got id "<<requestId;
	event_loop.exec();
	return(script_ret_val);
}

void NSScriptHelper::processReturnValue( int requestId, QVariant value ) {
	Q_UNUSED(requestId)
	qDebug()<<"Received return value "<<value;
	script_ret_val = value;
	event_loop.exit();
}

QVariant NSScriptHelper::DummyCall(QVariantMap server, QString dummy_string) {
	client = new xmlrpc::Client(this);
	int port_number =server[QString("port")].toInt();
	QString address = "localhost";
	client->setHost(address, port_number);
	connect( client, SIGNAL(done( int, QVariant )),
		this, SLOT(processReturnValue( int, QVariant )) );
	connect( client, SIGNAL(failed( int, int, QString )),
		this, SLOT(processFault( int, int, QString )) );
	client->request( "Dummy",
		dummy_string);
	event_loop.exec();
	return(script_ret_val);
}

void NSScriptHelper::processFault( int requestId, int errorCode, QString errorString ) {
	Q_UNUSED( requestId );
	Logger("Name Server Scripthelper",
		   "server_log").WriteLogLine(QString("Error"),
			QString("Error in communication with application server: Code(%1), String(%2).").arg(errorCode).arg(errorString));
	// !!! how to return error here?
	event_loop.exit();
}

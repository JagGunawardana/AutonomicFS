#include "nsscripthelper.h"
#include "server.h"

#include "../RPC/xml_rpc/client.h"

NSScriptHelper::NSScriptHelper(QObject* parent,
							   Server* server,
							   QMap<QString, QVariant> params) :
	QObject(parent) {
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

QVariant NSScriptHelper::TryGetFile(QVariant server, QString file_name) {
	client = new xmlrpc::Client(this);
	client->setHost(QString("127.0.0.1"), server["port"].toInt());
	connect( client, SIGNAL(done( int, QVariant )),
		this, SLOT(processReturnValue( int, QVariant )) );
	connect( client, SIGNAL(failed( int, int, QString )),
		this, SLOT(processFault( int, int, QString )) );

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

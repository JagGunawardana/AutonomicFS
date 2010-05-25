#include "nsclient.h"
#include "../RPC/xml_rpc/client.h"
#include "../SharedServices/logger.h"
#include "filemanager.h"

#include <QtCore/QCoreApplication>
#include <QVariantList>
#include <QSettings>

NSClient::NSClient( const QString &address,
					QString this_server_name,
					QObject *parent )
			: QObject( parent ), address( address ) {
	port = QSettings("../NameServer/nameserver_config", QSettings::IniFormat).value("port", 0).toInt();
	server_name = this_server_name;
	Logger("Application NSClient",
		   "../NameServer/server_log").WriteLogLine(QString("Registration"),
			QString("Connecting to name server: application name (%1), port (%2)").arg(server_name).arg(port));

	client = new xmlrpc::Client(this);
	client->setHost(address, port);
	connect( client, SIGNAL(done( int, QVariant )),
		this, SLOT(processReturnValue( int, QVariant )) );
	connect( client, SIGNAL(failed( int, int, QString )),
		this, SLOT(processFault( int, int, QString )) );
}

NSClient::~NSClient() {
	if (client!=NULL)
		delete(client);
}

void NSClient::RegisterWithNameServer(int application_port, QString server_type) {
	int our_pid = QCoreApplication::applicationPid();
	client->request( "RegisterAppServer",
		server_name, our_pid, application_port, server_type);
	Logger("Application Server NSClient",
		   "../NameServer/server_log").WriteLogLine(QString("Registration"),
			QString("Registered successfully with name server: application name (%1), port (%2)").arg(server_name).arg(port));
}

void NSClient::Ping(int application_port, int read_count, int write_count) {
	int our_pid = QCoreApplication::applicationPid();
	QList<xmlrpc::Variant> list;
	list.append(xmlrpc::Variant(our_pid));
	list.append(xmlrpc::Variant(application_port));
	list.append(xmlrpc::Variant(read_count));
	list.append(xmlrpc::Variant(write_count));
	list.append(xmlrpc::Variant(server_name));
	client->request( "Ping",
		list);
	Logger("Application Server NSClient",
		   "../NameServer/server_log").WriteLogLine(QString("Ping"),
			QString("Sending ping from application server(%1), pid(%2).").arg(server_name).arg(our_pid));
}

void NSClient::processReturnValue( int requestId, QVariant value ) {
	Q_UNUSED(requestId)
	Q_UNUSED(value)
}

void NSClient::processFault( int requestId, int errorCode, QString errorString ) {
	Q_UNUSED( requestId );
	Logger("Application Server NSClient",
		   "../NameServer/server_log").WriteLogLine(QString("Error"),
			QString("Error in communication: Code(%1), String(%2).").arg(errorCode).arg(errorString));
}


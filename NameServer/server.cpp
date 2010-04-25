#include "server.h"
#include "../RPC/xml_rpc/server.h"
#include "../RPC/xml_rpc/client.h"

#include "applicationserver.h"
#include <QHostAddress>
#include <QHostInfo>
#include <QDebug>

#include "../SharedServices/logger.h"
#include "servicerequest.h"


Server::Server( quint16 port, QObject *parent )
	  : QObject( parent ) {
	srv = new xmlrpc::Server(this);
	tick = QSettings("nameserver_config", QSettings::IniFormat).value("periodic_tick", 0).toInt();
	keep_alive_gap = QSettings("nameserver_config", QSettings::IniFormat).value("KeepAlive", 0).toInt();
	//register our methods
	srv->registerMethod( "RegisterAppServer", QVariant::Bool, QVariant::String, QVariant::Int, QVariant::Int, QVariant::String );
	srv->registerMethod( "Ping", QVariant::Bool, QVariant::Int );
	srv->registerMethod("Service_RequestFile", QVariant::ByteArray, QVariant::String);
	srv->registerMethod("Service_GetAllFilesUnderMgt",QVariant::List);
	connect(srv, SIGNAL(incomingRequest( int, QString, QList<xmlrpc::Variant>)),
		this, SLOT(processRequest( int, QString, QList<xmlrpc::Variant>)));

	if(! srv->listen( port ) )
		Logger("Name server", "server_log").WriteLogLine(QString("Registration"), QString("Error with Name server starting on host %1, using port %2....").arg(QHostInfo::localHostName()).arg(port));
}

Server::~Server() {
	delete(srv);
// Clear up the map
	foreach(ApplicationServer* app, appserver_map)
		delete(app);
	appserver_map.clear();
}

void Server::processRequest( int requestId, QString methodName,
							 QList<xmlrpc::Variant> parameters ) {
	if (methodName == "RegisterAppServer") {
		Logger("Name Server", "server_log").WriteLogLine(QString("Service"),
				QString("Name server service request name(%2).").arg(methodName));
		QVariant ret_val = RegisterAppServer(parameters[0], parameters[1],
						  parameters[2], parameters[3]);
		srv->sendReturnValue( requestId, ret_val.toBool());
	}
	else if (methodName == "Ping") {
		QVariant ret_val = Ping(parameters[0]);
		srv->sendReturnValue( requestId, ret_val.toBool());
	}
	else if (methodName == "Service_RequestFile") {
		Logger("Name Server", "server_log").WriteLogLine(QString("Service"),
				QString("Name server service request name(%2).").arg(methodName));
		ServiceRequest *request = new ServiceRequest(srv, this, parameters,
			requestId, ServiceRequest::request_file);
		request->setAutoDelete(true);
		QThreadPool::globalInstance()->start(request);
		request->TransferSocket();
	}
	else if (methodName == "Service_GetAllFilesUnderMgt") {
		Logger("Name Server", "server_log").WriteLogLine(QString("Service"),
				QString("Name server service request name(%2).").arg(methodName));
		ServiceRequest *request = new ServiceRequest(srv, this, parameters,
			requestId, ServiceRequest::request_filesundermgt);
		request->setAutoDelete(true);
		QThreadPool::globalInstance()->start(request);
		request->TransferSocket();
	}
	else {
		qDebug() << QString("Name server - bad service name given ("+methodName+").");
		srv->sendReturnValue(requestId, QVariant(true).toBool());
	}
}

QVariant Server::RegisterAppServer(QVariant server_name,
	QVariant pid, QVariant port_number,
	QVariant server_type) {
// Called by each application server when it starts
	ApplicationServer* app = appserver_map[pid.toInt()] = new ApplicationServer(pid.toInt(), port_number.toInt(),
													   server_name.toString(), server_type);
	Logger("Name Server", "server_log").WriteLogLine(QString("Registration"),
			QString("Application server registration pid(%1), name(%2), on port(%3).").arg(app->GetPid()).arg(app->GetServerName()).arg(app->GetPortNumber()));
	return(QVariant(true));
}

QVariant Server::Ping(QVariant pid) {
// Called periodically by each application server
	if (!appserver_map.contains(pid.toInt())) {
		Logger("Name server", "server_log").WriteLogLine(QString("KeepAlive"),
				QString("Keep alive message from pid(%1), no registered server matches.").arg(pid.toInt()) );
		return(QVariant(false));
	}
	ApplicationServer* app = appserver_map[pid.toInt()];
	int gap = app->KeepAliveMessage();
	Logger("Name server", "server_log").WriteLogLine(QString("KeepAlive"),
			QString("Keep alive message from pid(%1), name(%2), keep alive gap(%3).").arg(app->GetPid()).arg(app->GetServerName()).arg(gap));
	return(QVariant(true));
}

// Script helper stuff

QList<int> Server::GetActiveApplicationServerPorts(void) {
	QList<int> list;
	foreach(ApplicationServer* app, appserver_map) {
		if (app->GetKeepAliveGap() < tick*keep_alive_gap*2)
			list.append(app->GetPortNumber());
	}
	return(list);
}

QVariantMap Server::GetActiveApplicationServers(void) {
	QVariantMap our_map;
	foreach(ApplicationServer* app, appserver_map) {
		if (app->GetKeepAliveGap() < tick*keep_alive_gap*2) {
			QVariantMap attrs;
			attrs[QString("port")]=QVariant(app->GetPortNumber());
			attrs[QString("name")]=QVariant(app->GetServerName());
			attrs[QString("gap")]=QVariant(app->GetKeepAliveGap());
			our_map[QString(app->GetPid())] = attrs;
		}
	}
	return(our_map);
}

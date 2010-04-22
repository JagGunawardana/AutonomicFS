#include "server.h"
#include "../SharedServices/logger.h"
#include <QHostInfo>

#include "applicationservicerequest.h"

Server::Server(quint16 suggested_port, QString server_name, QObject* parent) :
		QObject(parent)
{
	srv = new xmlrpc::Server(this);
	this->server_name = server_name;
	//register our methods
	srv->registerMethod("Service_FileByName", QVariant::Map, QVariant::String);
	srv->registerMethod("Dummy", QVariant::String, QVariant::String);

	// Connect to our processor
	connect(srv, SIGNAL(incomingRequest( int, QString, QList<xmlrpc::Variant>)),
		this, SLOT(processRequest( int, QString, QList<xmlrpc::Variant>)));

	if (suggested_port != 0) {
		port = suggested_port;
		if(! srv->listen( port ) ) {
			Logger("Application server",
				   "../NameServer/server_log").WriteLogLine(QString("Startup"),
					QString("Error with Application server starting on host %1, using port %2....").arg(QHostInfo::localHostName()).arg(port));
			qFatal("Application server: Failed to listen on port.");
		}
		else
			port = suggested_port;
	}
	else {
		// find a free port
		int min_port = QSettings("../NameServer/nameserver_config", QSettings::IniFormat).value("AppServerPortMin", 0).toInt();
		int max_port = QSettings("../NameServer/nameserver_config", QSettings::IniFormat).value("AppServerPortMax", 0).toInt();
		int current_port = min_port;
		port = 0;
		while(current_port<=max_port) {
			if (srv->listen(current_port)) {
				port = current_port;
				Logger("Application server",
					   "../NameServer/server_log").WriteLogLine(QString("Startup"),
						QString("Application server, name %1, listening on port %2....").arg(server_name).arg(port));
				break;
			}
			current_port++;
		}
		if (port==0) {
			Logger("Application server",
				   "../NameServer/server_log").WriteLogLine(QString("Startup"),
					QString("Error with Application server, failed to find a free port in the range %1 - %2....").arg(min_port).arg(max_port));
			qFatal("Application server: Failed to listen on port.");
		}
	}
}

Server::~Server() {
	delete(srv);
}

void Server::processRequest( int requestId, QString methodName,
							 QList<xmlrpc::Variant> parameters ) {
	Logger("Application server",
		   "../NameServer/server_log").WriteLogLine(QString("Script"),
			QString("Application server name (%1), on port (%2) received request name (%3)....").arg(server_name).arg(port).arg(methodName));
	if (methodName == "Service_FileByName") {
		ApplicationServiceRequest* request = new ApplicationServiceRequest(srv, parameters, requestId,
			ApplicationServiceRequest::request_FileByName);
		request->setAutoDelete(true);
		QThreadPool::globalInstance()->start(request);
		request->TransferSocket();
	}
	else if (methodName == "Service_FileByHash") {
		QVariant ret_val;
		srv->sendReturnValue( requestId, ret_val.toBool());
	}
	else if (methodName == "Dummy") {
		QVariant ret_val = "Hello dummy";
		srv->sendReturnValue( requestId, ret_val.toString());
	}
	else {
		qDebug() << QString("Application server - bad service name given ("+methodName+").");
		srv->sendReturnValue(requestId, QVariant(false).toBool());
	}
}



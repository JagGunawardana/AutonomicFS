#include "../RPC/xml_rpc/server.h"
#include "../RPC/xml_rpc/client.h"

#include "applicationserver.h"
#include <QHostAddress>
#include <QHostInfo>
#include <QDebug>
#include <QtNetwork>

#include "server.h"
#include "../SharedServices/logger.h"
#include "servicerequest.h"
#include "nameserver.h"

Server::Server( quint16 port, QObject *parent )
	  : QObject( parent ) {
	// Setup server
	srv = new xmlrpc::Server(this);
	tick = QSettings("nameserver_config", QSettings::IniFormat).value("periodic_tick", 0).toInt();
	keep_alive_gap = QSettings("nameserver_config", QSettings::IniFormat).value("KeepAlive", 0).toInt();
	//register our methods
	srv->registerMethod( "RegisterAppServer", QVariant::Bool, QVariant::String, QVariant::Int, QVariant::Int, QVariant::String );
	srv->registerMethod( "Ping", QVariant::Bool, QVariant::List);
	srv->registerMethod("Service_RequestFileByName", QVariant::List, QVariant::String);
	srv->registerMethod("Service_RequestFileByHash", QVariant::List, QVariant::String);
	srv->registerMethod("Service_GetAllLocalFilesUnderMgt",QVariant::List);
	srv->registerMethod("Service_SaveFile", QVariant::Bool, QVariant::String, QVariant::ByteArray, QVariant::Bool);
	srv->registerMethod("Service_DeleteFile", QVariant::Bool, QVariant::String);
	srv->registerMethod("Client_RequestFileByName", QVariant::List, QVariant::String);
	srv->registerMethod("Client_RequestFileByHash", QVariant::List, QVariant::String);
	srv->registerMethod("Client_GetAllLocalFilesUnderMgt",QVariant::List);
	srv->registerMethod("Client_GetAllFilesUnderMgt",QVariant::List);
	srv->registerMethod("Client_GetAllNameServers",QVariant::List);
	srv->registerMethod("Client_SaveFile", QVariant::Bool, QVariant::String, QVariant::ByteArray);
	srv->registerMethod("Client_DeleteFile", QVariant::Bool, QVariant::String);
	connect(srv, SIGNAL(incomingRequest( int, QString, QList<xmlrpc::Variant>)),
		this, SLOT(processRequest( int, QString, QList<xmlrpc::Variant>)));

	if(! srv->listen( port ) )
		Logger("Name server", "server_log").WriteLogLine(QString("Registration"),
			QString("Error with Name server starting on host %1, using port %2....").arg(QHostInfo::localHostName()).arg(port));

	// Our timer
	life_timer = new QTime();
	life_timer->start();

	// Setup broadcast stuff
	// Listener first
	broadcastListener = new QUdpSocket(this);
	broadcastPort = QSettings("nameserver_config", QSettings::IniFormat).value("broadcast_port", 0).toInt();
	broadcastListener->bind(broadcastPort, QUdpSocket::ShareAddress);
	connect(broadcastListener, SIGNAL(readyRead()),
			this, SLOT(processNSBroadcast()));
	// Broadcaster next
	// Setup our periodic timer
	broadcast_timer = new QTimer(this);
	connect(broadcast_timer , SIGNAL(timeout()), this, SLOT(SendBroadcast()));
	ns_tick = QSettings("nameserver_config", QSettings::IniFormat).value("broadcast_interval", 0).toInt();
	broadcast_timer->start(tick);

	// Periodic processes i.e. load calculation, lazy updater, replicator
	// Setup our periodic timer
	periodic_timer = new QTimer(this);
	connect(periodic_timer , SIGNAL(timeout()), this, SLOT(Periodics()));
	periodic_interval = QSettings("nameserver_config", QSettings::IniFormat).value("periodic_interval", 0).toInt();
	periodic_timer->start(periodic_interval);
}

Server::~Server() {
	delete(srv);
// Clear up the map
	foreach(ApplicationServer* app, appserver_map)
		delete(app);
	appserver_map.clear();
	foreach(NameServer* ns, nameserver_map)
		delete(ns);
	nameserver_map.clear();
	delete(broadcast_timer);
	delete(periodic_timer);
	delete(broadcastListener);
	delete(life_timer);
}

void Server::Periodics(void) {
		Logger("Name Server", "server_log").WriteLogLine(QString("Periodic_process"),
				QString("Running periodic processes."));
		QList<xmlrpc::Variant> parameters;
		ServiceRequest *request = new ServiceRequest(NULL, this, parameters,
			-1, ServiceRequest::request_periodicprocesses);
		request->SetSingleMutex(&periodic_mutex);
		request->setAutoDelete(true);
		QThreadPool::globalInstance()->start(request);
		request->DummyTransferSocket();
}

void Server::SendBroadcast(void) {
	QUdpSocket udpSocket(this);
	QByteArray datagram = "";
	datagram.append(QHostInfo::localHostName());
	double our_load = GetNSLoad();
	datagram.append(QString(":%1").arg(our_load));
	udpSocket.writeDatagram(datagram.data(), datagram.size(),
							 QHostAddress::Broadcast, broadcastPort);
	Logger("Name Server", "server_log").WriteLogLine(QString("Periodic_process"),
			QString("Sending name server broadcast.."));
}

void Server::processNSBroadcast(void) {
	while (broadcastListener->hasPendingDatagrams()) {
		QByteArray datagram;
		datagram.resize(broadcastListener->pendingDatagramSize());
		QHostAddress host;
		quint16 port;
		broadcastListener->readDatagram(datagram.data(), datagram.size(), &host, &port);
		QString bc(datagram.data());
		QStringList sl = bc.split(":");
		QString remote_host = sl.at(0);
		QString load_str = sl.at(1);
		double load = load_str.toDouble();
		if (remote_host!=QString(QHostInfo::localHostName())) {
			// Process broadcast address, if not received before then register, otherwise process as keep alive message
			if (nameserver_map.contains(host.toString())) {
				nameserver_map[host.toString()]->KeepAliveMessage();
				nameserver_map[host.toString()]->SetLoad(load);
				Logger("Name Server", "server_log").WriteLogLine(QString("KeepAlive"),
						QString("Received name server broadcast from host %1").arg(host.toString()));
			}
			else {
				nameserver_map[host.toString()] = new NameServer(host.toString(), port);
				nameserver_map[host.toString()]->SetLoad(load);
				Logger("Name Server", "server_log").WriteLogLine(QString("Registration"),
						QString("Name server registration Address(%1), on port(%2).").arg(host.toString()).arg(port));
			}
			Logger("Name Server", "server_log").WriteLogLine(QString("Periodic_process"),
				QString("Received name server broadcast: %1").arg(datagram.data()));
		}
	}
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
	else if (methodName == "Client_GetAllFilesUnderMgt") {
		Logger("Name Server", "server_log").WriteLogLine(QString("Service"),
				QString("Name server service request name(%2).").arg(methodName));
		ServiceRequest *request = new ServiceRequest(srv, this, parameters,
			requestId, ServiceRequest::request_allfilesundermgt);
		request->setAutoDelete(true);
		QThreadPool::globalInstance()->start(request);
		request->TransferSocket();
	}
	else if (methodName == "Service_RequestFileByName") {
		Logger("Name Server", "server_log").WriteLogLine(QString("Service"),
				QString("Name server service request name(%2).").arg(methodName));
		ServiceRequest *request = new ServiceRequest(srv, this, parameters,
			requestId, ServiceRequest::request_local_file_byname);
		request->setAutoDelete(true);
		QThreadPool::globalInstance()->start(request);
		request->TransferSocket();
	}
	else if (methodName == "Service_RequestFileByHash") {
		Logger("Name Server", "server_log").WriteLogLine(QString("Service"),
				QString("Name server service request name(%2).").arg(methodName));
		ServiceRequest *request = new ServiceRequest(srv, this, parameters,
			requestId, ServiceRequest::request_local_file_byhash);
		request->setAutoDelete(true);
		QThreadPool::globalInstance()->start(request);
		request->TransferSocket();
	}
	else if (methodName == "Client_RequestFileByName") {
		Logger("Name Server", "server_log").WriteLogLine(QString("Service"),
				QString("Name server service request name(%2).").arg(methodName));
		ServiceRequest *request = new ServiceRequest(srv, this, parameters,
			requestId, ServiceRequest::request_file_byname);
		request->setAutoDelete(true);
		QThreadPool::globalInstance()->start(request);
		request->TransferSocket();
	}
	else if (methodName == "Client_RequestFileByHash") {
		Logger("Name Server", "server_log").WriteLogLine(QString("Service"),
				QString("Name server service request name(%2).").arg(methodName));
		ServiceRequest *request = new ServiceRequest(srv, this, parameters,
			requestId, ServiceRequest::request_file_byhash);
		request->setAutoDelete(true);
		QThreadPool::globalInstance()->start(request);
		request->TransferSocket();
	}
	else if (methodName == "Service_GetAllLocalFilesUnderMgt" || methodName == "Client_GetAllLocalFilesUnderMgt") {
		Logger("Name Server", "server_log").WriteLogLine(QString("Service"),
				QString("Name server service request name(%2).").arg(methodName));
		ServiceRequest *request = new ServiceRequest(srv, this, parameters,
			requestId, ServiceRequest::request_localfilesundermgt);
		request->setAutoDelete(true);
		QThreadPool::globalInstance()->start(request);
		request->TransferSocket();
	}
	else if (methodName == "Client_GetAllNameServers") {
		Logger("Name Server", "server_log").WriteLogLine(QString("Service"),
				QString("Name server service request name(%2).").arg(methodName));
		QList<xmlrpc::Variant> others = GetActiveNameServers();
		QString my_ip = GetIPAddress(requestId);
		others.append(xmlrpc::Variant(my_ip));
		srv->sendReturnValue( requestId, xmlrpc::Variant(others));
	}
	else if (methodName == "Service_SaveFile") {
		Logger("Name Server", "server_log").WriteLogLine(QString("Service"),
				QString("Name server service request name(%2).").arg(methodName));
		ServiceRequest *request = new ServiceRequest(srv, this, parameters,
			requestId, ServiceRequest::request_savelocalfile);
		request->setAutoDelete(true);
		QThreadPool::globalInstance()->start(request);
		request->TransferSocket();
	}
	else if (methodName == "Client_SaveFile") {
		Logger("Name Server", "server_log").WriteLogLine(QString("Service"),
				QString("Name server service request name(%2).").arg(methodName));
		ServiceRequest *request = new ServiceRequest(srv, this, parameters,
			requestId, ServiceRequest::request_savefile);
		request->setAutoDelete(true);
		QThreadPool::globalInstance()->start(request);
		request->TransferSocket();
	}
	else if (methodName == "Client_DeleteFile") {
		Logger("Name Server", "server_log").WriteLogLine(QString("Service"),
				QString("Name server service request name(%2).").arg(methodName));
		ServiceRequest *request = new ServiceRequest(srv, this, parameters,
			requestId, ServiceRequest::request_deletefile);
		request->setAutoDelete(true);
		QThreadPool::globalInstance()->start(request);
		request->TransferSocket();
	}
	else if (methodName == "Service_DeleteFile") {
		Logger("Name Server", "server_log").WriteLogLine(QString("Service"),
				QString("Name server service request name(%2).").arg(methodName));
		ServiceRequest *request = new ServiceRequest(srv, this, parameters,
			requestId, ServiceRequest::request_deletelocalfile);
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

QVariant Server::Ping(QVariant params) {
	QList<QVariant> list = params.toList();
	QVariant pid = list.at(0);
	QVariant application_port = list.at(1);
	QVariant read_count = list.at(2);
	QVariant write_count = list.at(3);
	QVariant server_name = list.at(4);
// Called periodically by each application server
	if (!appserver_map.contains(pid.toInt())) {
		Logger("Name server", "server_log").WriteLogLine(QString("KeepAlive"),
				QString("Keep alive message from pid(%1), no registered server matches.").arg(pid.toInt()) );
		// Try to register
		RegisterAppServer(server_name, pid, application_port, QVariant(QString("NEUTRAL")));
	}
	ApplicationServer* app = appserver_map[pid.toInt()];
	int gap = app->KeepAliveMessage();
	app->SetReadCount(read_count.toInt());
	app->SetWriteCount(write_count.toInt());
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

double Server::GetLoadOfServer(QVariant IP) {
	QString ip_address = IP.toString();
	if (ip_address == "localhost" || ip_address == "127.0.0.1") {
		return(GetNSLoad());
	}
	if (nameserver_map.contains(ip_address)) {
		return(nameserver_map[ip_address]->GetLoad());
	}
	else
		return(-1.0);
}

QString Server::GetIdleNameServerFromList(QVariant list) {
	QVariantList ns_list = list.toList();
	if (ns_list.size()==0)
		return(QString(""));
	double load = -1;
	int j = 0;
	for(int i=0;i<ns_list.size();i++) {
		NameServer* ns = nameserver_map[ns_list.at(i).toString()];
		double current_load = ns->GetLoad();
		if (load<0 || current_load<load) {
			j = i;
			load = current_load;
		}
	}
	return(ns_list.at(j).toString());
}

QList<xmlrpc::Variant> Server::GetActiveNameServers(void) {
	QList<xmlrpc::Variant> our_addresses;
	foreach(NameServer* ns, nameserver_map) {
		if ((ns->GetKeepAliveGap()) < ns_tick*2)
			our_addresses.append(xmlrpc::Variant(ns->GetAddress()));
	}
	return(our_addresses);
}

double Server::GetNSLoad(void) {
	// Get the average load for all our application servers
	double total_load = 0.0;
	int count = 0;
	foreach(ApplicationServer* app, appserver_map) {
		if (app->GetKeepAliveGap() < tick*keep_alive_gap*2) { // Is this one still alive
			count++;
			total_load += app->GetLoadRate();
		}
	}
	return(total_load/count);
}

QVariant Server::GetIdleApplicationServer(void) {
	QVariantMap app_map;
	double app_load = -1;
	foreach(ApplicationServer* app, appserver_map) {
		if (app->GetKeepAliveGap() < tick*keep_alive_gap*2) { // Is this one still alive
			double current_load = app->GetLoadRate();
			if (app_load<0 || current_load < app_load) {
				app_load = current_load;
				app_map.clear();
				app_map[QString("success")] = QVariant(true);
				app_map[QString("port")] = QVariant(app->GetPortNumber());
				app_map[QString("name")]= QVariant(app->GetServerName());
				app_map[QString("gap")]=QVariant(app->GetKeepAliveGap());
			}
		}
	}
	if (!app_map.contains(QString("name")))
		app_map[QString("success")] = QVariant(false);
	return(app_map);
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

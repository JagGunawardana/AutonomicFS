#include <QtCore/QCoreApplication>
#include "appperiodicprocess.h"
#include <QTimer>
#include <QDebug>
#include <QtScript>

#include "nsclient.h"
#include "../SharedServices/logger.h"
#include "scriptrunner.h"

AppPeriodicProcess::AppPeriodicProcess(QString our_server_name, int server_port,
	QObject* parent) : QThread(parent) {
	server_name = our_server_name;
	ns_client = NULL;
	periodic_timer = NULL;
	tick_count = 0;
	port = server_port;
}

AppPeriodicProcess::~AppPeriodicProcess() {
	periodic_timer->stop();
	delete(periodic_timer);
	delete(ns_client);
}

void AppPeriodicProcess::run(void) {
	QTimer::singleShot(10, this, SLOT(SetupPeriodicProcesses())); // One shot timer to setup
	exec();
}

void AppPeriodicProcess::SetupPeriodicProcesses(void) {
	// Register our presense with the name server
	QString address = "localhost";
	ns_client = new NSClient(address, server_name);
	ns_client->RegisterWithNameServer(port, "NEUTRAL");

	// Setup our periodic timer
	periodic_timer = new QTimer(this);
	connect(periodic_timer, SIGNAL(timeout()), this, SLOT(PeriodicProcesses()));
	int tick = QSettings("../NameServer/nameserver_config", QSettings::IniFormat).value("periodic_tick", 0).toInt();
	periodic_timer->start(tick);
}

bool AppPeriodicProcess::TimeFor(QString event_name) {
	int tick = QSettings("../NameServer/nameserver_config", QSettings::IniFormat).value(event_name, 0).toInt();
	if (tick_count%tick == 0) {
		Logger("Application Server", "../NameServer/server_log").WriteLogLine(QString("Periodic_process"),
				QString("Time to %1, interval %2..").arg(event_name).arg(tick) );

		return(true);
	}
	else
		return(false);
}

void AppPeriodicProcess::PeriodicProcesses(void) {
	tick_count++;
	if (TimeFor("KeepAlive")) {
		// Keep alive message to name server
		if (ns_client!=NULL)
			ns_client->Ping();
	}
	if (TimeFor("ReportLoad")) {
		// Report on load
		//ScriptRunner script("report_load");
		//int load;
		//script.GetResult(load);
	}
	// Report on files under management

	// Periodic scripts

}

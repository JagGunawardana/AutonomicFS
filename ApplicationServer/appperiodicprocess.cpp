#include <QtCore/QCoreApplication>
#include "appperiodicprocess.h"
#include <QTimer>
#include <QDebug>

#include "nsclient.h"
#include "../SharedServices/logger.h"

AppPeriodicProcess::AppPeriodicProcess(QString our_server_name,
	QObject* parent) : QThread(parent) {
	server_name = our_server_name;
	ns_client = NULL;
	periodic_timer = NULL;
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
		QString address = "127.0.0.1";
		ns_client = new NSClient(address, server_name);
		ns_client->RegisterWithNameServer(100 /* !!! application port */, "NEUTRAL");

	// Setup our periodic timer
	periodic_timer = new QTimer(this);
	connect(periodic_timer, SIGNAL(timeout()), this, SLOT(PeriodicProcesses()));
	periodic_timer->start(500); // half a second should be enough for our purposes
}

void AppPeriodicProcess::PeriodicProcesses(void) {
	// Keep alive message to name server
	if (ns_client!=NULL)
		ns_client->Ping();
	// Report on load

	// Report on files under management

	// Periodic scripts

}

#include <QtCore/QCoreApplication>
#include <QSettings>
#include <QDebug>
#include <QHostInfo>

#include "nsclient.h"
#include "filemanager.h"
#include "../SharedServices/logger.h"
#include "appperiodicprocess.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

	if (argc!=3) {
		qFatal("Error: usage %s server_name file_store_directory", argv[0]);
	}
	QString server_name = argv[1];
	QString file_store = argv[2];
	Logger log("Application Server", "server_log");
	log.WriteLogLine(QString("Registration"),
		QString("Application server starting on host %1, name %2, file store %3....").arg(QHostInfo::localHostName()).arg(server_name).arg(argv[2]));

// Setup the file management object
	log.WriteLogLine(QString("Startup"),
					 QString("Starting file management for name %1.").arg(server_name));
	FileManager* file_mgr = new FileManager(server_name, file_store);
	file_mgr->ScanFullFileStore();
// Setup our periodic processes
	log.WriteLogLine(QString("Startup"),
					 QString("Starting application server periodic processes for name %1.").arg(server_name));
	AppPeriodicProcess* app = new AppPeriodicProcess(server_name);
	app->start();

    return a.exec();
}

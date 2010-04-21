#include <QtCore/QCoreApplication>
#include <QSettings>
#include <QDebug>
#include <QHostInfo>
#include <QDir>

#include "server.h"
#include "../SharedServices/logger.h"
#include "../SharedServices/profilemgr.h"

int main(int argc, char *argv[]) {
	QCoreApplication app( argc, argv );
// Read required config parameters
	QSettings settings("nameserver_config", QSettings::IniFormat);
	QString address = "localhost";
	quint16 port = settings.value("port", 0).toInt();
	Logger log("Name server", "server_log");
	log.WriteLogLine(QString("Registration"), QString("Name server starting on host %1, using port %2....").arg(QHostInfo::localHostName()).arg(port));
	ProfileMgr::GetProfileManager(QDir("scripts").absolutePath());
	Server s( port );

	return app.exec();
}

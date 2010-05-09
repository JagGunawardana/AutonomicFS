#include <QTime>
#include <QString>
#include <QVariant>

#include "applicationserver.h"

ApplicationServer::ApplicationServer(int new_pid, int new_port_number, QString new_server_name, QVariant new_server_type) {
	pid = new_pid;
	port_number = new_port_number;
	server_type = new_server_type;
	server_name = new_server_name;
	last_keep_alive_gap = 0;
	last_time.start();
}

int ApplicationServer::KeepAliveMessage(void) {
	last_keep_alive_gap = last_time.elapsed();
	last_time.restart();
	return(last_keep_alive_gap);
}

int ApplicationServer::GetKeepAliveGap(void) {
	return(last_time.elapsed());
}

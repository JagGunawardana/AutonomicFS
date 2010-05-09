#include <QString>
#include <QTime>

#include "nameserver.h"

NameServer::NameServer(QString ip_address, int port_number) {
	address = ip_address;
	this->port_number = port_number;
	last_keep_alive_gap = 0;
	last_time.start();
}

int NameServer::KeepAliveMessage(void) {
	last_keep_alive_gap = last_time.elapsed();
	last_time.restart();
	return(last_keep_alive_gap);
}

int NameServer::GetKeepAliveGap(void) {
	return(last_time.elapsed());
}

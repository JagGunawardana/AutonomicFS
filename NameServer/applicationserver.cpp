#include <QTime>
#include <QString>
#include <QVariant>
#include <QDebug>

#include "applicationserver.h"

ApplicationServer::ApplicationServer(int new_pid, int new_port_number, QString new_server_name, QVariant new_server_type) {
	pid = new_pid;
	port_number = new_port_number;
	server_type = new_server_type;
	server_name = new_server_name;
	read_count = 0;
	write_count = 0;
	last_keep_alive_gap = 0;
	last_time.start();
	elapsed_time.start();
}

double ApplicationServer::GetReadRate(void) {
	if (elapsed_time.elapsed()==0)
		return(0);
	return((double)read_count/((double)elapsed_time.elapsed()/1000.0));
}

double ApplicationServer::GetWriteRate(void) {
	if (elapsed_time.elapsed()==0)
		return(0);
	return((double)write_count/((double)elapsed_time.elapsed()/1000.0));
}

double ApplicationServer::GetLoadRate(void) {
	return((GetReadRate()+GetWriteRate())/2);
}

int ApplicationServer::KeepAliveMessage(void) {
	last_keep_alive_gap = last_time.elapsed();
	last_time.restart();
	return(last_keep_alive_gap);
}

int ApplicationServer::GetKeepAliveGap(void) {
	return(last_time.elapsed());
}

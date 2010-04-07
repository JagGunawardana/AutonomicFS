#ifndef APPLICATIONSERVER_H
#define APPLICATIONSERVER_H

#include <QTime>
#include <QString>
#include <QVariant>

class ApplicationServer
{
private:
	int pid;
	int port_number;
	QVariant server_type;
	QString server_name;
	int keep_alive_gap;
	QTime last_time;
public:
	ApplicationServer(int pid, int port_number, QString server_name,
					  QVariant server_type);
	int KeepAliveMessage(void);
	int GetKeepAliveGap(void) {return(keep_alive_gap);}
	int GetPid(void) {return(pid);}
	int GetPortNumber(void) {return(port_number);}
	QString GetServerName(void) {return(server_name);}
};

#endif // APPLICATIONSERVER_H

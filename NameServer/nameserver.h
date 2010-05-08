#ifndef NAMESERVER_H
#define NAMESERVER_H

class QString;
class QTime;

class NameServer {
private:
	QString address;
	int port_number;
	int keep_alive_gap;
	QTime last_time;
public:
	NameServer(QString ip_address, int port_number);
	int KeepAliveMessage(void);
	int GetKeepAliveGap(void) {return(keep_alive_gap);}
	int GetPortNumber(void) {return(port_number);}
	QString GetAddress(void) {return(address);}
};

#endif // NAMESERVER_H

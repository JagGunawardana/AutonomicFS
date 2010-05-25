#ifndef NAMESERVER_H
#define NAMESERVER_H

class QString;
class QTime;

class NameServer {
private:
	QString address;
	int port_number;
	int last_keep_alive_gap;
	QTime last_time;
	double load;
public:
	NameServer(QString ip_address, int port_number);
	int KeepAliveMessage(void);
	int GetKeepAliveGap(void);
	int GetLastKeepAliveGap(void) {return(last_keep_alive_gap);}
	int GetPortNumber(void) {return(port_number);}
	QString GetAddress(void) {return(address);}
	void SetLoad(double new_load) {load=new_load;}
	double GetLoad(void) {return(load);}
};

#endif // NAMESERVER_H

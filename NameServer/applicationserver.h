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
	int last_keep_alive_gap;
	int read_count;
	int write_count;
	QTime last_time;
	QTime elapsed_time;
public:
	ApplicationServer(int pid, int port_number, QString server_name,
					  QVariant server_type);
	void SetReadCount(int new_count) {read_count=new_count;}
	void SetWriteCount(int new_count) {write_count=new_count;}
	double GetReadRate(void);
	double GetWriteRate(void);
	double GetLoadRate(void);
	int GetReadCount(void) {return(read_count);}
	int GetWriteCount(void) {return(write_count);}
	int KeepAliveMessage(void);
	int GetKeepAliveGap(void);
	int GetLastKeepAliveGap(void) {return(last_keep_alive_gap);}
	int GetPid(void) {return(pid);}
	int GetPortNumber(void) {return(port_number);}
	QString GetServerName(void) {return(server_name);}
};

#endif // APPLICATIONSERVER_H

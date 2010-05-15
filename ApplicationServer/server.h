#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QVariant>

#include "../RPC/xml_rpc/server.h"

class Server : public QObject {
	Q_OBJECT
public:
	Server(quint16 port, QString server_name, QObject* parent = 0);
	~Server();
	int GetPort(void) {return(port);}
	QString GetIPAddress(int requestId) {return(srv->GetIPAddress(requestId));}
private:
	xmlrpc::Server* srv;
	int port;
	QString server_name;
private slots:
	void processRequest( int requestId, QString methodName, QList<xmlrpc::Variant> parameters );
};

#endif // SERVER_H

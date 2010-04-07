#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QVariant>
#include <QMap>

#include "../RPC/xml_rpc/server.h"
class ApplicationServer;

class Server : public QObject {
	Q_OBJECT

public:
	Server( quint16 port, QObject *parent = 0 );
	~Server();
private:
	xmlrpc::Server* srv;
	QMap<int, ApplicationServer*> appserver_map;
	QVariant RegisterAppServer(QVariant server_name, QVariant pid, QVariant port_number, QVariant server_type); // Called by each application server when it starts
	QVariant Ping(QVariant pid); // Used for keep alive and latency checks

private slots:
	void processRequest( int requestId, QString methodName, QList<xmlrpc::Variant> parameters );
};

#endif // SERVER_H

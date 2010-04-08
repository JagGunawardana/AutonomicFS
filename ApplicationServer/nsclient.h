#ifndef NSCLIENT_H
#define NSCLIENT_H

#include <QObject>
#include <QVariant>

#include "../RPC/xml_rpc/client.h"

class NSClient : public QObject {
	Q_OBJECT
public:
	NSClient( const QString &address, QString server_name, QObject *parent = 0 );
	~NSClient();
	void RegisterWithNameServer(int application_port, QString server_type);
	void Ping(void);

private slots:
	void processFault( int requestId, int errorCode, QString errorString );
	void processReturnValue( int requestId, QVariant value );

private:
	QString address;
	quint16 port;
	QString server_name;

	xmlrpc::Client* client;
};

#endif // NSCLIENT_H

#ifndef SERVICEREQUEST_H
#define SERVICEREQUEST_H

#include <QRunnable>
#include "../RPC/xml_rpc/server.h"

class ServiceRequest : public QRunnable
{
private:
	xmlrpc::Server *srv;
	QList<xmlrpc::Variant> parameters;
	int requestId;

public:
	void run();
	ServiceRequest(xmlrpc::Server* srv, QList<xmlrpc::Variant> parameters, int requestId);
	QVariant Service_RequestFile(QVariant file_name);
};

#endif // SERVICEREQUEST_H

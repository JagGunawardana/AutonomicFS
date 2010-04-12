#ifndef SERVICEREQUEST_H
#define SERVICEREQUEST_H

#include <QRunnable>
#include "../RPC/xml_rpc/server.h"

class QMutex;
class QThread;

class ServiceRequest : public QRunnable
{
private:
	xmlrpc::Server *srv;
	QList<xmlrpc::Variant> parameters;
	int requestId;
	QMutex* sync_mutex[2];
	QThread* our_thread;
	QObject* dummy;
public:
	void run();
	QThread* GetOurThread(void) {return(our_thread);}
	ServiceRequest(xmlrpc::Server* srv, QList<xmlrpc::Variant> parameters, int requestId);
	~ServiceRequest();
	QVariant Service_RequestFile(QVariant file_name);
};

#endif // SERVICEREQUEST_H

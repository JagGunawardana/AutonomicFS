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
	QSemaphore sync_sem;
	QThread* our_thread;
	QThread* master_thread;
	QObject* socket_parent;
public:
	void run();
	enum RequestType {request_file} our_request;
	QThread* GetOurThread(void) {return(our_thread);}
	ServiceRequest(xmlrpc::Server* srv,
				   QList<xmlrpc::Variant> parameters,
				   int requestId,
				   ServiceRequest::RequestType request,
				   QList<int> active_ports);
	~ServiceRequest();
	QVariant Service_RequestFile(QVariant file_name);
	void TransferSocket(void);
	void TransferBackSocket(QTcpSocket* socket);
};

#endif // SERVICEREQUEST_H

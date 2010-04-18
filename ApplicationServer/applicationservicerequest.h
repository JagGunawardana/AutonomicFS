#ifndef APPLICATIONSERVICEREQUEST_H
#define APPLICATIONSERVICEREQUEST_H

#include <QRunnable>
#include "../RPC/xml_rpc/server.h"

class ApplicationServiceRequest : public QRunnable
{	
public:
	enum RequestType {request_FileByName, request_FileByHash} our_request;
	ApplicationServiceRequest(xmlrpc::Server* srv,
				   QList<xmlrpc::Variant> parameters,
				   int requestId,
				   ApplicationServiceRequest::RequestType request);
	~ApplicationServiceRequest();
	void run();
	QThread* GetOurThread(void) {return(our_thread);}
	QVariant Service_RequestFileByName(QVariant file_name);
	void TransferSocket(void);
	void TransferBackSocket(QTcpSocket* socket);
private:
	xmlrpc::Server *srv;
	QList<xmlrpc::Variant> parameters;
	int requestId;
	QSemaphore sync_sem;
	QThread* our_thread;
	QThread* master_thread;
	QObject* socket_parent;
};

#endif // APPLICATIONSERVICEREQUEST_H

#ifndef APPLICATIONSERVICEREQUEST_H
#define APPLICATIONSERVICEREQUEST_H

#include <QRunnable>
#include "../RPC/xml_rpc/server.h"

class ApplicationServiceRequest : public QRunnable
{	
public:
	enum RequestType {request_FileByName, request_FileByHash, request_AllFilesList} our_request;
	ApplicationServiceRequest(xmlrpc::Server* srv,
				   QList<xmlrpc::Variant> parameters,
				   int requestId,
				   ApplicationServiceRequest::RequestType request);
	~ApplicationServiceRequest();
	void run();
	QThread* GetOurThread(void) {return(our_thread);}
	void TransferSocket(void);
	void TransferBackSocket(QTcpSocket* socket);
protected:
	QVariant Service_RequestFileByName(QVariant file_name);
	QList<QMap<QString, QString> > Service_GetAllFilesList(void);
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

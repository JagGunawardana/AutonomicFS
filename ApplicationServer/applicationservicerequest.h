#ifndef APPLICATIONSERVICEREQUEST_H
#define APPLICATIONSERVICEREQUEST_H

#include <QRunnable>
#include "../RPC/xml_rpc/server.h"

class Server;

class ApplicationServiceRequest : public QRunnable
{	
public:
	enum RequestType {request_FileByName, request_FileByHash, request_AllFilesList, request_SaveFile} our_request;
	ApplicationServiceRequest(Server* server,
					xmlrpc::Server* srv,
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
	QVariant Service_RequestFileByHash(QVariant hash);
	QVariant Service_SaveFile(QVariant file_name, QVariant file_content);
	QList<QList<QString> > Service_GetAllFilesList(QVariant IPAddress);
private:
	xmlrpc::Server *srv;
	QList<xmlrpc::Variant> parameters;
	int requestId;
	QSemaphore sync_sem;
	QThread* our_thread;
	QThread* master_thread;
	QObject* socket_parent;
	Server* server;
};

#endif // APPLICATIONSERVICEREQUEST_H

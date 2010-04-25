#ifndef SERVICEREQUEST_H
#define SERVICEREQUEST_H

#include <QRunnable>

#include "../RPC/xml_rpc/client.h"
#include "server.h"
class QMutex;
class QThread;

class ServiceRequest : public QObject, public QRunnable {
	Q_OBJECT
private:
	xmlrpc::Server *srv;
	QList<xmlrpc::Variant> parameters;
	int requestId;
	QSemaphore sync_sem;
	QThread* our_thread;
	QThread* master_thread;
	QObject* socket_parent;
	Server* server;
	xmlrpc::Client* client;
	QVariant script_ret_val;
protected:
	QVariant Service_RequestFile(QVariant file_name);
	QVariant  Service_GetAllFilesUnderMgt(void);
	QEventLoop event_loop;
public:
	void run();
	enum RequestType {request_file, request_filesundermgt} our_request;
	QThread* GetOurThread(void) {return(our_thread);}
	ServiceRequest(xmlrpc::Server* srv,
				   Server* server,
				   QList<xmlrpc::Variant> parameters,
				   int requestId,
				   ServiceRequest::RequestType request);
	~ServiceRequest();
	void TransferSocket(void);
	void TransferBackSocket(QTcpSocket* socket);
	QList<xmlrpc::Variant> ConvertToMapList(QVariant var_in);
protected slots:
	void processReturnValue(int requestId, QVariant value);
	void processFault(int requestId, int errorCode, QString errorString);

};

#endif // SERVICEREQUEST_H

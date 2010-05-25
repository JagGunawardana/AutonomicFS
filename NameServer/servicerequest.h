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
	QMutex* single_mutex;
protected:
	QVariant Client_RequestFileByName(QVariant file_name);
	QVariant Client_RequestFileByHash(QVariant hash);
	QVariant Service_RequestFileByName(QVariant file_name);
	QVariant Service_RequestFileByHash(QVariant hash);
	QVariant Service_GetAllLocalFilesUnderMgt(void);
	QVariant Service_GetAllFilesUnderMgt(void);
	QVariant Service_DeleteLocalFile(QVariant file_name);
	QVariant Client_DeleteFile(QVariant file_name);
	QVariant Client_SaveFile(QVariant file_name, QVariant file_content);
	QVariant Service_SaveLocalFile(QVariant file_name, QVariant file_content, QVariant force_new);
	void Service_PeriodicProcesses(void);
	QEventLoop event_loop;
	QList<xmlrpc::Variant> ConvertToListOfVariants(QVariant var_in);
	QList<xmlrpc::Variant> ConvertToList(QVariant var_in);
public:
	void run();
	enum RequestType {request_file_byname, request_file_byhash, request_local_file_byname, request_local_file_byhash,
						request_localfilesundermgt, request_allfilesundermgt,
						request_savefile, request_savelocalfile, request_periodicprocesses,
						request_deletefile, request_deletelocalfile} our_request;
	QThread* GetOurThread(void) {return(our_thread);}
	ServiceRequest(xmlrpc::Server* srv,
				   Server* server,
				   QList<xmlrpc::Variant> parameters,
				   int requestId,
				   ServiceRequest::RequestType request);
	~ServiceRequest();
	void TransferSocket(void);
	void DummyTransferSocket(void);
	void TransferBackSocket(QTcpSocket* socket);
	void SetSingleMutex(QMutex* mutex) {single_mutex=mutex;}
protected slots:
	void processReturnValue(int requestId, QVariant value);
	void processFault(int requestId, int errorCode, QString errorString);

};

#endif // SERVICEREQUEST_H

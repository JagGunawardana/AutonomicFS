#include "applicationservicerequest.h"

#include <QSemaphore>
#include <QTcpSocket>

#include "filemanager.h"
#include "../SharedServices/logger.h"

#include "../RPC/xml_rpc/variant.h"

ApplicationServiceRequest::ApplicationServiceRequest(xmlrpc::Server* srv,
							   QList<xmlrpc::Variant> parameters,
							   int requestId,
							   ApplicationServiceRequest::RequestType request_type) : sync_sem(0)
{
	this->srv = srv;
	this->parameters = parameters;
	this->requestId = requestId;
	master_thread = QThread::currentThread(); // used to switch back socket ownership at end
	socket_parent = srv->GetSocketParent(requestId);
	our_request = request_type;
}

ApplicationServiceRequest::~ApplicationServiceRequest() {
}

void ApplicationServiceRequest::TransferSocket(void) {
	// do the transfer of the socket - this is called by the master thread
	sync_sem.acquire(10); // wait until the other thread has sent it's QThread
	srv->SetSocketParent(NULL, requestId);
	srv->TransferSocketThread(our_thread, requestId);
	sync_sem.release(20); // allow the other thread to go and do it's thing
}

void ApplicationServiceRequest::TransferBackSocket(QTcpSocket* socket) {
	// Clean up socket, we're resposible for it's deletion
	delete(socket);
}

void ApplicationServiceRequest::run(void) {
	// Thread house keeping
	our_thread = QThread::currentThread();
	sync_sem.release(10);
	sync_sem.acquire(20);
	// Do our action
	QTcpSocket* socket = NULL;
	qDebug()<<"Received request";
	if (our_request == request_FileByName) {
		QVariant ret_val = Service_RequestFileByName(parameters[0]);
		xmlrpc::Variant v1 = ret_val.toList()[0].toBool();
		xmlrpc::Variant v2 = ret_val.toList()[1].toByteArray();
		QList<xmlrpc::Variant> temp_list;
		temp_list.append(v1);
		temp_list.append(v2);
		socket = srv->sendReturnValue( requestId, temp_list);
	}
	// Clean up threads and socket ownership
	sync_sem.release(20); // clean up sempahore
	if (socket!=NULL)
		TransferBackSocket(socket);
}

QVariant ApplicationServiceRequest::Service_RequestFileByName(QVariant file_name) {
		qDebug()<<"Getting file by name";
	Logger("Application server",
		   "../NameServer/server_log").WriteLogLine(QString("Service"),
													QString("Received request (FileByName) file (%1)....").arg(file_name.toString()));
	FileManager* fm = FileManager::GetFileManager();
	return(fm->CheckServeFileByName(file_name.toString()));
}

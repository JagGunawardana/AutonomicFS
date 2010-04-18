#include "applicationservicerequest.h"

#include <QSemaphore>
#include <QTcpSocket>

#include "filemanager.h"

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
	if (our_request == request_FileByName) {
		QVariant ret_val = Service_RequestFileByName(parameters[0]);
		socket = srv->sendReturnValue( requestId, ret_val.toByteArray());
	}
	// Clean up threads and socket ownership
	sync_sem.release(20); // clean up sempahore
	if (socket!=NULL)
		TransferBackSocket(socket);
}

QVariant ApplicationServiceRequest::Service_RequestFileByName(QVariant file_name) {
	FileManager* fm = FileManager::GetFileManager();
	return(fm->CheckServeFileByName(file_name.toString()));
}

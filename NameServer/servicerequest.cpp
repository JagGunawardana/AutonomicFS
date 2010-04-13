#include "servicerequest.h"

#include <QSemaphore>
#include <QTcpSocket>

ServiceRequest::ServiceRequest(xmlrpc::Server* srv,
							   QList<xmlrpc::Variant> parameters,
							   int requestId,
							   ServiceRequest::RequestType request_type) : sync_sem(0)
{
	this->srv = srv;
	this->parameters = parameters;
	this->requestId = requestId;
	master_thread = QThread::currentThread(); // used to switch back socket ownership at end
	socket_parent = srv->GetSocketParent(requestId);
	our_request = request_type;
}

ServiceRequest::~ServiceRequest() {
}

void ServiceRequest::TransferSocket(void) {
	// do the transfer of the socket - this is called by the master thread
	sync_sem.acquire(10); // wait until the other thread has sent it's QThread
	srv->SetSocketParent(NULL, requestId);
	srv->TransferSocketThread(our_thread, requestId);
	sync_sem.release(20); // allow the other thread to go and do it's thing
}

void ServiceRequest::TransferBackSocket(QTcpSocket* socket) {
	// Clean up socket, we're resposible for it's deletion
	delete(socket);
}

void ServiceRequest::run(void) {
	// Thread house keeping
	our_thread = QThread::currentThread();
	sync_sem.release(10);
	sync_sem.acquire(20);
	// Do our action
	QTcpSocket* socket = NULL;
	if (our_request == request_file) {
		QVariant ret_val = Service_RequestFile(parameters[0]);
		socket = srv->sendReturnValue( requestId, ret_val.toByteArray());
	}
	// Clean up threads and socket ownership
	sync_sem.release(20); // clean up sempahore
	if (socket!=NULL)
		TransferBackSocket(socket);
}

QVariant ServiceRequest::Service_RequestFile(QVariant file_name) {
	QFile file(file_name.toString());
	if (!file.exists()) {
		qFatal("Cant find Nameserver.");
	}
	file.open(QIODevice::ReadOnly);
	QByteArray stream(file.size(), '\0');
	stream = file.readAll();
	return (stream);
}

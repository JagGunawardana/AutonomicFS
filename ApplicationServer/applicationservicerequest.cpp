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
	if (our_request == request_FileByName) {
		QVariant ret_val = Service_RequestFileByName(parameters[0]);
		QMap<QString, xmlrpc::Variant> tmp_var;
		tmp_var["ret_val"]=ret_val.toList()[0].toBool();
		tmp_var["file"]=ret_val.toList()[1].toByteArray();
		socket = srv->sendReturnValue( requestId, tmp_var);
// We need to process events to send
	}
	else if (our_request == request_AllFilesList) {
		QVariant ret_val = Service_RequestFileByName(parameters[0]);
		QMap<QString, xmlrpc::Variant> tmp_var;
		tmp_var["ret_val"]=ret_val.toList()[0].toBool();
		tmp_var["file"]=ret_val.toList()[1].toByteArray();
		socket = srv->sendReturnValue( requestId, tmp_var);

	}
	// Next line is required to cleanly send
	QEventLoop().processEvents(QEventLoop::AllEvents, 200000);
	// Clean up threads and socket ownership
	sync_sem.release(20); // clean up sempahore
	if (socket!=NULL)
		TransferBackSocket(socket);
}

QVariant ApplicationServiceRequest::Service_RequestFileByName(QVariant file_name) {
	Logger("Application server",
		   "../NameServer/server_log").WriteLogLine(QString("Service"),
													QString("Received request (FileByName) for file (%1)....").arg(file_name.toString()));
	FileManager* fm = FileManager::GetFileManager();
	return(fm->CheckServeFileByName(file_name.toString()));
}

QList<QMap<QString, QString> > ApplicationServiceRequest::GetAllFilesList(void) {
	Logger("Application server",
		   "../NameServer/server_log").WriteLogLine(QString("Service"),
													QString("Received request (GetAllFilesList)...."));
	FileManager* fm = FileManager::GetFileManager();
	return(fm->GetAllFilesList());
}


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
		Q_ASSERT(ret_val.canConvert(QVariant::List));
		QVariantList tmp_lst = ret_val.toList();
		QList<xmlrpc::Variant> tmp_var;
		tmp_var.append(xmlrpc::Variant(tmp_lst[0].toBool()));
		tmp_var.append(xmlrpc::Variant(tmp_lst[1].toString()));
		tmp_var.append(xmlrpc::Variant(tmp_lst[2].toString()));
		QByteArray new_ba(tmp_lst[3].toByteArray().toBase64());
		tmp_var.append(xmlrpc::Variant(new_ba));
		socket = srv->sendReturnValue( requestId, xmlrpc::Variant(tmp_var));
	}
	else if (our_request == request_FileByHash) {
		QVariant ret_val = Service_RequestFileByHash(parameters[0]);
		Q_ASSERT(ret_val.canConvert(QVariant::List));
		QVariantList tmp_lst = ret_val.toList();
		QList<xmlrpc::Variant> tmp_var;
		tmp_var.append(xmlrpc::Variant(tmp_lst[0].toBool()));
		tmp_var.append(xmlrpc::Variant(tmp_lst[1].toString()));
		tmp_var.append(xmlrpc::Variant(tmp_lst[2].toString()));
		QByteArray new_ba(tmp_lst[3].toByteArray().toBase64());
		tmp_var.append(xmlrpc::Variant(new_ba));
		tmp_var.append(xmlrpc::Variant(new_ba));
		socket = srv->sendReturnValue( requestId, tmp_var);
	}
	else if (our_request == request_AllFilesList) {
		QList<QList<QString> >  all_files = Service_GetAllFilesList();
		QList<xmlrpc::Variant> ret_val;
		for (int i = 0; i < all_files.size(); ++i) {
			xmlrpc::Variant tmp_var(all_files.at(i));
			ret_val.append(tmp_var);
		}
		socket = srv->sendReturnValue( requestId, ret_val);
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

QVariant ApplicationServiceRequest::Service_RequestFileByHash(QVariant hash) {
	Logger("Application server",
		   "../NameServer/server_log").WriteLogLine(QString("Service"),
													QString("Received request (FileByHash) for file hash (%1)....").arg(hash.toString()));
	FileManager* fm = FileManager::GetFileManager();
	return(fm->CheckServeFileByHash(hash.toString()));
}

QList<QList<QString> > ApplicationServiceRequest::Service_GetAllFilesList(void) {
	Logger("Application server",
		   "../NameServer/server_log").WriteLogLine(QString("Service"),
													QString("Received request (GetAllFilesList)...."));
	FileManager* fm = FileManager::GetFileManager();
	return(fm->GetAllFilesList());
}


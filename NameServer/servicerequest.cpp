#include "servicerequest.h"

#include <QSemaphore>
#include <QTcpSocket>
#include <QTimer>

#include "../SharedServices/profilemgr.h"
#include "nsscriptrunner.h"

#include "../RPC/xml_rpc/client.h"
#include "../RPC/xml_rpc/variant.h"
#include "../SharedServices/logger.h"

ServiceRequest::ServiceRequest(xmlrpc::Server* srv,
							   Server* server,
							   QList<xmlrpc::Variant> parameters,
							   int requestId,
							   ServiceRequest::RequestType request_type) : QObject(server), QRunnable(), sync_sem(0), event_loop()
{
	this->srv = srv;
	this->parameters = parameters;
	this->requestId = requestId;
	this->server = server;
	master_thread = QThread::currentThread(); // used to switch back socket ownership at end
	socket_parent = srv->GetSocketParent(requestId);
	our_request = request_type;
	client = NULL;
}

ServiceRequest::~ServiceRequest() {
}

void ServiceRequest::TransferSocket(void) {
	// do the transfer of the socket - this is called by the master thread
	if (client != NULL)
		delete(client);
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
// Should we enter event loop here !!! ?
	}
	else if (our_request == request_filesundermgt) {
		QVariant ret_val = Service_GetAllFilesUnderMgt();
		QList<xmlrpc::Variant> tmp_var = ConvertToListOfVariants(ret_val);
		socket = srv->sendReturnValue(requestId, tmp_var);
	}
	// Clean up threads and socket ownership
	sync_sem.release(20); // clean up sempahore
	if (socket!=NULL)
		TransferBackSocket(socket);
}

QList<xmlrpc::Variant> ServiceRequest::ConvertToListOfVariants(QVariant var_in) {
	// Converts an array of arrays to a list of variants
	Q_ASSERT(var_in.canConvert(QVariant::List));
	QList<QVariant> in_list = var_in.toList();
	QList<xmlrpc::Variant> out_list;
	for(int i=0;i<in_list.size();++i) {
		QList<xmlrpc::Variant> tmp_list;
		Q_ASSERT(in_list.at(i).canConvert(QVariant::List));
		QList<QVariant> next_list = in_list.at(i).toList();
		for(int j=0;j<next_list.size();j++)
			tmp_list.append(next_list.at(j).toString());
		out_list.append(xmlrpc::Variant(tmp_list));
	}
	return(out_list);
}

void ServiceRequest::processReturnValue( int requestId, QVariant value ) {
	Q_UNUSED(requestId)
	script_ret_val = value;
	event_loop.processEvents(QEventLoop::AllEvents);
	event_loop.exit();
}

void ServiceRequest::processFault( int requestId, int errorCode, QString errorString ) {
	Q_UNUSED( requestId );
	Logger("Name Server Service Request",
		   "server_log").WriteLogLine(QString("Error"),
			QString("Error in communication with application server: Code(%1), String(%2).").arg(errorCode).arg(errorString));
	// !!! how to return error here
	event_loop.exit();
}

QVariant ServiceRequest::Service_RequestFile(QVariant file_name) {
	// Find the file and return it back
	QByteArray file;
	ProfileMgr* pro = ProfileMgr::GetProfileManager(QDir("scripts").absolutePath());
	QMap<QString, QVariant> params;
	params[QString("file_name")] = file_name;
	NSScriptRunner script(pro->GetRelativeScriptPath("read_file"), server, params);
	script.GetResult(file);
	return(file);
}

QVariant ServiceRequest::Service_GetAllFilesUnderMgt(void) {
	// Get the list of files from all servers
	ProfileMgr* pro = ProfileMgr::GetProfileManager(QDir("scripts").absolutePath());
	QMap<QString, QVariant> params;
	NSScriptRunner script(pro->GetRelativeScriptPath("get_all_files"), server, params);
	QVariant variant;
	script.GetResult(variant); // !!!
	return(variant);
}

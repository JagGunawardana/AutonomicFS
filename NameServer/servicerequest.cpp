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
	if (srv!=NULL)
		socket_parent = srv->GetSocketParent(requestId);
	else
		socket_parent = NULL;
	our_request = request_type;
	client = NULL;
	single_mutex = NULL;
}

ServiceRequest::~ServiceRequest() {
}

void ServiceRequest::DummyTransferSocket(void) {
	// use for periodic processes (or whenever there is no client call)
	if (client != NULL)
		delete(client);
	sync_sem.acquire(10); // wait until the other thread has sent it's QThread
	sync_sem.release(20); // allow the other thread to go and do it's thing
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
	// Clean up socket, we're resposible for it's deletion - don't bother with anything else as it's not used again
	delete(socket);
}

void ServiceRequest::run(void) {
	// Thread house keeping
	our_thread = QThread::currentThread();
	sync_sem.release(10);
	sync_sem.acquire(20);
	// If there is a mutex to control access to this function then check it to see if we can proceed
	if (single_mutex != NULL) {
		if (!single_mutex->tryLock()) { // If we can't run then abort
			sync_sem.release(20); // clean up sempahore
			return;
		}
	}
	// Do our action
	QTcpSocket* socket = NULL;
	if (our_request == request_local_file_byname) {
		QVariant ret_val = Service_RequestFileByName(parameters[0]);
		QList<xmlrpc::Variant> tmp_var = ConvertToList(ret_val);
		socket = srv->sendReturnValue(requestId, xmlrpc::Variant(tmp_var));
	}
	else if (our_request == request_local_file_byhash) {
		QVariant ret_val = Service_RequestFileByHash(parameters[0]);
		QList<xmlrpc::Variant> tmp_var = ConvertToList(ret_val);
		socket = srv->sendReturnValue(requestId, xmlrpc::Variant(tmp_var));
	}
	else if (our_request == request_file_byname) {
		QVariant ret_val = Client_RequestFileByName(parameters[0]);
		QList<xmlrpc::Variant> tmp_var = ConvertToList(ret_val);
		socket = srv->sendReturnValue(requestId, xmlrpc::Variant(tmp_var));
	}
	else if (our_request == request_file_byhash) {
		QVariant ret_val = Client_RequestFileByHash(parameters[0]);
		QList<xmlrpc::Variant> tmp_var = ConvertToList(ret_val);
		socket = srv->sendReturnValue(requestId, xmlrpc::Variant(tmp_var));
	}
	else if (our_request == request_localfilesundermgt) {
		QVariant ret_val = Service_GetAllLocalFilesUnderMgt();
		QList<xmlrpc::Variant> tmp_var = ConvertToListOfVariants(ret_val);
		socket = srv->sendReturnValue(requestId, tmp_var);
	}
	else if (our_request == request_allfilesundermgt) {
		QVariant ret_val = Service_GetAllFilesUnderMgt();
		QList<xmlrpc::Variant> tmp_var = ConvertToListOfVariants(ret_val);
		socket = srv->sendReturnValue(requestId, tmp_var);
	}
	else if (our_request == request_savefile) {
qDebug()<<"&&&&&&&&&&&&&&&&&"<<parameters[1].toByteArray();
qDebug()<<"&&&&&&&&&&&&&&&&&"<<QByteArray::fromBase64(parameters[1].toByteArray());
		QVariant ret_val = Service_SaveFile(parameters[0], parameters[1]);
		xmlrpc::Variant tmp_var = ret_val.toBool();
		socket = srv->sendReturnValue(requestId, tmp_var);
	}
	else if (our_request == request_periodicprocesses) {
		Service_PeriodicProcesses();
		socket = NULL;
	}

	// Next line is required to cleanly send
	QEventLoop().processEvents(QEventLoop::AllEvents, 200000);
	// Clean up threads and socket ownership
	sync_sem.release(20); // clean up sempahore
	if (single_mutex != NULL) {
		single_mutex->unlock();
	}
	if (socket!=NULL)
		TransferBackSocket(socket);
}

QList<xmlrpc::Variant> ServiceRequest::ConvertToList(QVariant var_in) {
	// Converts an array of arrays to a list of variants, convenience function
	Q_ASSERT(var_in.canConvert(QVariant::List));
	QList<QVariant> in_list = var_in.toList();
	QList<xmlrpc::Variant> out_list;
	for(int i=0;i<in_list.size();++i) {
		QVariant variant = in_list.at(i);
		if (variant.type() ==  QVariant::Bool)
			out_list.append(xmlrpc::Variant(variant.toBool()));
		else if (variant.type() == QVariant::String)
			out_list.append(xmlrpc::Variant(variant.toString()));
		else if (variant.type() == QVariant::ByteArray) {
			QByteArray ba = variant.toByteArray();
//			out_list.append(xmlrpc::Variant(QByteArray::fromBase64(ba)));
			out_list.append(xmlrpc::Variant(ba));
		}
		else
			Q_ASSERT(FALSE);
	}
	return(out_list);
}

QList<xmlrpc::Variant> ServiceRequest::ConvertToListOfVariants(QVariant var_in) {
	// Converts an array of arrays to a list of variants, convenience function
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

QVariant ServiceRequest::Client_RequestFileByName(QVariant file_name) {
	// Find the file and return it back
	ProfileMgr* pro = ProfileMgr::GetProfileManager(QDir("scripts").absolutePath());
	QMap<QString, QVariant> params;
	params[QString("file_name")] = file_name;
	NSScriptRunner script(pro->GetRelativeScriptPath("read_file_byname"), server, params, requestId);
	QVariant variant;
	script.GetResult(variant);
	return(variant);
}

QVariant ServiceRequest::Client_RequestFileByHash(QVariant hash) {
	// Find the file and return it back
	ProfileMgr* pro = ProfileMgr::GetProfileManager(QDir("scripts").absolutePath());
	QMap<QString, QVariant> params;
	params[QString("file_hash")] = hash;
	NSScriptRunner script(pro->GetRelativeScriptPath("read_file_byhash"), server, params, requestId);
	QVariant variant;
	script.GetResult(variant);
	return(variant);
}

QVariant ServiceRequest::Service_RequestFileByName(QVariant file_name) {
	// Find the file and return it back
	ProfileMgr* pro = ProfileMgr::GetProfileManager(QDir("scripts").absolutePath());
	QMap<QString, QVariant> params;
	params[QString("file_name")] = file_name;
	NSScriptRunner script(pro->GetRelativeScriptPath("read_local_file_byname"), server, params, requestId);
	QVariant variant;
	script.GetResult(variant);
	return(variant);
}

QVariant ServiceRequest::Service_RequestFileByHash(QVariant hash) {
	// Find the file and return it back
	ProfileMgr* pro = ProfileMgr::GetProfileManager(QDir("scripts").absolutePath());
	QMap<QString, QVariant> params;
	params[QString("file_hash")] = hash;
	NSScriptRunner script(pro->GetRelativeScriptPath("read_local_file_byhash"), server, params, requestId);
	QVariant variant;
	script.GetResult(variant);
	return(variant);
}


QVariant ServiceRequest::Service_GetAllFilesUnderMgt(void) {
	// Get the list of files from all servers
	ProfileMgr* pro = ProfileMgr::GetProfileManager(QDir("scripts").absolutePath());
	QMap<QString, QVariant> params;
	NSScriptRunner script(pro->GetRelativeScriptPath("get_all_files"), server, params, requestId);
	QVariant variant;
	script.GetResult(variant); // !!!
	return(variant);
}

QVariant ServiceRequest::Service_GetAllLocalFilesUnderMgt(void) {
	// Get the list of files from local app servers
	ProfileMgr* pro = ProfileMgr::GetProfileManager(QDir("scripts").absolutePath());
	QMap<QString, QVariant> params;
	NSScriptRunner script(pro->GetRelativeScriptPath("get_all_local_files"), server, params, requestId);
	QVariant variant;
	script.GetResult(variant); // !!!
	return(variant);
}

QVariant ServiceRequest::Service_SaveFile(QVariant file_name, QVariant file_content) {
	// Get the list of files from local app servers
	ProfileMgr* pro = ProfileMgr::GetProfileManager(QDir("scripts").absolutePath());
	QMap<QString, QVariant> params;
	params[QString("file_name")] = file_name;
	params[QString("file_content")] = file_content;
qDebug()<<"******* Content: "<<file_content;
	NSScriptRunner script(pro->GetRelativeScriptPath("save_file"), server, params, requestId);
	QVariant variant;
	script.GetResult(variant);
	return(variant);
}

void ServiceRequest::Service_PeriodicProcesses(void) {
	// Carry out periodic processes
	ProfileMgr* pro = ProfileMgr::GetProfileManager(QDir("scripts").absolutePath());
	QMap<QString, QVariant> params;
	NSScriptRunner script(pro->GetRelativeScriptPath("periodic_processes"), server, params, requestId);
//	QVariant variant;
//	script.GetResult(variant);
}

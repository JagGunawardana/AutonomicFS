#include "servicerequest.h"

ServiceRequest::ServiceRequest(xmlrpc::Server* srv, QList<xmlrpc::Variant> parameters, int requestId )
{
	this->srv = srv;
	this->parameters = parameters;
	this->requestId = requestId;
}


void ServiceRequest::run() {
	QVariant ret_val = Service_RequestFile(parameters[0]);
	srv->sendReturnValue( requestId, ret_val.toByteArray());
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

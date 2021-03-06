
#include "server.h"
#include <QtCore/QCoreApplication>
#include <QtScript>
#include "../SharedServices/logger.h"
#include "nsscriptrunner.h"
#include "nsscripthelper.h"

NSScriptRunner::NSScriptRunner(QString script_file,
							   Server* server,
							   QMap<QString, QVariant> params, int requestId) {
	this->server = server;
	this->params = params;
	this->requestId = requestId;
	engine = new QScriptEngine();
	helper = new NSScriptHelper(engine, server, this->params, requestId);
	helper_value = new QScriptValue;
	*helper_value = engine->newQObject(helper);
	engine->globalObject().setProperty("helper", *helper_value);
	ret_val = NULL;
	script = "scripts/"+script_file;
	ReadFile();
	EvaluateScript();
}

NSScriptRunner::~NSScriptRunner() {
	delete(engine);
	delete(helper_value);
	if (ret_val!=NULL)
		delete(ret_val);
}

void NSScriptRunner::GetResult(int& int_val) {
	if (ret_val->isNumber())
		int_val = ret_val->toInteger();
	else
		int_val = 0; // !!! should handle better
}

void NSScriptRunner::GetResult(QString& str_val) {
	if (ret_val->isString())
		str_val = ret_val->toString();
	else
		str_val = "";
}

void NSScriptRunner::GetResult(QByteArray& byte_val) {
	byte_val = ret_val->toVariant().toByteArray();
}

void NSScriptRunner::GetResult(QVariant& variant) {
	variant = engine->fromScriptValue<QVariant>(*ret_val);
}

void NSScriptRunner::EvaluateScript(void) {
	Logger("Name Server", "server_log").WriteLogLine(QString("Script"),
		QString("Running script: file(%1).").arg(script));
	ret_val = new QScriptValue;
	*ret_val = engine->evaluate(script_contents);
}

void NSScriptRunner::ReadFile(void) {
	QFile file(script);
	if (!file.exists()) {
		qDebug("Script doesn't exist.");
		script_contents = "";
	}
	file.open(QIODevice::ReadOnly);
	QTextStream stream(&file);
	script_contents = stream.readAll();
	file.close();
}

#include <QtScript>
#include "scriptrunner.h"
#include "../SharedServices/logger.h"

ScriptRunner::ScriptRunner(QString script_file) {
	engine = new QScriptEngine();
	ret_val = NULL;
	script = "scripts/"+script_file;
	ReadFile();
	EvaluateScript();
}

ScriptRunner::~ScriptRunner() {
	delete(engine);
	if (ret_val!=NULL)
		delete(ret_val);
}

void ScriptRunner::GetResult(int& int_val) {
	if (ret_val->isNumber())
		int_val = ret_val->toInteger();
	else
		int_val = 0; // !!! should handle better
}

void ScriptRunner::GetResult(QString& str_val) {
	if (ret_val->isString())
		str_val = ret_val->toString();
	else
		str_val = "";
}

void ScriptRunner::EvaluateScript(void) {
//	qDebug()<<"Running: " <<endl << script_text;
	Logger("Application Server", "../NameServer/server_log").WriteLogLine(QString("Script"),
		QString("Running script: file(%1).").arg(script));
	ret_val = new QScriptValue;
	*ret_val = engine->evaluate(script_text);
//	qDebug() << "Return value "<<ret_val->toInteger();
}

void ScriptRunner::ReadFile(void) {
	QFile file(script);
	if (!file.exists()) {
		qFatal("Script doesn't exist.");
	}
	file.open(QIODevice::ReadOnly);
	script_text.clear();
	script_text = file.readAll();
	file.close();
}

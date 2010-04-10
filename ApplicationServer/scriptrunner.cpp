#include <QtScript>
#include "scriptrunner.h"
#include "../SharedServices/logger.h"
#include "scripthelper.h"

ScriptRunner::ScriptRunner(QString script_file) {
	engine = new QScriptEngine();
	helper = new ScriptHelper(engine);
	helper_value = new QScriptValue;
	*helper_value = engine->newQObject(helper);
	engine->globalObject().setProperty("helper", *helper_value);

	ret_val = NULL;
	script = "scripts/"+script_file;
	ReadFile();
	EvaluateScript();
}

ScriptRunner::~ScriptRunner() {
	delete(engine);
	delete(helper_value);
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
	Logger("Application Server", "../NameServer/server_log").WriteLogLine(QString("Script"),
		QString("Running script: file(%1).").arg(script));
	ret_val = new QScriptValue;
	*ret_val = engine->evaluate(script_contents);
}

void ScriptRunner::ReadFile(void) {
	QFile file(script);
	if (!file.exists()) {
		qFatal("Script doesn't exist.");
	}
	file.open(QIODevice::ReadOnly);
	QTextStream stream(&file);
	script_contents = stream.readAll();
	file.close();
}

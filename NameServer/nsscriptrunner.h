#ifndef NSSCRIPTRUNNER_H
#define NSSCRIPTRUNNER_H

class NSScriptHelper;
class QString;
class QScriptValue;
class QScriptEngine;
class Server;

class NSScriptRunner {
public:
	NSScriptRunner(QString script_file, Server* server, QMap<QString, QVariant> params);
	~NSScriptRunner();
	void GetResult(int& int_val);
	void GetResult(QString& str_val);
	void GetResult(QByteArray& byte_val);
	void GetResult(QVariant& variant);

protected:
	void ReadFile(void);
	void EvaluateScript(void);

private:
	QMap<QString, QVariant> params;
	QString script;
	QString script_contents;
	QScriptValue* ret_val;
	QScriptValue* helper_value;
	QScriptEngine* engine;
	NSScriptHelper* helper;
	Server* server;
};

#endif // NSSCRIPTRUNNER_H

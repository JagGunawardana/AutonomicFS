#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

class ScriptRunner {
public:
	ScriptRunner(QString script_file);
	~ScriptRunner();
	void GetResult(int& int_val);
	void GetResult(QString& str_val);

protected:
	void ReadFile(void);
	void EvaluateScript(void);

private:
	QString script;
	QByteArray script_text;
	QScriptValue* ret_val;
	QScriptEngine* engine;
};

#endif // SCRIPTRUNNER_H

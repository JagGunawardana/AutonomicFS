#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

class ScriptHelper;

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
	QString script_contents;
	QScriptValue* ret_val;
	QScriptValue* helper_value;
	QScriptEngine* engine;
	ScriptHelper* helper;
};

#endif // SCRIPTRUNNER_H

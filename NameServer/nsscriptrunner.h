#ifndef NSSCRIPTRUNNER_H
#define NSSCRIPTRUNNER_H

class NSScriptHelper;

class NSScriptRunner {
public:
	NSScriptRunner(QString script_file);
	~NSScriptRunner();
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
	NSScriptHelper* helper;
};

#endif // NSSCRIPTRUNNER_H

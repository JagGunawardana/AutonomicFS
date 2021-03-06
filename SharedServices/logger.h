#ifndef LOGGER_H
#define LOGGER_H

#include <QDebug>

class Logger {
private:
	QString file_name;
	QString app_source;
	bool IsIgnored(QString type);
public:
	Logger(QString source, QString file);
	void WriteLogLine(QString type, QString text);
};

#endif // LOGGER_H

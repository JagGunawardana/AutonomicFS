#include <QString>
#include <QDebug>
#include <QTime>
#include <QFile>
#include "logger.h"

Logger::Logger(QString source, QString file) {
	file_name = file;
	app_source = source;
}

void Logger::WriteLogLine(QString type, QString text) {
	QString line = app_source +":"+type+":"+ "("+QTime::currentTime().toString()+"): "+text;
	qDebug() << line;
	QFile our_file(file_name);
	our_file.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream out(&our_file);
	out << line << endl;
	our_file.flush();
	our_file.close();
}


#ifndef APPPERIODICPROCESS_H
#define APPPERIODICPROCESS_H

#include <QThread>
class QTimer;
class NSClient;

class AppPeriodicProcess : public QThread {
	Q_OBJECT
public:
	AppPeriodicProcess(QString our_server_name, QObject* parent = NULL);
	~AppPeriodicProcess();

protected:
	void run(void);
	bool TimeFor(QString event_name);
private:
	QTimer* periodic_timer;
	QString server_name;
	NSClient* ns_client;
	int tick_count;

private slots:
	void PeriodicProcesses(void);
	void SetupPeriodicProcesses(void);
};

#endif // APPPERIODICPROCESS_H

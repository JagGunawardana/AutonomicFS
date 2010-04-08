#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QThread>
#include <Soprano/Soprano>

class QDir;

class FileManager : public QThread  {
	Q_OBJECT
public:
	FileManager(QString server_name, QString file_store, int max_size = 0);
	~FileManager();
	int ScanFullFileStore(void);

protected:
	void run(void);
	QString GenerateHash(QString path_to_file);
private:
	QString server_name;
	QString file_store;
	int max_size;
	Soprano::Model* rdfmod;
	QDir* our_dir;
	int num_files;
	Soprano::Node predicate_hasname;
	Soprano::Node predicate_hassize;
	Soprano::Node predicate_hashash;

private slots:

};

#endif // FILEMANAGER_H

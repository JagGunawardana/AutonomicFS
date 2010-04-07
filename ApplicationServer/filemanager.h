#ifndef FILEMANAGER_H
#define FILEMANAGER_H

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
	void GenerateHash(void);
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

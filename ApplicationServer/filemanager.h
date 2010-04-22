#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QThread>
#include <Soprano/Soprano>

class QDir;

class FileManager : public QThread  {
	Q_OBJECT
public:
	~FileManager();
	int ScanFullFileStore(void);
	static FileManager* GetFileManager(QString server_name="", QString file_store="", int max_size = 0);
	// Our access functions
	QVariant CheckServeFileByName(QString file_name);
	QList<QMap<QString, QString> > GetAllFilesList(void);
protected:
	void run(void);
	QString GenerateHash(QString path_to_file);
	bool CheckFileInStoreByName(QString file_name);
private:
	FileManager(QString server_name, QString file_store, int max_size = 0);
	static FileManager* ptr_self;
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

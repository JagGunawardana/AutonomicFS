#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <Soprano/Soprano>

class QDir;
class QFile;

class FileManager : public QThread  {
	Q_OBJECT
public:
	~FileManager();
	int ScanFullFileStore(void);
	static FileManager* GetFileManager(QString server_name="", QString file_store="", int max_size = 0);
	// Our access functions
	QVariant CheckServeFileByName(QString file_name);
	QVariant CheckServeFileByHash(QString hash);
	QList<QList<QString> > GetAllFilesList(QString IPAddress);
	bool SaveFile(QString file_name, QByteArray file_content);
protected:
	void run(void);
	QString GenerateHash(QString path_to_file);
	bool CheckFileInStoreByName(QString file_name);
	QString GetFileHashFromName(QString file_name);
	QString GetFileNameFromHash(QString hash);
	int GetFileWriteCountFromName(QString file_name);
	int GetFileReadCountFromName(QString file_name);
	QDateTime GetFileDateStampFromName(QString file_name);
	int IncReadCount(QString file_name);
	int IncWriteCount(QString file_name);
private:
	FileManager(QString server_name, QString file_store, int max_size = 0);
	void ReScanFileByName(QFile& file, QString file_name);
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
	Soprano::Node predicate_hasreadcount;
	Soprano::Node predicate_haswritecount;
	Soprano::Node predicate_hasdatestamp;
	QMutex critical_section;
private slots:

};

#endif // FILEMANAGER_H

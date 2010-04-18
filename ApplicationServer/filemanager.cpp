#include "filemanager.h"

#include <QDir>
#include <QCryptographicHash>

#include "../SharedServices/logger.h"
#include <QDebug>


FileManager* FileManager::ptr_self = 0;

FileManager* FileManager::GetFileManager(QString server_name, QString file_store, int max_size) {
	if (!ptr_self) {
		if (server_name.length() == 0 || file_store.length() == 0)
			qFatal("File manager hasn't been initialised.");
		ptr_self = new FileManager(server_name, file_store, max_size);
	}
	return(ptr_self);
}

FileManager::FileManager(QString server_name,
						 QString file_store,
						 int max_size) :
predicate_hasname(QUrl("uri:predicate:has_name")),
predicate_hassize(QUrl("uri:predicate:has_size")),
predicate_hashash(QUrl("uri:predicate:has_hash"))
{
// Set up our parameters
	this->server_name = server_name;
	this->file_store = file_store;
	this->max_size = max_size;
// Setup our file store
	rdfmod = Soprano::createModel();
	file_store = file_store + (file_store.right(1)=="/" ? "" : "/"); // Add a trailing slash to the file store
	our_dir = new QDir(file_store);
	if (!our_dir->exists())
		qFatal("Fatal error, failed to find directory.");
}

FileManager::~FileManager() {
	delete(rdfmod);
	delete(our_dir);
}

void FileManager::run(void) {
// nothing special for now
	exec();
}

int FileManager::ScanFullFileStore(void) {
	// catalogue the full file store given to us
	num_files = 0;
	if (!our_dir->exists()) { // Make sure that the directory is still there - not sure what we'll do if is isn't?
			Logger("Application Server", "../NameServer/server_log").WriteLogLine(QString("Startup"),
						 QString("Scanning directory for content: server(%1), directory(%2).").arg(server_name).arg(file_store));
		qFatal("Failed to located directory ... aborting");
	}
	rdfmod->removeAllStatements(); // clear the model
	our_dir->setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
	our_dir->setSorting(QDir::Size | QDir::Reversed);
	QFileInfoList list = our_dir->entryInfoList();
	for (int i = 0; i < list.size(); ++i) {
		QFileInfo fileInfo = list.at(i);
		QString identifier = QString("uri:filestore:%1").arg(fileInfo.fileName());
		Logger("Application Server", "../NameServer/server_log").WriteLogLine(QString("Startup"),
					 QString("Cataloging file:file name(%3), server(%1), directory(%2).").arg(server_name).arg(file_store).arg(fileInfo.fileName()));
		rdfmod->addStatement(QUrl(identifier),
							 predicate_hasname,
							 Soprano::LiteralValue(fileInfo.fileName()));
		rdfmod->addStatement(QUrl(identifier),
							 predicate_hassize,
							 Soprano::LiteralValue(fileInfo.size()));
		rdfmod->addStatement(QUrl(identifier),
							 predicate_hashash,
							 Soprano::LiteralValue(GenerateHash(fileInfo.absoluteFilePath())));

	}
	Soprano::StatementIterator it = rdfmod->listStatements();
	while( it.next() )
		qDebug() << *it;
	return(num_files);
}

QString FileManager::GenerateHash(QString path_to_file) {
	// Generate the SHA1 hash of a file as a string.
	QCryptographicHash hash(QCryptographicHash::Sha1);
	QFile file(path_to_file);
	if (!file.open(QIODevice::ReadOnly)) {
		qDebug() << "Failed to open file "<<path_to_file;
	}
	QByteArray store(file.size()+1, '\0');
	file.read(store.data(), file.size()+1);
	hash.addData(store.constData());
	QByteArray sha1 = hash.result();
	QString ret = sha1.toHex();
	return(ret);
}

QVariant FileManager::CheckServeFileByName(QString file_name) {
	if (CheckFileInStoreByName(file_name)) {
		QFile file(our_dir->absoluteFilePath(file_name));
		if (!file.exists()) {
			Logger("Application Server", "../NameServer/server_log").WriteLogLine(QString("Service"),
						 QString("Error serving file (doesn't exist):file name(%3), server(%1), directory(%2).").arg(server_name).arg(file_store).arg(file_name));
			return(QVariant(""));
		}
		file.open(QIODevice::ReadOnly);
		QByteArray stream(file.size(), '\0');
		stream = file.readAll();
		return (stream);
	}
	else
		return(QVariant(""));
}

bool FileManager::CheckFileInStoreByName(QString file_name) {
	// Is this file in our file store, check rdf model
	Soprano::StatementIterator it = rdfmod->listStatements(Soprano::Node(),
														   predicate_hasname,
														   Soprano::LiteralValue(file_name));
	it.next();
	Soprano::Statement stmt = *it;
	if (stmt.isValid() && stmt.object()==Soprano::LiteralValue(file_name))
		return(true);
	else
		return(false);
}

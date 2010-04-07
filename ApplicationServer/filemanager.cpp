#include "filemanager.h"

#include <QDir>
#include "../SharedServices/logger.h"
#include <QDebug>

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
	file_store = file_store + (file_store.right(1)=="/" ? "" : "/");
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
	// !!! Should we clear our the model here ???
	our_dir->setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
	our_dir->setSorting(QDir::Size | QDir::Reversed);
	QFileInfoList list = our_dir->entryInfoList();
	for (int i = 0; i < list.size(); ++i) {
		QFileInfo fileInfo = list.at(i);
		QString identifier = QString("uri:filestore:%1").arg(fileInfo.fileName());
		rdfmod->addStatement(QUrl(identifier),
							 predicate_hasname,
							 Soprano::LiteralValue(fileInfo.fileName()));
		rdfmod->addStatement(QUrl(identifier),
							 predicate_hassize,
							 Soprano::LiteralValue(fileInfo.size()));

	}
	Soprano::StatementIterator it = rdfmod->listStatements();
	while( it.next() )
		qDebug() << *it;
	return(num_files);
}

void FileManager::GenerateHash(void) {


}

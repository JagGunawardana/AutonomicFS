#include "filemanager.h"

#include <QDir>
#include <QCryptographicHash>

#include "../SharedServices/logger.h"
#include <QDebug>
#include <QDateTime>

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
predicate_hashash(QUrl("uri:predicate:has_hash")),
predicate_hasreadcount(QUrl("uri:predicate:has_readcount")),
predicate_haswritecount(QUrl("uri:predicate:has_writecount")),
predicate_hasdatestamp(QUrl("uri:predicate:has_datestamp"))
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
	// Scan the file store directory and put info into RDF store
	critical_section.lock();
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
		rdfmod->addStatement(QUrl(identifier),
							 predicate_hasreadcount,
							 Soprano::LiteralValue(0));
		rdfmod->addStatement(QUrl(identifier),
							 predicate_haswritecount,
							 Soprano::LiteralValue(0));
		rdfmod->addStatement(QUrl(identifier),
							 predicate_hasdatestamp,
							 Soprano::LiteralValue(fileInfo.lastModified()));
		num_files++;
	}
	Soprano::StatementIterator it = rdfmod->listStatements();
	while( it.next() )
		qDebug() << *it;
	critical_section.unlock();
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

QVariant FileManager::CheckServeFileByHash(QString hash) {
	// return [true/false (have file), file_hash/"", file_name/"", file_content/""]
	QList<QVariant> ret_val;
	QString file_name = GetFileNameFromHash(hash);
	if (file_name==QString("")) { // not in our store
		ret_val.append(QVariant(false));
		ret_val.append(hash);
		ret_val.append("");
		ret_val.append("");
		return(ret_val);
	}
	else
		return(CheckServeFileByName(file_name));
}

QVariant FileManager::CheckServeFileByName(QString file_name) {
	// return [true/false (have file), file_hash/"", file_name/"", file_content/""]
	QList<QVariant> ret_val;
	if (CheckFileInStoreByName(file_name)) {
		QFile file(our_dir->absoluteFilePath(file_name));
		if (!file.exists()) { // This should never happen i.e. file does not exist but we this it does, but just in case.
			Logger("Application Server", "../NameServer/server_log").WriteLogLine(QString("Service"),
						 QString("Error serving file (doesn't exist):file name(%3), server(%1), directory(%2).").arg(server_name).arg(file_store).arg(file_name));
			ret_val.append(false);
			ret_val.append("");
			ret_val.append(file_name);
			ret_val.append("");

		}
		else { // We've got the file, lets serve it
			file.open(QIODevice::ReadOnly);
			QByteArray stream(file.size(), '\0');
			stream = file.readAll();
			ret_val.append(QVariant(true));
			ret_val.append(GetFileHashFromName(file_name));
			ret_val.append(file_name);
			IncReadCount(file_name); // Show we've read this file again
			ret_val.append(stream);
		}
	}
	else { // We don't have it in our store
		ret_val.append(QVariant(false));
		ret_val.append("");
		ret_val.append(file_name);
		ret_val.append("");
	}
	return(ret_val);
}

int FileManager::IncReadCount(QString file_name) {
	// Increment the read count for file with name file_name
	Soprano::StatementIterator it = rdfmod->listStatements(Soprano::Node(),
														   predicate_hasname,
														   Soprano::LiteralValue(file_name));
	if (!it.isValid()) // we don't have it so lets get out - should never happen
		return(0);
	it.next();
	Soprano::Statement stmt = *it;
	// Get the read count
	critical_section.lock();
	Soprano::StatementIterator it1 = rdfmod->listStatements(stmt.subject(),
															predicate_hasreadcount,
															Soprano::Node());
	if (!it1.isValid()) {// another should never happen
			critical_section.unlock();
			return(0);
	}
	it1.next();
	Soprano::Statement stmt1 = *it1;
	Q_ASSERT(stmt1.object().literal().isInt());
	int read_count = stmt1.object().literal().toInt();
	// Increment it
	read_count++;
	it.close(); // Need to close the iterators - else they will block access
	it1.close();
	rdfmod->removeStatement(stmt1);
	rdfmod->addStatement(stmt.subject(),
						 predicate_hasreadcount,
						 Soprano::LiteralValue(read_count));
	critical_section.unlock();
	return(read_count); // return the count
}

int FileManager::IncWriteCount(QString file_name) {
	// Increment the read count for file with name file_name
	Soprano::StatementIterator it = rdfmod->listStatements(Soprano::Node(),
														   predicate_hasname,
														   Soprano::LiteralValue(file_name));
	if (!it.isValid()) // we don't have it so lets get out - should never happen
		return(0);
	it.next();
	Soprano::Statement stmt = *it;
	// Get the read count
	critical_section.lock();
	Soprano::StatementIterator it1 = rdfmod->listStatements(stmt.subject(),
															predicate_haswritecount,
															Soprano::Node());
	if (!it1.isValid()) {// another should never happen
			critical_section.unlock();
			return(0);
	}
	it1.next();
	Soprano::Statement stmt1 = *it1;
	Q_ASSERT(stmt1.object().literal().isInt());
	int write_count = stmt1.object().literal().toInt();
	// Increment it
	write_count++;
	it.close(); // Need to close the iterators - else they will block access
	it1.close();
	rdfmod->removeStatement(stmt1);
	rdfmod->addStatement(stmt.subject(),
						 predicate_haswritecount,
						 Soprano::LiteralValue(write_count));
	critical_section.unlock();
	return(write_count); // return the count
}

bool FileManager::CheckFileInStoreByName(QString file_name) {
	// Is this file in our file store? check rdf model
	Soprano::StatementIterator it = rdfmod->listStatements(Soprano::Node(),
														   predicate_hasname,
														   Soprano::LiteralValue(file_name));
	if (!it.isValid()) {// we don't have it so lets get out
		return(false);
	}
	if (!it.next()) {
		return(false);
	}
	if (!it.isValid()) {// we don't have it so lets get out
		return(false);
	}
	Soprano::Statement stmt = *it;
	if (stmt.isValid() && stmt.object()==Soprano::LiteralValue(file_name))
		return(true);
	else
		return(false);
}

QString FileManager::GetFileHashFromName(QString file_name) {
	// Get the hash of content given the file name
	Soprano::StatementIterator it = rdfmod->listStatements(Soprano::Node(),
														   predicate_hasname,
														   Soprano::LiteralValue(file_name));
	if (!it.isValid()) // we don't have it so lets get out - should never happen
		return(QString(""));
	it.next();
	Soprano::Statement stmt = *it;
	// Get the hash now
	Soprano::StatementIterator it1 = rdfmod->listStatements(stmt.subject(),
															predicate_hashash,
															Soprano::Node());
	if (!it1.isValid()) // another should never happen
			return(QString(""));
	it1.next();
	Soprano::Statement stmt1 = *it1;
	return(stmt1.object().toString()); // return the file hash
}

QDateTime FileManager::GetFileDateStampFromName(QString file_name) {
	// Get the hash of content given the file name
	Soprano::StatementIterator it = rdfmod->listStatements(Soprano::Node(),
														   predicate_hasname,
														   Soprano::LiteralValue(file_name));
	if (!it.isValid()) // we don't have it so lets get out - should never happen
		return(QDateTime());
	it.next();
	Soprano::Statement stmt = *it;
	// Get the time now
	Soprano::StatementIterator it1 = rdfmod->listStatements(stmt.subject(),
															predicate_hasdatestamp,
															Soprano::Node());
	if (!it1.isValid()) // another should never happen
			return(QDateTime());
	it1.next();
	Soprano::Statement stmt1 = *it1;
	return(stmt1.object().literal().toDateTime()); // return the file hash
}

int FileManager::GetFileWriteCountFromName(QString file_name) {
	// Get the write count given the file name
	Soprano::StatementIterator it = rdfmod->listStatements(Soprano::Node(),
														   predicate_hasname,
														   Soprano::LiteralValue(file_name));
	if (!it.isValid()) // we don't have it so lets get out - should never happen
		return(0);
	it.next();
	Soprano::Statement stmt = *it;
	// Get the count now
	Soprano::StatementIterator it1 = rdfmod->listStatements(stmt.subject(),
															predicate_haswritecount,
															Soprano::Node());
	if (!it1.isValid()) // another should never happen
			return(0);
	it1.next();
	Soprano::Statement stmt1 = *it1;
	return(stmt1.object().literal().toInt()); // return the count
}

int FileManager::GetFileReadCountFromName(QString file_name) {
	// Get the read count given the file name
	Soprano::StatementIterator it = rdfmod->listStatements(Soprano::Node(),
														   predicate_hasname,
														   Soprano::LiteralValue(file_name));
	if (!it.isValid()) // we don't have it so lets get out - should never happen
		return(0);
	it.next();
	Soprano::Statement stmt = *it;
	// Get the count now
	Soprano::StatementIterator it1 = rdfmod->listStatements(stmt.subject(),
															predicate_hasreadcount,
															Soprano::Node());
	if (!it1.isValid()) // another should never happen
			return(0);
	it1.next();
	Soprano::Statement stmt1 = *it1;
	return(stmt1.object().literal().toInt()); // return the count
}

QString FileManager::GetFileNameFromHash(QString hash) {
	// Get the file name from the hash of content
	Soprano::StatementIterator it = rdfmod->listStatements(Soprano::Node(),
														   predicate_hashash,
														   Soprano::LiteralValue(hash));
	if (!it.isValid()) // we don't have it so lets get out
		return(QString(""));
	if (!it.next())  // we don't have it so lets get out
		return(QString(""));
	Soprano::Statement stmt = *it;
	// Get the name now
	Soprano::StatementIterator it1 = rdfmod->listStatements(stmt.subject(),
															predicate_hasname,
															Soprano::Node());
	if (!it1.isValid()) // should never happen
			return(QString(""));
	it1.next();
	Soprano::Statement stmt1 = *it1;
	return(stmt1.object().toString()); // return the file name
}

bool FileManager::SaveFile(QString file_name, QByteArray file_content, bool Force) {
	if (!Force) {
		if (!CheckFileInStoreByName(file_name))
			return(false);
	}
	QByteArray file_to_save = QByteArray::fromBase64(file_content);
	QFile file(our_dir->absoluteFilePath(file_name));
	if (Force) {
		file.open(QIODevice::WriteOnly);
		file.write(file_to_save);
		file.close();
		ScanNewFileByName(file, file_name);
	}
	else {
		if (!file.exists()) // shouldn't happen
				return(false);
		file.open(QIODevice::WriteOnly);
		file.write(file_to_save);
		file.close();
		IncWriteCount(file_name);
		// Deal with rest of record
		ReScanFileByName(file, file_name);
	}
	return(true);
}

void FileManager::GetLoad(int& read_load, int& write_load) {
	// Get the total read load and write load
	Soprano::StatementIterator it = rdfmod->listStatements(Soprano::Node(),
														   predicate_hasreadcount,
														   Soprano::Node());
	read_load = write_load = 0;
	while(it.next()) {
		Soprano::Statement stmt = *it;
		read_load += stmt.object().literal().toInt();
	}
	it.close();
	it = rdfmod->listStatements(Soprano::Node(),
		predicate_haswritecount,
		Soprano::Node());
	while(it.next()) {
		Soprano::Statement stmt = *it;
		write_load += stmt.object().literal().toInt();
	}
	it.close();
}

void FileManager::ScanNewFileByName(QFile& file, QString file_name) {
	QString identifier = QString("uri:filestore:%1").arg(file_name);
	QFileInfo fileInfo(file);
	rdfmod->addStatement(QUrl(identifier),
					 predicate_hasname,
					 Soprano::LiteralValue(fileInfo.fileName()));
	rdfmod->addStatement(QUrl(identifier),
					 predicate_hassize,
					 Soprano::LiteralValue(fileInfo.size()));
	rdfmod->addStatement(QUrl(identifier),
					 predicate_hashash,
					 Soprano::LiteralValue(GenerateHash(fileInfo.absoluteFilePath())));
	rdfmod->addStatement(QUrl(identifier),
					 predicate_hasreadcount,
					 Soprano::LiteralValue(0));
	rdfmod->addStatement(QUrl(identifier),
					 predicate_haswritecount,
					 Soprano::LiteralValue(0));
	rdfmod->addStatement(QUrl(identifier),
					 predicate_hasdatestamp,
					 Soprano::LiteralValue(fileInfo.lastModified()));
}

void FileManager::ReScanFileByName(QFile& file, QString file_name) {
	Soprano::StatementIterator it = rdfmod->listStatements(Soprano::Node(),
														   predicate_hasname,
														   Soprano::LiteralValue(file_name));
	it.next();
	Soprano::Statement stmt = *it;
	critical_section.lock();
	// Find hash
	Soprano::StatementIterator it1 = rdfmod->listStatements(stmt.subject(),
															predicate_hashash,
															Soprano::Node());
	it1.next();
	Soprano::Statement stmt1 = *it1;
	Q_ASSERT(stmt1.object().literal().isString());
	// Find size
	Soprano::StatementIterator it2 = rdfmod->listStatements(stmt.subject(),
															predicate_hassize,
															Soprano::Node());
	it2.next();
	Soprano::Statement stmt2 = *it2;
	Q_ASSERT(stmt2.object().literal().isInt());
	// Find time stamp
	Soprano::StatementIterator it3 = rdfmod->listStatements(stmt.subject(),
															predicate_hasdatestamp,
															Soprano::Node());
	it3.next();
	Soprano::Statement stmt3 = *it3;
	Q_ASSERT(stmt3.object().literal().isDateTime());
	// Now set correctly
	it.close(); // Need to close the iterators - else they will block access
	it1.close();
	it2.close();
	it3.close();
	rdfmod->removeStatement(stmt1);
	rdfmod->addStatement(stmt.subject(),
						 predicate_hashash,
						 Soprano::LiteralValue(GenerateHash(our_dir->absoluteFilePath(file_name))));
	rdfmod->removeStatement(stmt2);
	rdfmod->addStatement(stmt.subject(),
						 predicate_hassize,
						 Soprano::LiteralValue(file.size()));
	rdfmod->removeStatement(stmt3);
	QFileInfo fi(file);
	rdfmod->addStatement(stmt.subject(),
						 predicate_hasdatestamp,
						 Soprano::LiteralValue(fi.lastModified()));
	critical_section.unlock();
}

QVariant FileManager::DeleteFile(QString file_name) {
	if (!CheckFileInStoreByName(file_name))
		return(false);
	QFile file(our_dir->absoluteFilePath(file_name));
	Soprano::StatementIterator it = rdfmod->listStatements(Soprano::Node(),
														   predicate_hasname,
														   Soprano::LiteralValue(file_name));
	it.next();
	Soprano::Statement stmt = *it;
	Soprano::Node subject = stmt.subject();
	it.close();
	while(true) {
		it = rdfmod->listStatements(subject,
			Soprano::Node(),
			Soprano::Node());
		if (!it.next())
			break;
		Soprano::Statement statement = *it;
		it.close();
		rdfmod->removeStatement(statement);
	}
	return(QVariant(file.remove()));
}

QList<QList<QString> > FileManager::GetAllFilesList(QString IPAddress) {
	// Return a list of lists [[file_hash, file_name], ...]
	critical_section.lock();
	QList<QList<QString> > ret_val;
	Soprano::StatementIterator it = rdfmod->listStatements(Soprano::Node(),
														   predicate_hasname,
														   Soprano::Node());
	if (!it.isValid()) {// we don't have any files so lets get out
		critical_section.unlock();
		return(ret_val);
	}
	QString file_name;
	QString file_hash;
	QDateTime date_stamp;
	int read_count, write_count;
	while(it.next()) {
		Soprano::Statement stmt = *it;
		if (stmt.isValid())
			file_name = stmt.object().toString();
		else
			continue;
		file_hash = GetFileHashFromName(file_name);
		date_stamp = GetFileDateStampFromName(file_name);
		read_count = GetFileReadCountFromName(file_name);
		write_count = GetFileWriteCountFromName(file_name);
		QList<QString> tmp_list;
		tmp_list.append(file_hash);
		tmp_list.append(file_name);
		tmp_list.append(server_name);
		tmp_list.append(IPAddress);
		tmp_list.append(QString("%1").arg(date_stamp.toTime_t()));
		tmp_list.append(QString("%1").arg(read_count));
		tmp_list.append(QString("%1").arg(write_count));
		ret_val.append(tmp_list);
	}
	critical_section.unlock();
	return(ret_val);
}

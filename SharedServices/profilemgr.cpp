#include "profilemgr.h"
#include <QString>
#include <QDir>
#include <QTextStream>

#include "logger.h"

ProfileMgr* ProfileMgr::ptr_self = 0;

ProfileMgr* ProfileMgr::GetProfileManager(QString script_dir) {
	if (!ptr_self) {
		if (script_dir.length() == 0 || !QDir(script_dir).exists())
			qFatal("Profile manager hasn't been initialised.");
		ptr_self = new ProfileMgr(script_dir);
	}
	Q_ASSERT_X(script_dir==ptr_self->script_directory, "Singleton creation", "Incorrect script directory given.");
	return(ptr_self);
}

ProfileMgr::ProfileMgr(QString script_dir) {
	this->script_directory = script_dir;
	default_profile.clear();
	override_profile.clear();
	current_override = "";
	// Read in the default profile
	LoadProfile("default.profile", default_profile);
}

ProfileMgr::~ProfileMgr() {

}

void ProfileMgr::Override(QString profile_name) {
	current_override = profile_name;
	LoadProfile(profile_name, override_profile);
}

void ProfileMgr::LoadProfile(QString profile_name,
						QMap<QString, QString>& profile) {
	QDir sd(script_directory);
	profile.clear();
	QFile pfile(sd.absoluteFilePath(profile_name));
	Q_ASSERT_X(pfile.exists(), "File", "Profile doesn't exist.");
	Logger("Shared serices", "../NameServer/server_log").WriteLogLine(QString("Startup"),
				 QString("Loading profile %1.").arg(profile_name));
	pfile.open(QIODevice::ReadOnly);
	QString content = QTextStream(&pfile).readAll();
	pfile.close();
	QStringList lst = content.split("\n", QString::SkipEmptyParts);
	QStringList::const_iterator constIterator;
	for (constIterator = lst.constBegin(); constIterator != lst.constEnd(); ++constIterator) {
		QString line = constIterator->toLocal8Bit().constData();
		QStringList param = line.split("=", QString::SkipEmptyParts);
		profile[param[0]]=param[1];
//		qDebug()<<"Setting "<<param[0]<<"="<<param[1];
	}
}

QString ProfileMgr::GetFullScriptPath(QString verb) {
	if (override_profile.contains(verb)) {
		return(QDir(script_directory).absoluteFilePath(override_profile[verb]));
	}
	else if (default_profile.contains(verb)) {
		return(QDir(script_directory).absoluteFilePath(default_profile[verb]));
	}
	else {
		// We don't have this one.
		qFatal(QString("Can't run script for verb %1").arg(verb).toAscii());
	}
	return(QString(""));
}

QString ProfileMgr::GetRelativeScriptPath(QString verb) {
	QString ret_val("");
	if (override_profile.contains(verb)) {
		ret_val = override_profile[verb];
	}
	else if (default_profile.contains(verb)) {
		ret_val = default_profile[verb];
	}
	else {
		// We don't have this one.
		qFatal(QString("Can't run script for verb %1").arg(verb).toAscii());
	}
	return(ret_val);
}

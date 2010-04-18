#include "profilemgr.h"
#include <QString>
#include <QDir>

ProfileMgr* ProfileMgr::ptr_self = 0;

ProfileMgr::ProfileMgr(QString script_dir) {
	QDir sd(script_dir);
}

ProfileMgr* ProfileMgr::GetProfileManager(QString script_dir) {
	if (!ptr_self) {
		if (script_dir.length() == 0 || !QDir(script_dir).exists())
			qFatal("Profile manager hasn't been initialised.");
		ptr_self = new ProfileMgr(script_dir);
	}
	return(ptr_self);
}

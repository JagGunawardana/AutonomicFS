#ifndef PROFILEMGR_H
#define PROFILEMGR_H

#include <QString>

class ProfileMgr {
public:
	static ProfileMgr* GetProfileManager(QString script_dir);
private:
	QString script_directory;
	ProfileMgr(QString script_dir);
	static ProfileMgr* ptr_self;
};

#endif // PROFILEMGR_H

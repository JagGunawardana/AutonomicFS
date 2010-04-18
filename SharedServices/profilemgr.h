#ifndef PROFILEMGR_H
#define PROFILEMGR_H

#include <QString>
#include <QMap>

class ProfileMgr {
public:
	static ProfileMgr* GetProfileManager(QString script_dir);
	~ProfileMgr();
	QString GetFullScriptPath(QString verb);
	QString GetRelativeScriptPath(QString verb);
	void Override(QString profile_name);
private:
	ProfileMgr(QString script_dir);
	void LoadProfile(QString profile_name,
							QMap<QString, QString>& profle);
	QString script_directory;
	QString current_override;
	static ProfileMgr* ptr_self;
	QMap<QString, QString> default_profile;
	QMap<QString, QString> override_profile;

};

#endif // PROFILEMGR_H

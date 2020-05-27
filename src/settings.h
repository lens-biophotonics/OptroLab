#ifndef SETTINGS_H
#define SETTINGS_H

#include <QMap>
#include <QVariant>

#define SETTINGSGROUP_OTHERSETTINGS "OtherSettings"
#define SETTINGSGROUP_ACQUISITION "Acquisition"

#define SETTINGSGROUP_CAMTRIG "CameraTrigger"

#define SETTING_LUTPATH "LUTPath"

#define SETTING_PHYSCHANS "physicalChannels"

#define SETTING_TRIGGER_TERM "triggerTerm"

#define SETTING_EXPTIME "exposureTime"

#define SETTING_OUTPUTPATH "outputPath"

#define SETTING_SAMPRATE "samplingRate"

typedef QMap<QString, QVariant> SettingsMap;

class Settings
{
public:
    Settings();
    virtual ~Settings();

    QVariant value(const QString &group, const QString &key) const;
    void setValue(const QString &group, const QString &key, const QVariant val);

    void loadSettings();
    void saveSettings() const;

private:
    QMap<QString, SettingsMap*> map;
};

Settings& settings();

#endif // SETTINGS_H

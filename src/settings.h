#ifndef SETTINGS_H
#define SETTINGS_H

#include <QMap>
#include <QVariant>

#define SETTINGSGROUP_OTHERSETTINGS "OtherSettings"
#define SETTINGSGROUP_ACQUISITION "Acquisition"
#define SETTINGSGROUP_MAINTRIG "MainTrigger"
#define SETTINGSGROUP_ELREADOUT "ElectrodeReadout"
#define SETTINGSGROUP_SHUTTER "ShutterPulse"
#define SETTINGSGROUP_TIMING "Timing"

#define SETTINGSGROUP_CAMTRIG "CameraTrigger"

#define SETTING_LUTPATH "LUTPath"

#define SETTING_PHYSCHAN "physicalChannel"
#define SETTING_FREQ "frequency"
#define SETTING_TERM "terminal"
#define SETTING_DUTY "dutyCycle"
#define SETTING_STIMDURATION "stimDuration"

#define SETTING_INITIALDELAY "initialDelay"
#define SETTING_POSTSTIMULATION "postStimulation"

#define SETTING_TRIGGER_TERM "triggerTerm"

#define SETTING_EXPTIME "exposureTime"

#define SETTING_OUTPUTPATH "outputPath"

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

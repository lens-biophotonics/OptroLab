#ifndef SETTINGS_H
#define SETTINGS_H

#include <QMap>
#include <QVariant>

#define SETTINGSGROUP_OTHERSETTINGS "OtherSettings"
#define SETTINGSGROUP_ACQUISITION "Acquisition"
#define SETTINGSGROUP_MAINTRIG "MainTrigger"
#define SETTINGSGROUP_LED1 "LED1"
#define SETTINGSGROUP_LED2 "LED2"
#define SETTINGSGROUP_BEHAVCAMROI "BehavCamROI"
#define SETTINGSGROUP_ELREADOUT "ElectrodeReadout"
#define SETTINGSGROUP_STIMULATION "Stimulation"
#define SETTINGSGROUP_AUXSTIMULATION "AuxStimulation"
#define SETTINGSGROUP_TIMING "Timing"
#define SETTINGSGROUP_ZAXIS "zAxis"
#define SETTINGSGROUP_DDS "DDS"

#define SETTING_POS "pos"
#define SETTING_VELOCITY "velocity"
#define SETTING_STEPSIZE "stepsize"

#define SETTING_BAUD "baud"
#define SETTING_DEVICENUMBER "deviceNumber"
#define SETTING_SERIALNUMBER "serialNumber"
#define SETTING_PORTNAME "portName"

#define SETTING_LUTPATH "LUTPath"

#define SETTING_DEVNAME "devName"
#define SETTING_PHYSCHAN "physicalChannel"
#define SETTING_FREQ "frequency"
#define SETTING_LOW_TIME "lowTime"
#define SETTING_HIGH_TIME "highTime"
#define SETTING_NPULSES "nPulses"
#define SETTING_TERM "terminal"
#define SETTING_STIMDURATION "stimDuration"
#define SETTING_ENABLED "enabled"
#define SETTING_ALWAYS_ON "alwaysOn"
#define SETTING_AOD_ENABLED "aodEnabled"

#define SETTING_INITIALDELAY "initialDelay"
#define SETTING_POSTSTIMULATION "postStimulation"

#define SETTING_TRIGGER_TERM "triggerTerm"

#define SETTING_EXPTIME "exposureTime"

#define SETTING_MULTIRUN_ENABLED "multiRunEnabled"
#define SETTING_NRUNS "nRuns"

#define SETTING_OUTPUTPATH "outputPath"
#define SETTING_RUNNAME "runName"
#define SETTING_SAVEELECTRODE "saveElectrode"
#define SETTING_SAVEBEHAVIOR "saveBehavior"

#define SETTING_ROI "ROI"

typedef QMap<QString, QVariant> SettingsMap;

class Settings
{
public:
    Settings();
    virtual ~Settings();

    QVariant value(const QString &group, const QString &key) const;
    void setValue(const QString &group, const QString &key, const QVariant val);

    void loadSettings();
    void saveSettings();

private:
    QMap<QString, SettingsMap*> map;
};

Settings& settings();

#endif // SETTINGS_H

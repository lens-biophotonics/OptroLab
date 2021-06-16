#include <memory>

#include <QSettings>
#include <QDir>
#include <QRect>

#include "optrode.h"

#include "settings.h"

#define SET_VALUE(group, key, default_val) \
    setValue(group, key, settings.value(key, default_val))

Settings::Settings()
{
    loadSettings();
}

Settings::~Settings()
{
}

QVariant Settings::value(const QString &group, const QString &key) const
{
    if (!map.contains(group) || !map[group]->contains(key)) {
        return QVariant();
    }
    return map[group]->value(key);
}

void Settings::setValue(const QString &group, const QString &key,
                        const QVariant val)
{
    if (!map.contains(group)) {
        map.insert(group, new SettingsMap());
    }
    map.value(group)->insert(key, val);
}

void Settings::loadSettings()
{
    map.clear();

    QSettings settings;
    QString groupName;

    groupName = SETTINGSGROUP_OTHERSETTINGS;
    settings.beginGroup(groupName);

#ifdef WIN32
    SET_VALUE(groupName, SETTING_LUTPATH, "C:/Fiji.app/luts");
#else
    SET_VALUE(groupName, SETTING_LUTPATH, "/opt/Fiji.app/luts/");
#endif

    settings.endGroup();


    groupName = SETTINGSGROUP_MAINTRIG;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_PHYSCHAN, "Dev1/ctr0");
    SET_VALUE(groupName, SETTING_TERM, "/Dev1/PFI0");

    settings.endGroup();


    groupName = SETTINGSGROUP_LED1;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_PHYSCHAN, "Dev1/ctr1");
    SET_VALUE(groupName, SETTING_TERM, "/Dev1/PFI1");
    SET_VALUE(groupName, SETTING_FREQ, 45);
    SET_VALUE(groupName, SETTING_ENABLED, true);

    settings.endGroup();


    groupName = SETTINGSGROUP_LED2;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_PHYSCHAN, "Dev1/ctr2");
    SET_VALUE(groupName, SETTING_TERM, "/Dev1/PFI2");
    SET_VALUE(groupName, SETTING_ENABLED, true);

    settings.endGroup();

    groupName = SETTINGSGROUP_BEHAVCAMROI;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_ROI, QRect(0, 0, 1280, 1024));

    settings.endGroup();


    groupName = SETTINGSGROUP_ELREADOUT;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_PHYSCHAN, "Dev1/ai0");
    SET_VALUE(groupName, SETTING_TERM, "/Dev1/PFI0");
    SET_VALUE(groupName, SETTING_FREQ, 50);
    SET_VALUE(groupName, SETTING_ENABLED, true);

    settings.endGroup();


    groupName = SETTINGSGROUP_STIMULATION;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_PHYSCHAN, "Dev1/ctr3");
    SET_VALUE(groupName, SETTING_TERM, "/Dev1/PFI3");
    SET_VALUE(groupName, SETTING_FREQ, 50);
    SET_VALUE(groupName, SETTING_INITIALDELAY, 10);
    SET_VALUE(groupName, SETTING_LOW_TIME, .5);
    SET_VALUE(groupName, SETTING_HIGH_TIME, .5);
    SET_VALUE(groupName, SETTING_ENABLED, true);

    settings.endGroup();


    groupName = SETTINGSGROUP_ZAXIS;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_BAUD, 38400);
    SET_VALUE(groupName, SETTING_DEVICENUMBER, 1);
    SET_VALUE(groupName, SETTING_VELOCITY, 0.1);
    SET_VALUE(groupName, SETTING_POS, 0);
    SET_VALUE(groupName, SETTING_STEPSIZE, 0.1);

    settings.endGroup();


    groupName = SETTINGSGROUP_TIMING;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_INITIALDELAY, 10);
    SET_VALUE(groupName, SETTING_STIMDURATION, 10);
    SET_VALUE(groupName, SETTING_POSTSTIMULATION, 60);

    settings.endGroup();


    groupName = SETTINGSGROUP_ACQUISITION;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_EXPTIME, 0.15);
    SET_VALUE(groupName, SETTING_RUNNAME, "myexperiment");
    SET_VALUE(groupName, SETTING_SAVEELECTRODE, true);
    SET_VALUE(groupName, SETTING_SAVEBEHAVIOR, true);
    SET_VALUE(groupName, SETTING_OUTPUTPATH,
              QDir::toNativeSeparators(QDir::homePath()));

    settings.endGroup();
}

void Settings::saveSettings() const
{
    QSettings settings;

    QMapIterator<QString, SettingsMap*> groupIt(map);

    while (groupIt.hasNext()) {
        groupIt.next();
        const SettingsMap *map = groupIt.value();
        QMapIterator<QString, QVariant> it(*map);

        settings.beginGroup(groupIt.key());
        while (it.hasNext()) {
            it.next();
            settings.setValue(it.key(), it.value());
        }
        settings.endGroup();
    }
}

Settings &settings()
{
    static auto instance = std::make_unique<Settings>();
    return *instance;
}

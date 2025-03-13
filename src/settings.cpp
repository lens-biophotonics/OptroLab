#include <memory>

#include <QSettings>
#include <QDir>
#include <QRect>
#include <QSerialPortInfo>

#include "optrode.h"
#include "tasks.h"
#include "chameleoncamera.h"
#include "dds.h"

#include "settings.h"

#define SET_VALUE(group, key, default_val) \
    setValue(group, key, settings.value(key, default_val))

Settings::Settings()
{
    loadSettings();
}

Settings::~Settings()
{
    saveSettings();
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

    SET_VALUE(groupName, SETTING_TERM, "/Dev1/PFI0");

    settings.endGroup();


    groupName = SETTINGSGROUP_LED1;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_TERM, "/Dev1/PFI1");
    SET_VALUE(groupName, SETTING_FREQ, 45);
    SET_VALUE(groupName, SETTING_ENABLED, true);

    settings.endGroup();


    groupName = SETTINGSGROUP_LED2;
    settings.beginGroup(groupName);

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

    SET_VALUE(groupName, SETTING_TERM, "/Dev1/PFI3");
    SET_VALUE(groupName, SETTING_FREQ, 50);
    SET_VALUE(groupName, SETTING_INITIALDELAY, 10);
    SET_VALUE(groupName, SETTING_LOW_TIME, .5);
    SET_VALUE(groupName, SETTING_HIGH_TIME, .5);
    SET_VALUE(groupName, SETTING_ENABLED, true);
    SET_VALUE(groupName, SETTING_ALWAYS_ON, false);
    SET_VALUE(groupName, SETTING_AOD_ENABLED, false);

    settings.endGroup();


    groupName = SETTINGSGROUP_AUXSTIMULATION;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_ENABLED, false);
    SET_VALUE(groupName, SETTING_TERM, "/Dev1/PFI4");
    SET_VALUE(groupName, SETTING_INITIALDELAY, 10);
    SET_VALUE(groupName, SETTING_HIGH_TIME, .5);
    SET_VALUE(groupName, SETTING_NPULSES, 1);

    settings.endGroup();


    groupName = SETTINGSGROUP_ZAXIS;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_BAUD, 38400);
    SET_VALUE(groupName, SETTING_DEVICENUMBER, 1);
    SET_VALUE(groupName, SETTING_VELOCITY, 0.1);
    SET_VALUE(groupName, SETTING_POS, 0);
    SET_VALUE(groupName, SETTING_STEPSIZE, 0.1);

    settings.endGroup();

    groupName = SETTINGSGROUP_DDS;
    settings.beginGroup(groupName);

    SET_VALUE(groupName, SETTING_DEVNAME, "Dev2");

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

    SET_VALUE(groupName, SETTING_MULTIRUN_ENABLED, false);
    SET_VALUE(groupName, SETTING_NRUNS, 2);

    settings.endGroup();


    //////////////////////////////////////

    Tasks *t = optrode().NITasks();
    QString g = SETTINGSGROUP_MAINTRIG;
    t->setMainTrigTerm(value(g, SETTING_TERM).toString());

    g = SETTINGSGROUP_LED1;
    t->setLEDFreq(value(g, SETTING_FREQ).toDouble());
    t->setLED1Term(value(g, SETTING_TERM).toString());
    t->setLED1Enabled(value(g, SETTING_ENABLED).toBool());

    g = SETTINGSGROUP_LED2;
    t->setLED2Term(value(g, SETTING_TERM).toString());
    t->setLED2Enabled(value(g, SETTING_ENABLED).toBool());

    g = SETTINGSGROUP_ELREADOUT;
    t->setElectrodeReadoutPhysChan(value(g, SETTING_PHYSCHAN).toString());
    t->setElectrodeReadoutRate(value(g, SETTING_FREQ).toDouble());
    t->setElectrodeReadoutEnabled(value(g, SETTING_ENABLED).toBool());

    g = SETTINGSGROUP_STIMULATION;
    t->setStimulationHighTime(value(g, SETTING_HIGH_TIME).toDouble());
    t->setStimulationLowTime(value(g, SETTING_LOW_TIME).toDouble());
    t->setStimulationTerm(value(g, SETTING_TERM).toString());
    t->setStimulationEnabled(value(g, SETTING_ENABLED).toBool());
    t->setContinuousStimulation(value(g, SETTING_ALWAYS_ON).toBool());
    t->setAODEnabled(value(g, SETTING_AOD_ENABLED).toBool());

    g = SETTINGSGROUP_AUXSTIMULATION;
    t->setAuxStimulationEnabled(value(g, SETTING_ENABLED).toBool());
    t->setAuxStimulationTerm(value(g, SETTING_TERM).toString());
    t->setAuxStimulationHighTime(value(g, SETTING_HIGH_TIME).toDouble());
    t->setAuxStimulationNPulses(value(g, SETTING_NPULSES).toDouble());
    t->setAuxStimulationDelay(value(g, SETTING_INITIALDELAY).toDouble());

    g = SETTINGSGROUP_TIMING;
    t->setStimulationInitialDelay(value(g, SETTING_INITIALDELAY).toDouble());
    t->setStimulationDuration(value(g, SETTING_STIMDURATION).toDouble());
    optrode().setPostStimulation(value(g, SETTING_POSTSTIMULATION).toDouble());

    g = SETTINGSGROUP_ACQUISITION;
    optrode().setOutputDir(value(g, SETTING_OUTPUTPATH).toString());
    optrode().setRunName(value(g, SETTING_RUNNAME).toString());
    optrode().setSaveElectrodeEnabled(value(g, SETTING_SAVEELECTRODE).toBool());
    optrode().setSaveBehaviorEnabled(value(g, SETTING_SAVEBEHAVIOR).toBool());
    optrode().setMultiRunEnabled(value(g, SETTING_MULTIRUN_ENABLED).toBool());
    optrode().setNRuns(value(g, SETTING_NRUNS).toInt());

    g = SETTINGSGROUP_BEHAVCAMROI;
    optrode().getBehaviorCamera()->setROI(value(g, SETTING_ROI).toRect());

    g = SETTINGSGROUP_ZAXIS;
    PIDevice *dev = optrode().getZAxis();
    dev->setBaud(value(g, SETTING_BAUD).toUInt());
    dev->setDeviceNumber(value(g, SETTING_DEVICENUMBER).toUInt());
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        if (info.manufacturer() == "PI" || info.description().startsWith("PI")) {
            dev->setPortName(info.portName());
            break;
        }
    }

    g = SETTINGSGROUP_DDS;
    t->getDDS()->setDevName(value(g, SETTING_DEVNAME).toString());
}

void Settings::saveSettings()
{
    Tasks *t = optrode().NITasks();

    QString g = SETTINGSGROUP_MAINTRIG;
    setValue(g, SETTING_TERM, t->getMainTrigTerm());

    g = SETTINGSGROUP_BEHAVCAMROI;
    setValue(g, SETTING_ROI, optrode().getBehaviorCamera()->getROI());

    g = SETTINGSGROUP_LED1;
    setValue(g, SETTING_FREQ, t->getLEDFreq());
    setValue(g, SETTING_TERM, t->getLED1Term());
    setValue(g, SETTING_ENABLED, t->getLED1Enabled());

    g = SETTINGSGROUP_LED2;
    setValue(g, SETTING_TERM, t->getLED2Term());
    setValue(g, SETTING_ENABLED, t->getLED2Enabled());

    g = SETTINGSGROUP_ELREADOUT;
    setValue(g, SETTING_PHYSCHAN, t->getElectrodeReadoutPhysChan());
    setValue(g, SETTING_FREQ, t->getElectrodeReadoutRate());
    setValue(g, SETTING_ENABLED, t->getElectrodeReadoutEnabled());

    g = SETTINGSGROUP_STIMULATION;
    setValue(g, SETTING_LOW_TIME, t->getStimulationLowTime());
    setValue(g, SETTING_HIGH_TIME, t->getStimulationHighTime());
    setValue(g, SETTING_TERM, t->getStimulationTerm());
    setValue(g, SETTING_ENABLED, t->getStimulationEnabled());
    setValue(g, SETTING_ALWAYS_ON, t->getContinuousStimulation());
    setValue(g, SETTING_AOD_ENABLED, t->isAODEnabled());

    g = SETTINGSGROUP_AUXSTIMULATION;
    setValue(g, SETTING_ENABLED, t->getAuxStimulationEnabled());
    setValue(g, SETTING_TERM, t->getAuxStimulationTerm());
    setValue(g, SETTING_HIGH_TIME, t->getAuxStimulationHighTime());
    setValue(g, SETTING_NPULSES, t->getAuxStimulationNPulses());
    setValue(g, SETTING_INITIALDELAY, t->getAuxStimulationDelay());

    g = SETTINGSGROUP_TIMING;
    setValue(g, SETTING_INITIALDELAY, t->getStimulationInitialDelay());
    setValue(g, SETTING_STIMDURATION, t->stimulationDuration());
    setValue(g, SETTING_POSTSTIMULATION, optrode().getPostStimulation());

    g = SETTINGSGROUP_ACQUISITION;
    setValue(g, SETTING_OUTPUTPATH, optrode().getOutputDir());
    setValue(g, SETTING_RUNNAME, optrode().getRunName());
    setValue(g, SETTING_SAVEELECTRODE, optrode().isSaveElectrodeEnabled());
    setValue(g, SETTING_SAVEBEHAVIOR, optrode().isSaveBehaviorEnabled());
    setValue(g, SETTING_MULTIRUN_ENABLED, optrode().isMultiRunEnabled());
    setValue(g, SETTING_NRUNS, optrode().getNRuns());

    g = SETTINGSGROUP_ZAXIS;
    PIDevice *dev = optrode().getZAxis();
    setValue(g, SETTING_BAUD, dev->getBaud());
    setValue(g, SETTING_DEVICENUMBER, dev->getDeviceNumber());

    g = SETTINGSGROUP_DDS;
    setValue(g, SETTING_DEVNAME, t->getDDS()->getDevName());

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

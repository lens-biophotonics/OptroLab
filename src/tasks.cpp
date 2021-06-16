#include <qtlab/core/logmanager.h>

#include "tasks.h"

static Logger *logger = logManager().getLogger("Tasks");


Tasks::Tasks(QObject *parent) : QObject(parent)
{
    mainTrigger = new NITask(this);
    stimulation = new NITask(this);
    LED1 = new NITask(this);
    LED2 = new NITask(this);
    elReadout = new NITask(this);
}

void Tasks::init()
{
    QList<NITask *> taskList;
    QList<NITask *> triggeredTasks;

    triggeredTasks << stimulation << LED1 << LED2;

    taskList << triggeredTasks << elReadout << mainTrigger;

    for (NITask *t : taskList) {
        if (t->isInitialized())
            t->clearTask();
    }


    // mainTrigger
    mainTrigger->createTask("mainTrigger");
    mainTrigger->createCOPulseChanFreq(mainTrigPhysChan.toLatin1(),
                                       nullptr,
                                       NITask::FreqUnits_Hz,
                                       NITask::IdleState_Low,
                                       0, 2 * LEDFreq, 0.5);

    if (freeRunEnabled) {
        mainTrigger->cfgImplicitTiming(NITask::SampMode_ContSamps, 10);
    } else {
        mainTrigger->cfgImplicitTiming(NITask::SampMode_FiniteSamps, getMainTrigNPulses());
        logger->info(QString("Total number of trigger pulses: %1").arg(getMainTrigNPulses()));
    }
    mainTrigger->setCOPulseTerm(nullptr, mainTrigTerm.toLatin1());


    // stimulation
    stimulation->createTask("stimulation");
    stimulation->createCOPulseChanTime(
        stimulationCounter.toLatin1(),
        nullptr,
        DAQmx_Val_Seconds,
        NITask::IdleState_Low,
        stimulationDelay,
        stimulationLowTime,
        stimulationHighTime);
    stimulation->setCOPulseTerm(nullptr, stimulationTerm.toLatin1());
    if (stimulationEnabled) {
        stimulation->cfgImplicitTiming(NITask::SampMode_FiniteSamps,
                                       stimulationNPulses);
    }


    // LED1
    double LEDPeriod = 1 / LEDFreq;
    double initDelay1, initDelay2, tempLEDFreq;

    if (LED1Enabled && LED2Enabled) {
        initDelay1 = LEDPeriod - LEDdelay;
        initDelay2 = 0.5 * LEDPeriod - LEDdelay;
        tempLEDFreq = LEDFreq;
    }
    else {
        initDelay1 = 0;
        initDelay2 = 0;
        tempLEDFreq = 1 / totalDuration / 2;  // always on
    }

    LED1->createTask("LED1");
    LED1->createCOPulseChanFreq(LED1PhysChan.toLatin1(),
                                nullptr,
                                NITask::FreqUnits_Hz,
                                NITask::IdleState_Low,
                                initDelay1, tempLEDFreq, 0.5);
    LED1->setCOPulseTerm(nullptr, LED1Term.toLatin1());
    LED1->cfgImplicitTiming(NITask::SampMode_ContSamps, 1000);


    // LED2
    LED2->createTask("LED2");
    LED2->createCOPulseChanFreq(LED2PhysChan.toLatin1(),
                                nullptr,
                                NITask::FreqUnits_Hz,
                                NITask::IdleState_Low,
                                initDelay2, tempLEDFreq, 0.5);
    LED2->setCOPulseTerm(nullptr, LED2Term.toLatin1());
    LED2->cfgImplicitTiming(NITask::SampMode_ContSamps, 1000);


    // electrodeReadout
    elReadout->createTask("electrodeReadout");
    elReadout->createAIVoltageChan(electrodeReadoutPhysChan.toLatin1(),
                                   nullptr,
                                   NITask::TermConf_Default,
                                   -10., 10.,
                                   NITask::VoltUnits_Volts, nullptr);

    double sBuffer;  // how many seconds of buffering
    NITask::SampleMode sampleMode;

    if (freeRunEnabled) {
        sBuffer = 2;
        sampleMode = NITask::SampMode_ContSamps;
    } else {
        sBuffer = totalDuration;
        sampleMode = NITask::SampMode_FiniteSamps;
    }

    if (electrodeReadoutEnabled) {
        elReadout->cfgSampClkTiming(
            nullptr,
            electrodeReadoutRate,
            NITask::Edge_Rising,
            sampleMode,
            sBuffer * electrodeReadoutRate);
        elReadout->setReadReadAllAvailSamp(true);
        elReadout->cfgDigEdgeStartTrig(
            elReadoutTriggerTerm.toStdString().c_str(),
            NITask::Edge_Rising);
    }

    // configure start triggers
    for (NITask *task : triggeredTasks) {
        task->cfgDigEdgeStartTrig(mainTrigTerm.toStdString().c_str(),
                                  NITask::Edge_Rising);
    }

    initialized = true;
}

NITask *Tasks::electrodeReadout()
{
    return elReadout;
}

void Tasks::start()
{
    if (!initialized) {
        init();
    }
    if (!isFreeRunEnabled()) {
        if (stimulationEnabled) {
            stimulation->startTask();
        }
        if (LED1Enabled) {
            LED1->startTask();
        }
        if (LED2Enabled) {
            LED2->startTask();
        }
    }
    if (electrodeReadoutEnabled) {
        elReadout->startTask();
        emit elReadoutStarted();
    }

    // last to be started because it will trigger the other tasks
    mainTrigger->startTask();
}

void Tasks::stop()
{
    initialized = false;
    mainTrigger->stopTask();
    stimulation->stopTask();
    stopLEDs();
    elReadout->stopTask();
}

void Tasks::stopLEDs()
{
    LED1->stopTask();
    LED2->stopTask();
}

bool Tasks::getElectrodeReadoutEnabled() const
{
    return electrodeReadoutEnabled;
}

void Tasks::setElectrodeReadoutEnabled(bool value)
{
    electrodeReadoutEnabled = value;
}

QString Tasks::getElectrodeReadoutTriggerTerm() const
{
    return elReadoutTriggerTerm;
}

void Tasks::setElectrodeReadoutTriggerTerm(const QString &value)
{
    elReadoutTriggerTerm = value;
}

void Tasks::setLEDdelay(double value)
{
    LEDdelay = value;
    logger->info(QString("LEDdelay: %1").arg(LEDdelay));
}

bool Tasks::getLED2Enabled() const
{
    return LED2Enabled;
}

void Tasks::setLED2Enabled(bool value)
{
    LED2Enabled = value;
}

bool Tasks::getLED1Enabled() const
{
    return LED1Enabled;
}

void Tasks::setLED1Enabled(bool value)
{
    LED1Enabled = value;
}

QString Tasks::getMainTrigTerm() const
{
    return mainTrigTerm;
}

void Tasks::setMainTrigTerm(const QString &value)
{
    mainTrigTerm = value;
}

QString Tasks::getLED2Term() const
{
    return LED2Term;
}

void Tasks::setLED2Term(const QString &value)
{
    LED2Term = value;
}

QString Tasks::getLED1Term() const
{
    return LED1Term;
}

void Tasks::setLED1Term(const QString &value)
{
    LED1Term = value;
}

QString Tasks::getLED2PhysChan() const
{
    return LED2PhysChan;
}

void Tasks::setLED2PhysChan(const QString &value)
{
    LED2PhysChan = value;
}

QString Tasks::getLED1PhysChan() const
{
    return LED1PhysChan;
}

void Tasks::setLED1PhysChan(const QString &value)
{
    LED1PhysChan = value;
}

double Tasks::getLEDFreq() const
{
    return LEDFreq;
}

void Tasks::setLEDFreq(double value)
{
    LEDFreq = value;
}

void Tasks::setTotalDuration(double value)
{
    totalDuration = value;
}

NITask *Tasks::getElReadout() const
{
    return elReadout;
}

double Tasks::getStimulationInitialDelay() const
{
    return stimulationDelay;
}

void Tasks::setStimulationInitialDelay(double value)
{
    stimulationDelay = value;
}

bool Tasks::isFreeRunEnabled() const
{
    return freeRunEnabled;
}

void Tasks::setFreeRunEnabled(bool value)
{
    freeRunEnabled = value;
}

/**
 * @brief Set the duration of the stimulation
 * @param s Duration in seconds.
 *
 * \note Set stimulation pulse frequency beforehand. This will call setStimulationNPulses().
 */

void Tasks::setStimulationDuration(double s)
{
    setStimulationNPulses(s * getStimulationFrequency());
}

double Tasks::stimulationDuration()
{
    return stimulationNPulses / getStimulationFrequency();
}

QString Tasks::getMainTrigPhysChan() const
{
    return mainTrigPhysChan;
}

void Tasks::setMainTrigPhysChan(const QString &value)
{
    mainTrigPhysChan = value;
}

double Tasks::getMainTrigFreq() const
{
    return 2 * getLEDFreq();
}

double Tasks::getMainTrigNPulses() const
{
    return totalDuration * getMainTrigFreq();
}

QString Tasks::getStimulationCounter() const
{
    return stimulationCounter;
}

void Tasks::setStimulationCounter(const QString &value)
{
    stimulationCounter = value;
}

QString Tasks::getStimulationTerm() const
{
    return stimulationTerm;
}

void Tasks::setStimulationTerm(const QString &value)
{
    stimulationTerm = value;
}

double Tasks::getStimulationLowTime() const
{
    return stimulationLowTime;
}

void Tasks::setStimulationLowTime(double value)
{
    stimulationLowTime = value;
}

double Tasks::getStimulationHighTime() const
{
    return stimulationHighTime;
}

void Tasks::setStimulationHighTime(double value)
{
    stimulationHighTime = value;
}

double Tasks::getStimulationFrequency()
{
    return 1. / (stimulationLowTime + stimulationHighTime);
}

uInt64 Tasks::getStimulationNPulses() const
{
    return stimulationNPulses;
}

void Tasks::setStimulationNPulses(const uInt64 &value)
{
    stimulationNPulses = value;
}

bool Tasks::getStimulationEnabled() const
{
    return stimulationEnabled;
}

void Tasks::setStimulationEnabled(bool value)
{
    stimulationEnabled = value;
}

QString Tasks::getElectrodeReadoutPhysChan() const
{
    return electrodeReadoutPhysChan;
}

void Tasks::setElectrodeReadoutPhysChan(const QString &value)
{
    electrodeReadoutPhysChan = value;
}

double Tasks::getElectrodeReadoutRate() const
{
    return electrodeReadoutRate;
}

void Tasks::setElectrodeReadoutRate(double value)
{
    electrodeReadoutRate = value;
}

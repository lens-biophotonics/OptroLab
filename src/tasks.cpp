#include <qtlab/core/logmanager.h>

#include "tasks.h"

static Logger *logger = logManager().getLogger("BehavDispWorker");


Tasks::Tasks(QObject *parent) : QObject(parent)
{
    mainTrigger = new NITask("mainTrigger", this);
    behavCamTrigger = new NITask("behavCamTrigger", this);
    shutterPulse = new NITask("shutterPulse", this);
    LED1 = new NITask("LED1", this);
    LED2 = new NITask("LED2", this);
    elReadout = new NITask("electrodeReadout", this);
}

void Tasks::init()
{
    QList<NITask *> taskList;
    QList<NITask *> triggeredTasks;

    triggeredTasks << behavCamTrigger << shutterPulse << elReadout << LED1 << LED2;

    taskList << triggeredTasks << mainTrigger;

    for (NITask *t : taskList) {
        if (t->isInitialized())
            t->clearTask();
    }


    // mainTrigger
    mainTrigger->createTask();
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

    // behavior camera trigger
    behavCamTrigger->createTask();


    // shutterPulse
    shutterPulse->createTask();
    shutterPulse->createCOPulseChanFreq(
        shutterPulseCounter.toLatin1(),
        nullptr,
        DAQmx_Val_Hz,
        NITask::IdleState_Low,
        shutterInitialDelay,
        shutterPulseFrequency,
        shutterPulseDuty);
    shutterPulse->setCOPulseTerm(nullptr, shutterPulseTerm.toLatin1());
    shutterPulse->cfgImplicitTiming(NITask::SampMode_FiniteSamps,
                                    shutterPulseNPulses);


    // LED1
    LED1->createTask();
    LED1->createCOPulseChanFreq(LED1PhysChan.toLatin1(),
                                nullptr,
                                NITask::FreqUnits_Hz,
                                NITask::IdleState_Low,
                                0, LEDFreq, 0.5);
    LED1->setCOPulseTerm(nullptr, LED1Term.toLatin1());
    LED1->cfgImplicitTiming(NITask::SampMode_ContSamps, 1000);


    // LED2
    LED2->createTask();
    LED2->createCOPulseChanFreq(LED2PhysChan.toLatin1(),
                                nullptr,
                                NITask::FreqUnits_Hz,
                                NITask::IdleState_Low,
                                1 / (2 * LEDFreq), LEDFreq, 0.5);
    LED2->setCOPulseTerm(nullptr, LED2Term.toLatin1());
    LED2->cfgImplicitTiming(NITask::SampMode_ContSamps, 1000);


    // electrodeReadout
    elReadout->createTask();
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

    elReadout->cfgSampClkTiming(
        nullptr,
        electrodeReadoutRate,
        NITask::Edge_Rising,
        sampleMode,
        sBuffer * electrodeReadoutRate);
    elReadout->setReadReadAllAvailSamp(true);


    // configure start triggers
    for (NITask *task : triggeredTasks) {
        task->cfgDigEdgeStartTrig(mainTrigTerm.toStdString().c_str(),
                                  NITask::Edge_Rising);
    }
}

NITask *Tasks::electrodeReadout()
{
    return elReadout;
}

void Tasks::start()
{
    init();
    if (!isFreeRunEnabled()) {
        shutterPulse->startTask();
        LED1->startTask();
        LED2->startTask();
    }
    elReadout->startTask();
//    behavCamTrigger->startTask();

    // last to be started because it will trigger the other tasks
    mainTrigger->startTask();
}

void Tasks::stop()
{
    mainTrigger->stopTask();
    shutterPulse->stopTask();
    LED1->stopTask();
    LED2->stopTask();
    elReadout->stopTask();
//    behavCamTrigger->stopTask();
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

QString Tasks::getBehavCamTrigPhysChan() const
{
    return behavCamTrigPhysChan;
}

void Tasks::setBehavCamTrigPhysChan(const QString &value)
{
    behavCamTrigPhysChan = value;
}

double Tasks::getBehavCamTrigFreq() const
{
    return behavCamTrigFreq;
}

void Tasks::setBehavCamTrigFreq(double value)
{
    behavCamTrigFreq = value;
}

void Tasks::setTotalDuration(double value)
{
    totalDuration = value;
}

NITask *Tasks::getElReadout() const
{
    return elReadout;
}

double Tasks::getShutterInitialDelay() const
{
    return shutterInitialDelay;
}

void Tasks::setShutterInitialDelay(double value)
{
    shutterInitialDelay = value;
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
 * @brief Set the duration of the stimulation (shutter pulse)
 * @param s Duration in seconds.
 *
 * \note Set shutter pulse frequency beforehand. This will call setShutterPulseNPulses().
 */

void Tasks::setStimulationDuration(double s)
{
    setShutterPulseNPulses(s * shutterPulseFrequency);
}

double Tasks::stimulationDuration()
{
    return shutterPulseNPulses / shutterPulseFrequency;
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

QString Tasks::getShutterPulseCounter() const
{
    return shutterPulseCounter;
}

void Tasks::setShutterPulseCounter(const QString &value)
{
    shutterPulseCounter = value;
}

QString Tasks::getShutterPulseTerm() const
{
    return shutterPulseTerm;
}

void Tasks::setShutterPulseTerm(const QString &value)
{
    shutterPulseTerm = value;
}

double Tasks::getShutterPulseFrequency() const
{
    return shutterPulseFrequency;
}

void Tasks::setShutterPulseFrequency(double value)
{
    shutterPulseFrequency = value;
}

double Tasks::getShutterPulseDuty() const
{
    return shutterPulseDuty;
}

void Tasks::setShutterPulseDuty(double value)
{
    shutterPulseDuty = value;
}

uInt64 Tasks::getShutterPulseNPulses() const
{
    return shutterPulseNPulses;
}

void Tasks::setShutterPulseNPulses(const uInt64 &value)
{
    shutterPulseNPulses = value;
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

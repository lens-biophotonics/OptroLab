#include "tasks.h"

Tasks::Tasks(QObject *parent) : QObject(parent)
{
    mainTrigger = new NITask("mainTrigger", this);
    shutterPulse = new NITask("shutterPulse", this);
    elReadout = new NITask("electrodeReadout", this);

    needsInit = true;
}

void Tasks::init()
{
    QList<NITask *> taskList;

    taskList << mainTrigger << shutterPulse << elReadout;

    for (NITask *t : taskList) {
        if (t->isInitialized())
            t->clearTask();
    }


    // mainTrigger
    // we have to use an AO for the main trigger because this board has only 2
    // counters and they are both used for the SampMode_FiniteSamps in task
    // shutterPulse
    mainTrigger->createTask();
    mainTrigger->createAOVoltageChan(mainTrigPhysChan.toLatin1(),
                                     nullptr,
                                     0, 5, // range
                                     NITask::VoltUnits_Volts, nullptr);

    // because there are 2 samples, the sampling rate is twice the wanted
    // square wave frequency
    mainTrigger->cfgSampClkTiming(nullptr,
                                  2 * mainTrigFreq,
                                  NITask::Edge_Rising,
                                  NITask::SampMode_ContSamps, 2);
    const float64 data[] = {5, 0};
    int32 sampsPerChanWritten;
    mainTrigger->writeAnalogF64(2,      // number of samples
                                false,  // autostart
                                .2,     // timeout
                                NITask::DataLayout_GroupByChannel,
                                data,
                                &sampsPerChanWritten);
#ifdef WITH_HARDWARE
    if (sampsPerChanWritten != 2)
        throw std::runtime_error("invalid number of samples written");
#endif

    const char startTriggerSource[] = "ao/StartTrigger";



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
    shutterPulse->cfgDigEdgeStartTrig(startTriggerSource, NITask::Edge_Rising);



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
        "OnboardClock",
        electrodeReadoutRate,
        NITask::Edge_Rising,
        sampleMode,
        sBuffer * electrodeReadoutRate);
    elReadout->cfgDigEdgeStartTrig(startTriggerSource, NITask::Edge_Rising);
    elReadout->setReadReadAllAvailSamp(true);

    needsInit = false;
}

NITask *Tasks::electrodeReadout()
{
    return elReadout;
}

void Tasks::start()
{
    if (needsInit)
        init();
    if (!isFreeRunEnabled())
        shutterPulse->startTask();
    elReadout->startTask();

    // last to be started because it will trigger the other two tasks
    mainTrigger->startTask();
}

void Tasks::stop()
{
    mainTrigger->stopTask();
    shutterPulse->stopTask();
    elReadout->stopTask();
}

void Tasks::setTotalDuration(double value)
{
    totalDuration = value;
    needsInit = true;
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
    needsInit = true;
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
    needsInit = true;
    mainTrigPhysChan = value;
}

double Tasks::getMainTrigFreq() const
{
    return mainTrigFreq;
}

void Tasks::setMainTrigFreq(double value)
{
    needsInit = true;
    mainTrigFreq = value;
}

QString Tasks::getShutterPulseCounter() const
{
    return shutterPulseCounter;
}

void Tasks::setShutterPulseCounter(const QString &value)
{
    needsInit = true;
    shutterPulseCounter = value;
}

QString Tasks::getShutterPulseTerm() const
{
    return shutterPulseTerm;
}

void Tasks::setShutterPulseTerm(const QString &value)
{
    needsInit = true;
    shutterPulseTerm = value;
}

double Tasks::getShutterPulseFrequency() const
{
    return shutterPulseFrequency;
}

void Tasks::setShutterPulseFrequency(double value)
{
    needsInit = true;
    shutterPulseFrequency = value;
}

double Tasks::getShutterPulseDuty() const
{
    return shutterPulseDuty;
}

void Tasks::setShutterPulseDuty(double value)
{
    needsInit = true;
    shutterPulseDuty = value;
}

uInt64 Tasks::getShutterPulseNPulses() const
{
    return shutterPulseNPulses;
}

void Tasks::setShutterPulseNPulses(const uInt64 &value)
{
    needsInit = true;
    shutterPulseNPulses = value;
}

QString Tasks::getElectrodeReadoutPhysChan() const
{
    return electrodeReadoutPhysChan;
}

void Tasks::setElectrodeReadoutPhysChan(const QString &value)
{
    needsInit = true;
    electrodeReadoutPhysChan = value;
}

double Tasks::getElectrodeReadoutRate() const
{
    return electrodeReadoutRate;
}

void Tasks::setElectrodeReadoutRate(double value)
{
    needsInit = true;
    electrodeReadoutRate = value;
}

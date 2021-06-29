#include <qtlab/core/logmanager.h>

#include "tasks.h"
#include "optrode.h"
#include "dds.h"

#define MHZ_PER_PIXEL 0.151
#define MHZ_CENTRAL 95.0
#define MAX_POWER 3546

static Logger *logger = logManager().getLogger("Tasks");


Tasks::Tasks(QObject *parent) : QObject(parent)
{
    mainTrigger = new NITask(this);
    stimulation = new NITask(this);
    LED = new NITask(this);
    elReadout = new NITask(this);
    ddsSampClock = new NITask(this);
    dds = new DDS(this);
}

void Tasks::init()
{
    QList<NITask *> taskList;

    taskList << elReadout;

    if (!freeRunEnabled)  {
        taskList << LED;

        if (stimulationEnabled) {
            taskList << stimulation;

            if (aodEnabled) {
                taskList << dds->getTask();
                taskList << ddsSampClock;
            }
        }
    }
    taskList << mainTrigger;

    for (NITask *t : taskList) {
        if (t->isInitialized())
            t->clearTask();
    }

    // will cause routed signal to be disconnected
    // this is needed to reset the connection for LED2 (see below)
    NI::tristateOutputTerm(LED1Term);
    NI::tristateOutputTerm(LED2Term);

    QStringList coList;
    QStringList devList = NI::getSysDevNames();
    QStringListIterator devIt(devList);
    while (devIt.hasNext()) {
        QString dev = devIt.next();
        if (NI::getDevProductCategory(dev) == DAQmx_Val_XSeriesDAQ) {
            coList << NI::getDevCOPhysicalChans(dev).filter("/ctr");
        }
    }

    QString co;

    // mainTrigger
    co = coList.at(0);
    mainTrigger->createTask("mainTrigger");
    mainTrigger->createCOPulseChanFreq(co,
                                       nullptr,
                                       NITask::FreqUnits_Hz,
                                       NITask::IdleState_Low,
                                       0, 2 * LEDFreq, 0.5);
    mainTrigger->setCOPulseTerm(nullptr, mainTrigTerm);

    if (freeRunEnabled) {
        mainTrigger->cfgImplicitTiming(NITask::SampMode_ContSamps, 10);
    } else {
        mainTrigger->cfgImplicitTiming(NITask::SampMode_FiniteSamps, getMainTrigNPulses());
        logger->info(QString("Total number of trigger pulses: %1").arg(getMainTrigNPulses()));
    }


    // electrodeReadout
    elReadout->createTask("electrodeReadout");
    elReadout->createAIVoltageChan(electrodeReadoutPhysChan,
                                   nullptr,
                                   NITask::TermConf_Default,
                                   -10., 10.,
                                   NITask::VoltUnits_Volts, nullptr);
    elReadout->cfgDigEdgeStartTrig(mainTrigTerm.toStdString().c_str(), NITask::Edge_Rising);

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
    }

    if (freeRunEnabled) {
        initialized = true;
        return;
    }


    // LED1
    double LEDPeriod = 1 / LEDFreq;
    double initDelay, tempLEDFreq;

    if (LED1Enabled && LED2Enabled) {
        initDelay = LEDPeriod - LEDdelay;
        tempLEDFreq = LEDFreq;
    }
    else {
        initDelay = 0;
        tempLEDFreq = 1 / totalDuration / 2; // always on
    }

    co = coList.at(1);
    LED->createTask("LED");
    LED->createCOPulseChanFreq(co,
                               nullptr,
                               NITask::FreqUnits_Hz,
                               NITask::IdleState_Low,
                               initDelay, tempLEDFreq, 0.5);
    LED->resetCOPulseTerm(nullptr);
    QString ledTerm = LED1Term;
    if (LED1Enabled && LED2Enabled) {
        NI::connectTerms(LED1Term, LED2Term, DAQmx_Val_InvertPolarity); // LED2
    } else if (LED2Enabled) {
        ledTerm = LED2Term;
    }

    if (!ledTerm.isNull()) {
        LED->setCOPulseTerm(nullptr, ledTerm);
        LED->cfgImplicitTiming(NITask::SampMode_ContSamps, 1000);
        LED->cfgDigEdgeStartTrig(mainTrigTerm.toStdString().c_str(), NITask::Edge_Rising);
    }

    // stimulation
    /* if aod is enabled, this signal is used as the input UDCLK for the dds: it controls when the
     * newly written dds configuration becomes effective. */

    if (stimulationEnabled) {
        QString stimulationCounter = coList.at(2);
        stimulation->createTask("stimulation");
        if (continuousStimulation) {
            stimulation->createCOPulseChanFreq(
                stimulationCounter,
                nullptr,
                DAQmx_Val_Hz,
                NITask::IdleState_Low,
                stimulationDelay,
                1 / stimulationDuration() / 2, // always on
                0.5);
        } else {
            stimulation->createCOPulseChanTime(
                stimulationCounter,
                nullptr,
                DAQmx_Val_Seconds,
                NITask::IdleState_Low,
                stimulationDelay, // ignored on buffered implicit pulse trains
                stimulationLowTime, // ignored on buffered implicit pulse trains (page 7-31 DAQ X)
                continuousStimulation ? 3e9 : stimulationHighTime); // as above
        }

        stimulation->setCOPulseTerm(nullptr, stimulationTerm);
        stimulation->cfgImplicitTiming(NITask::SampMode_FiniteSamps, stimulationNPulses);
        stimulation->setCOCtrTimebaseSrc(nullptr, "/Dev1/20MhzTimeBase");
        stimulation->cfgDigEdgeStartTrig(mainTrigTerm.toStdString().c_str(), NITask::Edge_Rising);

        if (aodEnabled && !continuousStimulation) {
            // for each stimulation cycle, we have to generate two short pulses (i.e. two UDCLK)
            const int NSamples = 2 * stimulationNPulses;

            QVector<float64> duty = QVector<float64>(NSamples, 0.1);
            QVector<float64> freq = QVector<float64>(NSamples);
            QVector<float64> highTime, lowTime;
            highTime.reserve(NSamples);
            lowTime.reserve(NSamples);

            for (int i = 0; i < NSamples / 2; ++i) {
                lowTime << 0.9 * stimulationHighTime;
                highTime << 0.1 * stimulationHighTime;
                lowTime << 0.9 * stimulationLowTime;
                highTime << 0.1 * stimulationLowTime;
            }

            lowTime[0] = stimulationDelay; // low times are output before high times..
                                           // so this is the way to implement delay with buffered
                                           // implicit pulse trains

            stimulation->writeCtrTime(NSamples, false, 1, NITask::DataLayout_GroupByChannel,
                                      highTime.data(), lowTime.data(), nullptr);
        }
    }

    if (aodEnabled) {
        dds->initTask();
        dds->setWriteMode(DDS::WRITE_MODE_TO_NI_TASK);
        dds->setIOUDCLKInternal(false); // hardware-timed dds output (see stimulation task)
        // go to XY point
        dds->setFrequency1(MHZ_CENTRAL - (point.y() - 256) * MHZ_PER_PIXEL,
                           MHZ_CENTRAL - (point.x() - 256) * MHZ_PER_PIXEL);

        dds->setOSKI(MAX_POWER, MAX_POWER); // turn on

        // write all samples to buffer
        dds->clearBuffer();
        dds->setWriteMode(DDS::WRITE_MODE_TO_BUFFER);
        dds->setOSKI(0, 0);                 // turn off
        dds->setOSKI(MAX_POWER, MAX_POWER); // turn on

        dds->getTask()->setStartTrigRetriggerable(true);
        dds->getTask()->cfgDigEdgeStartTrig(mainTrigTerm.toStdString().c_str(),
                                            NITask::Edge_Rising);

        /* ddsSampClock is a CO that is used to provide the sample clock to the task that writes to
         * the dds. In a stimulation cycle, it is started twice: the first time it runs (at the
         * start of the stimulation) it makes dds write the first N / 2 samples, to set the
         * amplitude to 0 (this will become effective only at the next UDCLK). The second time it
         * runs (at the end of the stimulation), it makes dds write the remaining N / 2 samples that
         * set the amplitude to the maximum (again, this will become effective at the nect UDCLK).

         * stimulationTerm is used as UDCLK for the dds as well as start trigger for ddsSampClock.
         */
        ddsSampClock->createTask("ddsSampClock");
        co = coList.last(); // need to use counter from another device
        ddsSampClock->createCOPulseChanFreq(
            co, nullptr, NITask::FreqUnits_Hz, NITask::IdleState_Low, 0, 5e6, 0.5);
        ddsSampClock->cfgImplicitTiming(NITask::SampMode_FiniteSamps,
                                        dds->getBufferSize() / 2);
        ddsSampClock->cfgDigEdgeStartTrig(stimulationTerm.toLatin1(), NITask::Edge_Rising);
        ddsSampClock->setStartTrigRetriggerable(true);
        dds->getTask()->cfgSampClkTiming(ddsSampClock->getCOPulseTerm(nullptr), 5e6,
                                         NITask::Edge_Rising, NITask::SampMode_ContSamps,
                                         dds->getBufferSize());
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
    if (electrodeReadoutEnabled) {
        elReadout->startTask();
        emit elReadoutStarted();
    }
    if (!isFreeRunEnabled()) {
        if (LED1Enabled || LED2Enabled) {
            LED->startTask();
        }
        if (stimulationEnabled) {
            stimulation->startTask();
            if (aodEnabled && !continuousStimulation) {
                ddsSampClock->startTask();
            }
        }
    }

    // last to be started because it will trigger the other tasks
    mainTrigger->startTask();
}

void Tasks::stop()
{
    initialized = false;
    mainTrigger->stopTask();
    elReadout->stopTask();

    if (!freeRunEnabled) {
        if (stimulationEnabled) {
            stimulation->stopTask();
            if (aodEnabled) {
                ddsSampClock->stopTask();
                dds->getTask()->stopTask();
            }
        }
        stopLEDs();
    }
}

void Tasks::stopLEDs()
{
    LED->stopTask();
    if (LED1Enabled && LED2Enabled) {
        NI::disconnectTerms(LED1Term, LED2Term);
    }
}

bool Tasks::getContinuousStimulation() const
{
    return continuousStimulation;
}

void Tasks::setContinuousStimulation(bool value)
{
    continuousStimulation = value;
}

QPointF Tasks::getPoint() const
{
    return point;
}

void Tasks::setPoint(const QPointF &value)
{
    point = value;
}

DDS *Tasks::getDDS() const
{
    return dds;
}

void Tasks::ddsMasterReset()
{
    dds->masterReset();
    dds->setPLL(false, false, 6);
    dds->setOSK(true, false);
    dds->setUpdateClock(2);
    dds->setOSKI(0, 0);
    dds->setFrequency1(MHZ_CENTRAL, MHZ_CENTRAL);
}

bool Tasks::getElectrodeReadoutEnabled() const
{
    return electrodeReadoutEnabled;
}

void Tasks::setElectrodeReadoutEnabled(bool value)
{
    electrodeReadoutEnabled = value;
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

double Tasks::getMainTrigFreq() const
{
    return 2 * getLEDFreq();
}

double Tasks::getMainTrigNPulses() const
{
    return totalDuration * getMainTrigFreq();
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

void Tasks::setAODEnabled(bool enable)
{
    aodEnabled = enable;
}

bool Tasks::isAODEnabled() const
{
    return aodEnabled;
}

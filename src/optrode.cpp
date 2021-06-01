#include <stdexcept>
#include <memory>

#include <QHistoryState>
#include <QDir>
#include <QThread>
#include <QTextStream>
#include <QtMath>

#include <qtlab/core/logger.h>
#include <qtlab/hw/ni/nitask.h>
#include <qtlab/hw/pi/pidevice.h>

#include <Spinnaker.h>

#include "optrode.h"
#include "tasks.h"
#include "chameleoncamera.h"
#include "savestackworker.h"
#include "elreadoutworker.h"
#include "behavworker.h"


static Logger *logger = getLogger("Optrode");

Optrode::Optrode(QObject *parent) : QObject(parent)
{
    behaviorCamera = new ChameleonCamera(this);
    tasks = new Tasks(this);
    orca  = new OrcaFlash(this);
    zAxis = new PIDevice("Z Axis", this);
    elReadoutWorker = new ElReadoutWorker(tasks->getElReadout());
    QThread *thread = new QThread();
    thread->setObjectName("ElReadoutWorker_thread");
    elReadoutWorker->moveToThread(thread);
    thread->start();

    thread = new QThread();
    thread->setObjectName("BehavWorker_thread");
    behavWorker = new BehavWorker(behaviorCamera);
    behavWorker->moveToThread(thread);
    thread->start();

    thread = new QThread();
    thread->setObjectName("SaveStackWorker_thread");
    ssWorker = new SaveStackWorker(orca);
    ssWorker->moveToThread(thread);
    thread->start();

    connect(ssWorker, &SaveStackWorker::error, this, &Optrode::onError);
    connect(ssWorker, &SaveStackWorker::captureCompleted,
            this, &Optrode::incrementCompleted);

    connect(ssWorker, &SaveStackWorker::started, this, [ = ]() {
        try {
            tasks->start();
        } catch (std::runtime_error e) {
            onError(e.what());
            return;
        }
        emit started(false);
    });

    connect(tasks, &Tasks::elReadoutStarted, elReadoutWorker, &ElReadoutWorker::start);
    connect(this, &Optrode::stopped, elReadoutWorker, &ElReadoutWorker::stop);
    connect(behavWorker, &BehavWorker::captureCompleted,
            this, &Optrode::incrementCompleted);
    connect(elReadoutWorker, &ElReadoutWorker::acquisitionCompleted,
            this, &Optrode::incrementCompleted);

    postStimulation = 0;

    setupStateMachine();
}

Optrode::~Optrode()
{
    uninitialize();
}

QState *Optrode::getState(const Optrode::MACHINE_STATE stateEnum)
{
    return stateMap[stateEnum];
}

void Optrode::initialize()
{
    emit initializing();

    try {
        int nOfOrcas = DCAM::init_dcam();
        if (nOfOrcas < 1) {
            throw std::runtime_error("Cannot find Hamamatsu cameras");
        }
        orca->open(0);

        orca->setSensorMode(OrcaFlash::SENSOR_MODE_AREA);
        orca->setTriggerSource(OrcaFlash::TRIGGERSOURCE_EXTERNAL);
        orca->setTriggerPolarity(OrcaFlash::POL_POSITIVE);
        orca->setPropertyValue(DCAM::DCAM_IDPROP_BINNING, DCAM::DCAMPROP_BINNING__4);
        orca->buf_alloc(3000);

        behaviorCamera->open(0);
        behaviorCamera->logDeviceInfo();
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }
    catch (Spinnaker::Exception e) {
        onError(QString("Cannot open behavior camera.\n\n%1").arg(e.what()));
        return;
    }

    for (int i = 0; i < 6; i++) {
        try {
            zAxis->connectDevice();
            break;
        } catch (std::runtime_error e) {
            logger->warning(QString("Cannot connect PI, attempt %1 / 6").arg(i + 1));
#ifdef Q_OS_WIN
            Sleep(1000);
#else
            int ms = 500;
            struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
            nanosleep(&ts, NULL);
#endif
        }
    }

    if (!zAxis->isConnected()) {
        logger->critical("Cannot connect PI. (Probably busy in PI MikroMove?)");
    }

    emit initialized();
}

void Optrode::uninitialize()
{
    stop();
    if (behaviorCamera)
        delete behaviorCamera;
    behaviorCamera = nullptr;
}

ChameleonCamera *Optrode::getBehaviorCamera() const
{
    return behaviorCamera;
}

Tasks *Optrode::NITasks() const
{
    return tasks;
}

bool Optrode::isFreeRunEnabled() const
{
    return tasks->isFreeRunEnabled();
}

void Optrode::setPostStimulation(double s)
{
    postStimulation = s;
}

/**
 * @brief Return the total duration of a single run.
 * @return seconds
 *
 * Baseline + stimulation + post stimulation
 */

double Optrode::totalDuration() const
{
    double d = tasks->getShutterInitialDelay();
    if (tasks->getShutterPulseEnabled()) {
        d += tasks->stimulationDuration();
        d += postStimulation;
    }

    return d;
}

void Optrode::setupStateMachine()
{
    std::function<QState*(const MACHINE_STATE, QState *parent)> newState
        = [this](const MACHINE_STATE type, QState *parent = nullptr) {
        QState *state = new QState(parent);
        this->stateMap[type] = state;
        return state;
    };

    sm = new QStateMachine();

    QState *uninitState = newState(STATE_UNINITIALIZED, sm);
    sm->setInitialState(uninitState);

    QState *initializingState = newState(STATE_INITIALIZING, sm);
    QState *readyState = newState(STATE_READY, sm);
    QState *capturingState = newState(STATE_CAPTURING, sm);

    uninitState->addTransition(this, &Optrode::initializing, initializingState);
    initializingState->addTransition(this, &Optrode::initialized, readyState);
    initializingState->addTransition(this, &Optrode::error, uninitState);
    readyState->addTransition(this, &Optrode::started, capturingState);
    capturingState->addTransition(this, &Optrode::stopped, readyState);

    QHistoryState *historyState = new QHistoryState(sm);

    QState *errorState = newState(STATE_ERROR, sm);
    errorState->addTransition(historyState);

    sm->addTransition(this, &Optrode::error, errorState);

    sm->start();
}

void Optrode::startFreeRun()
{
    logger->info("Start acquisition (free run)");
    tasks->setFreeRunEnabled(true);
    behavWorker->setSaveToFileEnabled(false);
    behavWorker->setFrameCount(-1);
    elReadoutWorker->setSaveToFileEnabled(false);
    _startAcquisition();
    try {
        tasks->start();
    } catch (std::runtime_error e) {
        emit error(e.what());
        return;
    }
    emit started(true);
}

void Optrode::start()
{
    completedJobs = successJobs = 0;
    if (tasks->getElectrodeReadoutEnabled()) {
        nJobs = 3;
    } else {
        nJobs = 2;
    }

    logger->info("Start acquisition");
    logger->info(QString("Baseline %1s, stimul %2s (%3 pulses), post %4s")
                 .arg(tasks->getShutterInitialDelay())
                 .arg(tasks->stimulationDuration())
                 .arg(tasks->getShutterPulseNPulses())
                 .arg(getPostStimulation()));
    logger->info(QString("Total duration: %1s").arg(totalDuration()));

    // setup worker thread


    tasks->setFreeRunEnabled(false);
    tasks->setTotalDuration(totalDuration());

    elReadoutWorker->setOutputFile(outputFileFullPath() + ".dat");
    elReadoutWorker->setSaveToFileEnabled(
        saveElectrodeEnabled && tasks->getElectrodeReadoutEnabled());

    behavWorker->setSaveToFileEnabled(saveBehaviorEnabled);
    behavWorker->setOutputFile(outputFileFullPath());

    size_t frameCount = tasks->getMainTrigFreq() * totalDuration();
    ssWorker->setFrameCount(frameCount);
    ssWorker->setTimeout(2e6 / tasks->getMainTrigFreq());
    ssWorker->setOutputFile(outputFileFullPath());
    behavWorker->setFrameCount(frameCount);

    _startAcquisition();

    try {
        tasks->init();
    } catch (std::runtime_error e) {
        emit error(e.what());
        return;
    }
    ssWorker->requestStart();
    writeRunParams();
}

void Optrode::stop()
{
    if (!running)
        return;
    running = false;
    emit stopped();
    try {
        behaviorCamera->stopAcquisition();
        orca->cap_stop();
        tasks->stop();
    } catch (std::runtime_error e) {
        onError(e.what());
    } catch (Spinnaker::Exception e) {
        onError(e.what());
    }
    logger->info("Stopped");
}

PIDevice *Optrode::getZAxis() const
{
    return zAxis;
}

SaveStackWorker *Optrode::getSSWorker() const
{
    return ssWorker;
}

bool Optrode::isSaveBehaviorEnabled() const
{
    return saveBehaviorEnabled;
}

void Optrode::setSaveBehaviorEnabled(bool enable)
{
    saveBehaviorEnabled = enable;
}

bool Optrode::isSaveElectrodeEnabled() const
{
    return saveElectrodeEnabled;
}

void Optrode::setSaveElectrodeEnabled(bool enable)
{
    saveElectrodeEnabled = enable;
}

BehavWorker *Optrode::getBehavWorker() const
{
    return behavWorker;
}

bool Optrode::isSuccess()
{
    return successJobs == nJobs;
}

ElReadoutWorker *Optrode::getElReadoutWorker() const
{
    return elReadoutWorker;
}

QString Optrode::getRunName() const
{
    return runName;
}

void Optrode::setRunName(const QString &value)
{
    runName = value;
}

void Optrode::writeRunParams(QString fileName)
{
    QFile outFile(fileName);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit error(QString("Cannot open output file %1")
                   .arg(outFile.fileName()));
        return;
    };

    QTextStream out(&outFile);
    out << "camera_rate: " << tasks->getMainTrigFreq() << "\n";
    if (!tasks->getLED1Enabled() || !tasks->getLED2Enabled()) {
        out << "led_rate: " << 0 << "\n";
    } else {
        out << "led_rate: " << tasks->getLEDFreq() << "\n";
    }
    out << "orca_exposure_time: " << orca->getExposureTime() << "\n";
    out << "stimul_duty: " << tasks->getShutterPulseDuty() << "\n";
    out << "stimul_frequency: " << tasks->getShutterPulseFrequency() << "\n";
    out << "stimul_n_pulses: " << tasks->getShutterPulseNPulses() << "\n";
    out << "electrode_readout_rate: " << tasks->getElectrodeReadoutRate() << "\n";

    out << "timing:" << "\n";
    out << "  baseline: " <<  tasks->getShutterInitialDelay() << "\n";
    out << "  stimulation: " << tasks->stimulationDuration() << "\n";
    out << "  post: " << getPostStimulation() << "\n";
    out << "  total: " << totalDuration() << "\n";

    outFile.close();

    logger->info("Saved run params to " + fileName);
}

void Optrode::writeRunParams()
{
    QString fname = outputFileFullPath() + ".yaml";
    writeRunParams(fname);
}

QString Optrode::getOutputDir() const
{
    return outputPath;
}

void Optrode::setOutputDir(const QString &value)
{
    outputPath = value;
}

/**
 * @brief Full output path without file extension
 * @return Concatenation of outputPath and runName
 */

QString Optrode::outputFileFullPath()
{
    return QDir(outputPath).filePath(runName);
}

OrcaFlash *Optrode::getOrca() const
{
    return orca;
}

double Optrode::getPostStimulation() const
{
    return postStimulation;
}

void Optrode::_startAcquisition()
{
    running = true;
    try {
        double Vn = 2048;
        double lineInterval = orca->getLineInterval();
        double mainTrigFreq = tasks->getMainTrigFreq();

        // time during which LEDs are switching on/off
        // (camera should not be recording during this time)
        double blankTime = 0.002;

        // inverse formula to obtain exposure time
        double expTime = 1. / tasks->getMainTrigFreq() - (Vn / 2 + 10) * lineInterval;
        expTime -= blankTime;
        orca->setGetExposureTime(expTime);

        elReadoutWorker->setTotToBeRead(totalDuration() * tasks->getElectrodeReadoutRate());
        elReadoutWorker->setFreeRun(isFreeRunEnabled());

        double LEDFreq = mainTrigFreq / 2;
        tasks->setLEDFreq(LEDFreq);
        tasks->setLEDdelay(blankTime / 2);

        uint enabledWriters = 0;
        if (tasks->getLED1Enabled()) {
            enabledWriters += 0b01;
        }
        if (tasks->getLED2Enabled()) {
            enabledWriters += 0b10;
        }

        ssWorker->setEnabledWriters(enabledWriters);

        behaviorCamera->startAcquisition();
        orca->cap_start();
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    } catch (Spinnaker::Exception e) {
        onError(e.what());
        return;
    }
}

Optrode &optrode()
{
    static auto instance = std::make_unique<Optrode>(nullptr);
    return *instance;
}

void Optrode::onError(const QString &errMsg)
{
    stop();
    emit error(errMsg);
    logger->critical(errMsg);
}

void Optrode::incrementCompleted(bool ok)
{
    if (ok) {
        ++successJobs;
    }
    if (successJobs == 1) {
        tasks->stopLEDs();
        ssWorker->signalTriggerCompletion();
        emit pleaseWait();
    }
    if (++completedJobs == nJobs) {
        logger->info("All jobs completed");
        stop();
    }
    logger->info(QString("Completed %1 jobs (%2)").arg(completedJobs).arg(ok));
};

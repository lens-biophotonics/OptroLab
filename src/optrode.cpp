#include <stdexcept>
#include <memory>

#include <QHistoryState>
#include <QDir>
#include <QTextStream>
#include <QtMath>

#include <qtlab/core/logger.h>
#include <qtlab/hw/ni/nitask.h>

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
    elReadoutWorker = new ElReadoutWorker(tasks->getElReadout());
    QThread *thread = new QThread();
    thread->setObjectName("ElReadoutWorker_thread");
    elReadoutWorker->moveToThread(thread);
    behavWorker = new BehavWorker(behaviorCamera);
    thread->start();

    connect(this, &Optrode::started, elReadoutWorker, &ElReadoutWorker::start);
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
    }
    catch (Spinnaker::Exception e) {
        onError(QString("Cannot open behavior camera.\n\n%1").arg(e.what()));
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
    d += tasks->stimulationDuration();
    d += postStimulation;

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
    _startAcquisition();
    tasks->start();
    emit started(true);
}

void Optrode::start()
{
    completedJobs = 0;
    logger->info("Start acquisition");
    logger->info(QString("Baseline %1s, stimul %2s (%3 pulses), post %4s")
                 .arg(tasks->getShutterInitialDelay())
                 .arg(tasks->stimulationDuration())
                 .arg(tasks->getShutterPulseNPulses())
                 .arg(getPostStimulation()));
    logger->info(QString("Total duration: %1s").arg(totalDuration()));

    // setup worker thread
    SaveStackWorker *worker = new SaveStackWorker(orca);

    connect(worker, &QThread::finished, worker, &QThread::deleteLater);
    worker->setOutputFile(outputFileFullPath());

    connect(worker, &SaveStackWorker::error, this, &Optrode::onError);
    connect(worker, &SaveStackWorker::captureCompleted,
            this, &Optrode::incrementCompleted);
    connect(worker, &SaveStackWorker::started, tasks, &Tasks::start);
    connect(elReadoutWorker, &ElReadoutWorker::acquisitionCompleted,
            worker, &SaveStackWorker::signalTriggerCompletion);

    tasks->setFreeRunEnabled(false);
    tasks->setTotalDuration(totalDuration());

    elReadoutWorker->setOutputFile(outputFileFullPath() + ".dat");

    behavWorker->setSaveToFileEnabled(true);
    behavWorker->setOutputFile(outputFileFullPath());

    tasks->init();
    size_t frameCount = tasks->getMainTrigFreq() * totalDuration();
    worker->setFrameCount(frameCount);
    worker->setTimeout(2e6 / tasks->getMainTrigFreq());
    behavWorker->setFrameCount(frameCount);

    _startAcquisition();
    worker->start();
    writeRunParams();
    emit started(false);
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

BehavWorker *Optrode::getBehavWorker() const
{
    return behavWorker;
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
    out << "led_rate: " << tasks->getLEDFreq() << "\n";
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
        int Vn = 2048;
        double lineInterval = orca->getLineInterval();

        // inverse formula to obtain exposure time
        double temp = 1. / (1.01 * tasks->getMainTrigFreq()) - (Vn / 2 + 10) * lineInterval;
        double expTime = orca->setGetExposureTime(temp);

        double frameRate = qFloor(1. / (expTime + (Vn / 2 + 10) * lineInterval));

        if (tasks->getMainTrigFreq() != frameRate) {
            throw std::runtime_error("frame rate mismatch");
        }

        elReadoutWorker->setTotToBeRead(totalDuration() * tasks->getElectrodeReadoutRate());
        elReadoutWorker->setFreeRun(isFreeRunEnabled());

        tasks->setLEDFreq(frameRate / 2);

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

void Optrode::incrementCompleted()
{
    if (++completedJobs == 3) {
        logger->info("All jobs completed");
        stop();
    }
    logger->info(QString("Completed %1 jobs").arg(completedJobs));
};

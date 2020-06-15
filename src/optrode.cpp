#include <memory>

#include <QHistoryState>

#include <qtlab/core/logger.h>
#include <qtlab/hw/ni/nitask.h>

#include <Spinnaker.h>

#include "optrode.h"
#include "tasks.h"
#include "chameleoncamera.h"


static Logger *logger = getLogger("Optrod");

Optrode::Optrode(QObject *parent) : QObject(parent)
{
    behaviorCamera = new ChameleonCamera(this);
    tasks = new Tasks(this);
    postStimulation = 0;

    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &Optrode::stop);

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

    behaviorCamera->open(0);
    behaviorCamera->logDeviceInfo();

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

int Optrode::remainingTime() const
{
    return timer->remainingTime();
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
    _startAcquisition();
}

void Optrode::start()
{
    logger->info("Start acquisition");
    logger->info(QString("Baseline %1s, stimul %2s (%3 pulses), post %4s")
                 .arg(tasks->getShutterInitialDelay())
                 .arg(tasks->stimulationDuration())
                 .arg(tasks->getShutterPulseNPulses())
                 .arg(getPostStimulation()));
    logger->info(QString("Total duration: %1s").arg(totalDuration()));
    tasks->setFreeRunEnabled(false);
    timer->setInterval(totalDuration() * 1000);
    _startAcquisition();
    timer->start();
}

void Optrode::stop()
{
    if (!running)
        return;
    running = false;
    emit stopped();
    try {
        timer->stop();
        behaviorCamera->stopAcquisition();
        tasks->stop();
    } catch (std::runtime_error e) {
        onError(e.what());
    } catch (Spinnaker::Exception e) {
        onError(e.what());
    }
}

double Optrode::getPostStimulation() const
{
    return postStimulation;
}

void Optrode::_startAcquisition()
{
    running = true;
    try {
        behaviorCamera->startAcquisition();
        tasks->start();
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    } catch (Spinnaker::Exception e) {
        onError(e.what());
        return;
    }
    emit started(tasks->isFreeRunEnabled());
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
    logger->error(errMsg);
}

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
    readyState->addTransition(this, &Optrode::captureStarted, capturingState);
    capturingState->addTransition(this, &Optrode::stopped, readyState);

    QHistoryState *historyState = new QHistoryState(sm);

    QState *errorState = newState(STATE_ERROR, sm);
    errorState->addTransition(historyState);

    sm->addTransition(this, &Optrode::error, errorState);

    sm->start();
}

void Optrode::startFreeRun()
{
    logger->info("Start acquisition");
    tasks->setFreeRunEnabled(false);
    _startAcquisition();
}

void Optrode::stop()
{
    if (!running)
        return;
    running = false;
    emit stopped();
    try {
        behaviorCamera->stopAcquisition();
        tasks->stop();
    } catch (std::runtime_error e) {
        onError(e.what());
    }
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
    }
    emit captureStarted(tasks->isFreeRunEnabled());
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

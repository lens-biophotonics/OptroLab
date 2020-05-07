#include <memory>

#include <QHistoryState>

#include <qtlab/core/logger.h>

#include <qtlab/hw/aravis/aravis.h>
#include <qtlab/hw/aravis/camera.h>

#include "optrod.h"

static Logger *logger = getLogger("Optrod");

Optrod::Optrod(QObject *parent) : QObject(parent)
{
    behaviorCamera = new Aravis::Camera(this);
    setupStateMachine();
}

Optrod::~Optrod()
{
    uninitialize();
}

QState *Optrod::getState(const Optrod::MACHINE_STATE stateEnum)
{
    return stateMap[stateEnum];
}

void Optrod::initialize()
{
    emit initializing();
    bool ok = behaviorCamera->open(0);
    if (!ok) {
        onError("Cannot open behavior camera");
        return;
    }

    behaviorCamera->setPixelFormat("Mono8");

    emit initialized();
}

void Optrod::uninitialize()
{
    stop();
    behaviorCamera->close();
    Aravis::shutdown();
}

void Optrod::startFreeRun()
{
    freeRun = false;
    logger->info("Start acquisition");
    _startAcquisition();
}

void Optrod::stop()
{
    behaviorCamera->stopAcquisition();
    emit stopped();
}

Aravis::Camera *Optrod::getBehaviorCamera() const
{
    return behaviorCamera;
}

void Optrod::setupStateMachine()
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

    uninitState->addTransition(this, &Optrod::initializing, initializingState);
    initializingState->addTransition(this, &Optrod::initialized, readyState);
    initializingState->addTransition(this, &Optrod::error, uninitState);
    readyState->addTransition(this, &Optrod::captureStarted, capturingState);
    capturingState->addTransition(this, &Optrod::stopped, readyState);

    QHistoryState *historyState = new QHistoryState(sm);

    QState *errorState = newState(STATE_ERROR, sm);
    errorState->addTransition(historyState);

    sm->addTransition(this, &Optrod::error, errorState);

    sm->start();
}

void Optrod::_startAcquisition()
{
    behaviorCamera->startAcquisition();
    emit captureStarted();
}

Optrod &optrod()
{
    static auto instance = std::make_unique<Optrod>(nullptr);
    return *instance;
}

void Optrod::onError(const QString &errMsg)
{
    stop();
    emit error(errMsg);
    logger->error(errMsg);
}

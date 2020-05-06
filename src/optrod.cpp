#include <memory>

#include <QHistoryState>

#include <qtlab/hw/aravis/aravis.h>
#include <qtlab/hw/aravis/camera.h>

#include "optrod.h"

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
    bool ok = behaviorCamera->open(0);
    behaviorCamera->setPixelFormat("Mono8");
    if (!ok) {
        return;
    }

    behaviorCamera->startAcquisition();

    emit initialized();
}

void Optrod::uninitialize()
{
    behaviorCamera->close();
    Aravis::shutdown();
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

    QState *readyState = newState(STATE_READY, sm);
    QState *capturingState = newState(STATE_CAPTURING, sm);

    uninitState->addTransition(this, &Optrod::initialized, readyState);
    readyState->addTransition(this, &Optrod::captureStarted, capturingState);
    capturingState->addTransition(this, &Optrod::stopped, readyState);

    QHistoryState *historyState = new QHistoryState(sm);

    QState *errorState = newState(STATE_ERROR, sm);
    errorState->addTransition(historyState);

    sm->addTransition(this, &Optrod::error, errorState);

    sm->start();
}

Optrod &optrod()
{
    static auto instance = std::make_unique<Optrod>(nullptr);
    return *instance;
}

void Optrod::onError(const QString &errMsg)
{
    emit error(errMsg);
}

#ifndef OPTRODE_H
#define OPTRODE_H

#include <QObject>
#include <QStateMachine>

namespace Aravis {
class Camera;
}


class Optrode : public QObject
{
    Q_OBJECT
public:
    enum MACHINE_STATE {
        STATE_UNINITIALIZED,
        STATE_INITIALIZING,
        STATE_READY,
        STATE_CAPTURING,
        STATE_ERROR,

        STATE_FREERUN,
    };

    explicit Optrode(QObject *parent = nullptr);
    virtual ~Optrode();
    QState *getState(const MACHINE_STATE stateEnum);
    Aravis::Camera *getBehaviorCamera() const;

signals:
    void initializing() const;
    void initialized() const;
    void captureStarted() const;
    void stopped() const;
    void error(const QString) const;

public slots:
    void initialize();
    void uninitialize();
    void startFreeRun();
    void stop();

private:
    Aravis::Camera *behaviorCamera;
    QMap<MACHINE_STATE, QState *> stateMap;
    QStateMachine *sm = nullptr;
    bool freeRun;

    void setupStateMachine();
    void onError(const QString &errMsg);
    void _startAcquisition();
};

Optrode& optrode();

#endif // OPTRODE_H

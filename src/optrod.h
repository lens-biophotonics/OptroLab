#ifndef OPTROD_H
#define OPTROD_H

#include <QObject>
#include <QStateMachine>

namespace Aravis {
class Camera;
}


class Optrod : public QObject
{
    Q_OBJECT
public:
    enum MACHINE_STATE {
        STATE_UNINITIALIZED,
        STATE_READY,
        STATE_CAPTURING,
        STATE_ERROR,

        STATE_FREERUN,
    };

    explicit Optrod(QObject *parent = nullptr);
    virtual ~Optrod();
    QState *getState(const MACHINE_STATE stateEnum);
    Aravis::Camera *getBehaviorCamera() const;

signals:
    void initialized() const;
    void captureStarted() const;
    void stopped() const;
    void error(const QString) const;

public slots:
    void initialize();
    void uninitialize();

private:
    Aravis::Camera *behaviorCamera;
    QMap<MACHINE_STATE, QState *> stateMap;
    QStateMachine *sm = nullptr;

    void setupStateMachine();
    void onError(const QString &errMsg);
};

Optrod& optrod();

#endif // OPTROD_H

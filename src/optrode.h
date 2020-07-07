#ifndef OPTRODE_H
#define OPTRODE_H

#include <QObject>
#include <QStateMachine>
#include <QTimer>

#include <qtlab/hw/hamamatsu/orcaflash.h>

class ChameleonCamera;
class Tasks;
class ElReadoutWorker;
class BehavWorker;

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
    ChameleonCamera *getBehaviorCamera() const;
    Tasks *NITasks() const;
    bool isFreeRunEnabled() const;
    void setPostStimulation(double s);
    double getPostStimulation() const;
    double totalDuration() const;
    OrcaFlash *getOrca() const;

    QString getOutputDir() const;
    void setOutputDir(const QString &value);
    QString outputFileFullPath();

    QString getRunName() const;
    void setRunName(const QString &value);

    void writeRunParams(QString fileName);
    void writeRunParams();

    ElReadoutWorker *getElReadoutWorker() const;
    BehavWorker *getBehavWorker() const;

signals:
    void initializing() const;
    void initialized() const;
    void started(bool) const;
    void stopped() const;
    void error(const QString) const;
    void temp(double prova) const;

public slots:
    void initialize();
    void uninitialize();
    void startFreeRun();
    void start();
    void stop();

private:
    ChameleonCamera *behaviorCamera;
    Tasks *tasks;
    OrcaFlash *orca;
    QString outputPath;
    QString runName;
    ElReadoutWorker *elReadoutWorker;
    BehavWorker *behavWorker;

    QMap<MACHINE_STATE, QState *> stateMap;
    QStateMachine *sm = nullptr;
    bool running = false;
    double postStimulation;

    void setupStateMachine();
    void onError(const QString &errMsg);
    void _startAcquisition();
};

Optrode& optrode();

#endif // OPTRODE_H

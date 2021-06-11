#ifndef OPTRODE_H
#define OPTRODE_H

#include <QObject>
#include <QStateMachine>
#include <QTimer>

#include <qtlab/hw/hamamatsu/orcaflash.h>
#include <qtlab/hw/pi/pidevice.h>

class ChameleonCamera;
class Tasks;
class DDS;
class ElReadoutWorker;
class BehavWorker;
class SaveStackWorker;

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
    PIDevice *getZAxis() const;

    QString getOutputDir() const;
    void setOutputDir(const QString &value);
    QString outputFileFullPath();

    QString getRunName() const;
    void setRunName(const QString &value);

    void writeRunParams(QString fileName);
    void writeRunParams();

    ElReadoutWorker *getElReadoutWorker() const;
    BehavWorker *getBehavWorker() const;

    bool isSuccess();

    bool isSaveElectrodeEnabled() const;
    void setSaveElectrodeEnabled(bool enable);

    bool isSaveBehaviorEnabled() const;
    void setSaveBehaviorEnabled(bool enable);

    SaveStackWorker *getSSWorker() const;

    DDS *getDDS() const;

    void ddsMasterReset();

signals:
    void initializing() const;
    void initialized() const;
    void started(bool freeRun) const;
    void stopped() const;
    void error(const QString) const;
    void temp(double prova) const;
    void pleaseWait() const;

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
    PIDevice *zAxis;
    DDS *dds;
    QString outputPath;
    QString runName;
    ElReadoutWorker *elReadoutWorker;
    BehavWorker *behavWorker;
    SaveStackWorker *ssWorker;

    bool saveElectrodeEnabled = true, saveBehaviorEnabled = true;

    QMap<MACHINE_STATE, QState *> stateMap;
    QStateMachine *sm = nullptr;
    bool running = false;
    double postStimulation;
    int completedJobs;
    int successJobs;
    int nJobs;

    void setupStateMachine();
    void onError(const QString &errMsg);
    void _startAcquisition();
    void incrementCompleted(bool ok);
};

Optrode& optrode();

#endif // OPTRODE_H

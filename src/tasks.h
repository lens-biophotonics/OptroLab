#ifndef ELECTRODEREADOUT_H
#define ELECTRODEREADOUT_H

#include <QObject>

#include <qtlab/hw/ni/nitask.h>

class Tasks : public QObject
{
    Q_OBJECT
public:
    explicit Tasks(QObject *parent = nullptr);
    void init();

    NITask *electrodeReadout();

    QString getMainTrigPhysChan() const;
    void setMainTrigPhysChan(const QString &value);

    double getMainTrigFreq() const;
    void setMainTrigFreq(double value);

    QString getShutterPulseCounter() const;
    void setShutterPulseCounter(const QString &value);

    QString getShutterPulseTerm() const;
    void setShutterPulseTerm(const QString &value);

    double getShutterInitialDelay() const;
    void setShutterInitialDelay(double value);

    double getShutterPulseFrequency() const;
    void setShutterPulseFrequency(double value);

    double getShutterPulseDuty() const;
    void setShutterPulseDuty(double value);

    uInt64 getShutterPulseNPulses() const;
    void setShutterPulseNPulses(const uInt64 &value);

    QString getElectrodeReadoutPhysChan() const;
    void setElectrodeReadoutPhysChan(const QString &value);

    double getElectrodeReadoutRate() const;
    void setElectrodeReadoutRate(double value);

    bool isFreeRunEnabled() const;
    void setFreeRunEnabled(bool value);

signals:
    void started();
    void stopped();

public slots:
    void start();
    void stop();

private:
    NITask *mainTrigger;
    NITask *shutterPulse;
    NITask *elReadout;

    QString mainTrigPhysChan;
    double mainTrigFreq;

    QString shutterPulseCounter;
    QString shutterPulseTerm;
    double shutterInitialDelay = 10;
    double shutterPulseFrequency = 20;
    double shutterPulseDuty = 0.5;
    uInt64 shutterPulseNPulses = 20;

    QString electrodeReadoutPhysChan;
    double electrodeReadoutRate = 10000;

    bool needsInit;
    bool freeRunEnabled;
};

#endif // ELECTRODEREADOUT_H

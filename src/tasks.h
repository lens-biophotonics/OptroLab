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
    double getMainTrigNPulses() const;

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

    bool getShutterPulseEnabled() const;
    void setShutterPulseEnabled(bool value);

    QString getElectrodeReadoutPhysChan() const;
    void setElectrodeReadoutPhysChan(const QString &value);

    double getElectrodeReadoutRate() const;
    void setElectrodeReadoutRate(double value);

    bool isFreeRunEnabled() const;
    void setFreeRunEnabled(bool value);

    void setStimulationDuration(double s);
    double stimulationDuration();

    NITask *getElReadout() const;

    void setTotalDuration(double value);

    double getLEDFreq() const;
    void setLEDFreq(double value);

    QString getLED1PhysChan() const;
    void setLED1PhysChan(const QString &value);

    QString getLED2PhysChan() const;
    void setLED2PhysChan(const QString &value);

    QString getLED1Term() const;
    void setLED1Term(const QString &value);

    QString getLED2Term() const;
    void setLED2Term(const QString &value);

    QString getMainTrigTerm() const;
    void setMainTrigTerm(const QString &value);

    bool getLED1Enabled() const;
    void setLED1Enabled(bool value);

    bool getLED2Enabled() const;
    void setLED2Enabled(bool value);

    void setLEDdelay(double value);

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
    NITask *LED1, *LED2;

    QString mainTrigPhysChan, mainTrigTerm;
    double LEDFreq;
    double LEDdelay = 0;
    bool LED1Enabled = true, LED2Enabled = true;

    QString shutterPulseCounter;
    QString shutterPulseTerm;
    double shutterInitialDelay = 10;
    double shutterPulseFrequency = 20;
    double shutterPulseDuty = 0.5;
    uInt64 shutterPulseNPulses = 20;
    bool shutterPulseEnabled = true;

    QString LED1PhysChan, LED2PhysChan;
    QString LED1Term, LED2Term;

    QString electrodeReadoutPhysChan;
    double electrodeReadoutRate = 10000;

    double totalDuration = 10;

    bool freeRunEnabled;
    bool initialized = false;
};

#endif // ELECTRODEREADOUT_H

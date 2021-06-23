#ifndef ELECTRODEREADOUT_H
#define ELECTRODEREADOUT_H

#include <QObject>
#include <QPointF>

#include <qtlab/hw/ni/nitask.h>

class DDS;

class Tasks : public QObject
{
    Q_OBJECT
public:
    explicit Tasks(QObject *parent = nullptr);
    void init();

    NITask *electrodeReadout();

    double getMainTrigFreq() const;
    double getMainTrigNPulses() const;

    QString getStimulationTerm() const;
    void setStimulationTerm(const QString &value);

    double getStimulationInitialDelay() const;
    void setStimulationInitialDelay(double value);

    double getStimulationLowTime() const;
    void setStimulationLowTime(double value);

    double getStimulationHighTime() const;
    void setStimulationHighTime(double value);

    double getStimulationFrequency();

    uInt64 getStimulationNPulses() const;
    void setStimulationNPulses(const uInt64 &value);

    bool getStimulationEnabled() const;
    void setStimulationEnabled(bool value);

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

    bool getElectrodeReadoutEnabled() const;
    void setElectrodeReadoutEnabled(bool value);

    void ddsMasterReset();

    DDS *getDDS() const;

    QPointF getPoint() const;
    void setPoint(const QPointF &value);

    void setAODEnabled(bool enable);
    bool isAODEnabled() const;

    bool getContinuousStimulation() const;
    void setContinuousStimulation(bool value);

signals:
    void started();
    void stopped();
    void elReadoutStarted();

public slots:
    void start();
    void stop();
    void stopLEDs();

private:
    NITask *mainTrigger;
    NITask *stimulation;
    NITask *elReadout;
    NITask *LED;
    NITask *ddsSampClock;
    DDS *dds;
    QPointF point;

    QString mainTrigTerm;
    double LEDFreq;
    double LEDdelay = 0;
    bool LED1Enabled = true, LED2Enabled = true;
    bool electrodeReadoutEnabled = true;
    bool aodEnabled = false;

    QString stimulationTerm;
    double stimulationDelay = 10;
    double stimulationLowTime = 20;
    double stimulationHighTime = 0.5;
    uInt64 stimulationNPulses = 20;
    bool stimulationEnabled = true;
    bool continuousStimulation = false;

    QString LED1Term, LED2Term;

    QString electrodeReadoutPhysChan;
    double electrodeReadoutRate = 10000;

    double totalDuration = 10;

    bool freeRunEnabled;
    bool initialized = false;
};

#endif // ELECTRODEREADOUT_H

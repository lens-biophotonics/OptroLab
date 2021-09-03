#ifndef SAVESTACKWORKER_H
#define SAVESTACKWORKER_H

#include <QObject>
#include <QString>

class OrcaFlash;

class SaveStackWorker : public QObject
{
    Q_OBJECT
public:
    explicit SaveStackWorker(OrcaFlash *orca, QObject *parent = nullptr);

    double getTimeout() const; // ms
    void setTimeout(double value); // ms
    void setFrameCount(size_t count);
    size_t getFrameCount() const;
    void setOutputFile(const QString &fname);
    void signalTriggerCompletion();

    size_t getReadFrames() const;

    void requestStart();

    void setEnabledWriters(const uint &value);

    void stop();

signals:
    void error(QString msg = "");
    void captureCompleted(bool ok);
    void started();

// private signals
Q_SIGNALS:
#ifndef Q_MOC_RUN
private:
#endif
    void startRequested();

private:
    void start();
    bool stopped, triggerCompleted;
    double timeout;
    QString outputFile1, outputFile2;
    size_t frameCount, readFrames;
    OrcaFlash *orca;
    uint enabledWriters = 0b11;

    QString timeoutString(double delta, int i);
};

#endif // SAVESTACKWORKER_H

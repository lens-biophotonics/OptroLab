#ifndef SAVESTACKWORKER_H
#define SAVESTACKWORKER_H

#include <QThread>
#include <QString>

class OrcaFlash;

class SaveStackWorker : public QThread
{
    Q_OBJECT
public:
    explicit SaveStackWorker(OrcaFlash *orca, QObject *parent = nullptr);

    double getTimeout() const; // ms
    void setTimeout(double value); // ms
    void setFrameCount(int32_t count);
    void setOutputFile(const QString &fname);

signals:
    void error(QString msg = "");
    void captureCompleted();

protected:
    virtual void run();

private:
    bool stopped;
    double timeout;
    QString outputFile;
    int32_t frameCount;
    OrcaFlash *orca;

    QString timeoutString(double delta, int i);
};

#endif // SAVESTACKWORKER_H

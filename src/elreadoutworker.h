#ifndef ELREADOUTWORKER_H
#define ELREADOUTWORKER_H

#include <QVector>
#include <QTimer>
#include <QElapsedTimer>

#include <qtlab/hw/ni/nitask.h>

class ElReadoutWorker : public QObject
{
    Q_OBJECT
public:
    ElReadoutWorker(NITask *elReadoutTask, QObject *parent = nullptr);

    void setTotToBeRead(const size_t &value);

    void setFreeRun(bool value);

    void setOutputFile(const QString &value);

    size_t getTotRead() const;

    void setEmissionRate(double Hz);

    void setSaveToFileEnabled(bool value);

public slots:
    void start();
    void stop();
    void saveToFile(QString fullPath);

signals:
    void newData(const QVector<double> &buf);
    void acquisitionCompleted(bool ok);

private:
    void readOut();

    QTimer *timer;
    QElapsedTimer et;
    QVector<double> buf;
    QVector<double> mainBuffer;
    QString outputFile;
    double emissionRate = -1;

    bool freeRun = true;
    bool saveToFileEnabled = false;
    size_t totRead;
    size_t totToBeRead;
    size_t totEmitted;

    double readoutRate;

    NITask *task;
};

#endif // ELREADOUTWORKER_H

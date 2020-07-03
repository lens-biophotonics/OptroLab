#ifndef ELREADOUTWORKER_H
#define ELREADOUTWORKER_H

#include <QVector>
#include <QTimer>

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

public slots:
    void start();
    void stop();
    void saveToFile(QString fullPath);

signals:
    void newData(const QVector<double> &buf);
    void acquisitionCompleted();

private:
    void readOut();

    QTimer *timer;
    QVector<double> buf;
    QVector<double> mainBuffer;
    QString outputFile;

    bool freeRun = true;
    size_t totRead;
    size_t totToBeRead;

    NITask *task;
};

#endif // ELREADOUTWORKER_H

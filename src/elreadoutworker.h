#ifndef ELREADOUTWORKER_H
#define ELREADOUTWORKER_H

#include <QVector>
#include <QTimer>

class ElReadoutWorker : public QObject
{
    Q_OBJECT
public:
    ElReadoutWorker(QObject *parent = nullptr);

public slots:
    void start();
    void stop();

signals:
    void newData(const QVector<double> &buf);

private:
    void readOut();

    QTimer *timer;
    QVector<double> buf;
    size_t totRead;
};

#endif // ELREADOUTWORKER_H

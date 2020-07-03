#include <QFile>
#include <QTextStream>

#include "tasks.h"

#include "elreadoutworker.h"

#include <qtlab/core/logmanager.h>

#define INTERVALMSEC 150

ElReadoutWorker::ElReadoutWorker(NITask *elReadoutTask, QObject *parent)
    : QObject(parent), task(elReadoutTask)
{
    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, &ElReadoutWorker::readOut);
}

void ElReadoutWorker::start()
{
    totRead = 0;
    mainBuffer.clear();
    if (!freeRun) {
        mainBuffer.reserve(totToBeRead);
    }

    timer->start(INTERVALMSEC);
}

void ElReadoutWorker::stop()
{
    timer->stop();
    if (!freeRun) {
        saveToFile(outputFile);
    }
}

void ElReadoutWorker::saveToFile(QString fullPath)
{
    QFile outFile(fullPath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw std::runtime_error(
                  QString("Cannot open output file " + fullPath).toStdString());
    }

    QTextStream stream(&outFile);
    for (double v : mainBuffer) {
        stream << v << "\n";
    }

    outFile.close();
}

void ElReadoutWorker::readOut()
{
    double Hz = task->getSampClkRate();
    QVector<double> buf;
    buf.resize(Hz * INTERVALMSEC / 1000. * 3);
    int32 sampsPerChanRead;
    try {
        quint32 avail = task->getReadAvailSampPerChan();
        if (!avail)
            return;
        task->readAnalogF64(avail, INTERVALMSEC / 1000.,
                            DAQmx_Val_GroupByChannel,
                            buf.data(), buf.size(), &sampsPerChanRead);
    } catch (std::runtime_error e) {
        logManager().getLogger("ElReadoutWorker")->critical(e.what());
    }

#ifndef WITH_HARDWARE
    sampsPerChanRead = Hz * INTERVALMSEC / 1000.;
    buf = QVector<double>(sampsPerChanRead, rand());
#endif
    if (!sampsPerChanRead) {
        return;
    }
    buf.resize(sampsPerChanRead);

    if (!freeRun && (totRead + sampsPerChanRead >= totToBeRead)) {
        buf.resize(totToBeRead - totRead);
        mainBuffer.append(buf);
        timer->stop();
        emit acquisitionCompleted();
    }
    else {
        mainBuffer.append(buf);
    }

    totRead += buf.size();
    emit newData(buf);
}

size_t ElReadoutWorker::getTotRead() const
{
    return totRead;
}

void ElReadoutWorker::setOutputFile(const QString &value)
{
    outputFile = value;
}

void ElReadoutWorker::setFreeRun(bool value)
{
    freeRun = value;
}

void ElReadoutWorker::setTotToBeRead(const size_t &value)
{
    totToBeRead = value;
}

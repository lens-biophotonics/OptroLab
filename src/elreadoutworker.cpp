#include <QFile>
#include <QTextStream>

#include "tasks.h"

#include "elreadoutworker.h"

#include <qtlab/core/logmanager.h>

#define INTERVALMSEC 100

static Logger *logger = logManager().getLogger("ElReadoutWorker");

ElReadoutWorker::ElReadoutWorker(NITask *elReadoutTask, QObject *parent)
    : QObject(parent), task(elReadoutTask)
{
    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, &ElReadoutWorker::readOut);
}

void ElReadoutWorker::start()
{
    totRead = 0;
    totEmitted = 0;
    mainBuffer.clear();
    if (!freeRun) {
        mainBuffer.reserve(totToBeRead);
    }

    try {
        readoutRate = task->getSampClkRate();
    } catch (std::runtime_error e) {
        logger->critical(e.what());
        return;
    }

    timer->start(INTERVALMSEC);
    et.restart();
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
    int32 sampsPerChanRead;

#ifdef WITH_HARDWARE
    buf.resize(readoutRate * INTERVALMSEC / 1000. * 3);
    try {
        quint32 avail = task->getReadAvailSampPerChan();
        if (!avail)
            return;
        task->readAnalogF64(avail, INTERVALMSEC / 1000.,
                            DAQmx_Val_GroupByChannel,
                            buf.data(), buf.size(), &sampsPerChanRead);
    } catch (std::runtime_error e) {
        logger->critical(e.what());
    }
#else
    sampsPerChanRead = readoutRate * INTERVALMSEC / 1000.;
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
        emit acquisitionCompleted(true);
    }
    else {
        mainBuffer.append(buf);
    }

    totRead += buf.size();

    if (emissionRate <= 0) {
        emit newData(buf);
        return;
    }

    QVector<double> temp;
    size_t tempSize;
    double elapsed;
    if (timer->isActive())
        elapsed = et.elapsed() / 1000.;
    else
        elapsed = totToBeRead / readoutRate;
    tempSize = elapsed * emissionRate - totEmitted;
    temp.resize(tempSize);

    size_t stride = readoutRate / emissionRate;
    auto it = temp.begin();
    auto buf_it = buf.begin();
    while (it < temp.end()) {
        *it = *buf_it;
        it++;
        buf_it += stride;
    }

    totEmitted += temp.size();
    emit newData(temp);
}

/**
 * @brief Force a lower rate for data emission
 * @param Hz
 *
 * This is used to limit the amount of data sent for plotting. Even if the real sample rate is
 * 10 kHz, by setting this value data will be emitted (newData() signal) at a lower rate as if it
 * were acquired at that rate.
 */

void ElReadoutWorker::setEmissionRate(double Hz)
{
    emissionRate = Hz;
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

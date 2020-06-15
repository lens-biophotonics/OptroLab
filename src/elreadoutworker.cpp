#include "optrode.h"
#include "tasks.h"

#include "elreadoutworker.h"

#define INTERVALMSEC 150

ElReadoutWorker::ElReadoutWorker(QObject *parent) : QObject(parent)
{
    timer = new QTimer(this);

    connect(&optrode(), &Optrode::started, this, [ = ](){
        start();
        totRead = 0;
    });
    connect(&optrode(), &Optrode::stopped, this, &ElReadoutWorker::stop);
    connect(timer, &QTimer::timeout, this, &ElReadoutWorker::readOut);
}

void ElReadoutWorker::start()
{
    timer->start(INTERVALMSEC);
}

void ElReadoutWorker::stop()
{
    timer->stop();
}

void ElReadoutWorker::readOut()
{
    NITask *t = optrode().NITasks()->electrodeReadout();
    double Hz = optrode().NITasks()->getElectrodeReadoutRate();
    QVector<double> buf;
    buf.resize(Hz * INTERVALMSEC / 1000. * 3);
    int32 sampsPerChanRead;
    t->readAnalogF64(-1, INTERVALMSEC / 1000., DAQmx_Val_GroupByChannel,
                     buf.data(), buf.size(), &sampsPerChanRead);

#ifndef WITH_HARDWARE
    sampsPerChanRead = Hz * INTERVALMSEC / 1000.;
    buf = QVector<double>(sampsPerChanRead, rand());
#endif
    buf.resize(sampsPerChanRead);

    totRead += sampsPerChanRead;

    emit newData(buf);
}

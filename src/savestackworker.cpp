#ifndef WITH_HARDWARE
#include <unistd.h>
#endif

#include <QDir>

#include <qtlab/io/tiffwriter.h>
#include <qtlab/core/logger.h>
#include <qtlab/hw/hamamatsu/orcaflash.h>

#include "savestackworker.h"

static Logger *logger = getLogger("SaveStackWorker");

using namespace DCAM;

SaveStackWorker::SaveStackWorker(OrcaFlash *orca, QObject *parent)
    : QObject(parent), orca(orca)
{
    frameCount = 0;
    connect(this, &SaveStackWorker::startRequested, this, &SaveStackWorker::start);

    connect(orca, &OrcaFlash::stopped, this, [ = ] () {
        stopped = true;
    }, Qt::DirectConnection);
}

void SaveStackWorker::start()
{
    emit started();
    readFrames = 0;
    void *buf;
    size_t width = 256;
    size_t height = 256;

    triggerCompleted = false;

    logger->info(QString("Total number of frames: %1").arg(frameCount));

#ifdef WITH_HARDWARE
    const int32_t nFramesInBuffer = orca->nFramesInBuffer();
    QVector<qint64> timeStamps(frameCount, 0);
#else
    size_t n = 2 * width * height;
    buf = malloc(n);
#endif

    stopped = false;

    TIFFWriter *writers[2];

    if(enabledWriters == 0b11) {
        writers[0] = new TIFFWriter(outputFile1, true);
        writers[1] = new TIFFWriter(outputFile2, true);
    }
    else if (enabledWriters == 0b01) {
        writers[0] = new TIFFWriter(outputFile1, true);
        writers[1] = writers[0];
    }
    else if (enabledWriters == 0b10) {
        writers[0] = new TIFFWriter(outputFile2, true);
        writers[1] = writers[0];
    } else {
        writers[0] = new TIFFWriter(outputFile1.chopped(10).append(".tiff"), true);
        writers[1] = writers[0];
    }


    while (!stopped && readFrames < frameCount) {
#ifdef WITH_HARDWARE
        int32_t frame = readFrames % nFramesInBuffer;
        int32_t frameStamp = -1;

        DCAM_TIMESTAMP timeStamp;

        qint32 mask = DCAMWAIT_CAPEVENT_FRAMEREADY | DCAMWAIT_CAPEVENT_STOPPED;
        qint32 event = DCAMWAIT_CAPEVENT_FRAMEREADY;

        if (!triggerCompleted) {
            try {
                event = orca->wait(1000, mask);
            }
            catch (std::runtime_error e) {
                continue;
            }
        }

        switch (event) {
        case DCAMWAIT_CAPEVENT_FRAMEREADY:
            try {
                orca->lockFrame(frame, &buf, &frameStamp, &timeStamp);
                timeStamps[readFrames] = timeStamp.sec * 1e6 + timeStamp.microsec;
                if (readFrames != 0) {
                    double delta = double(timeStamps[readFrames]) - double(timeStamps[readFrames - 1]);
                    if (abs(delta) > timeout) {
                        logger->critical(timeoutString(delta, readFrames));
                    }
                    else if (abs(delta) > timeout * 0.75 || abs(delta) < timeout * 0.25) {
                        logger->warning(timeoutString(delta, readFrames));
                    }
                }
            }
            catch (std::runtime_error) {
                continue;
            }
#else
            orca->copyLastFrame(buf, n);
            usleep(20000);
#endif

            writers[readFrames % 2]->write((quint16 *)buf, width, height, 1);
            readFrames++;
#ifdef WITH_HARDWARE
            break;
        case DCAMERR_TIMEOUT:
            logger->warning(QString("Camera %1 timeout").arg(orca->getCameraIndex()));
            break;
        default:
            break;
        }  // switch
#endif
    }  // while

#ifdef WITH_HARDWARE
#else
    free(buf);
#endif

    delete writers[0];
    if(enabledWriters == 0b11) {
        delete writers[1];
    }

    emit captureCompleted(readFrames == frameCount);
    QString msg = QString("Saved %1/%2 frames").arg(readFrames).arg(frameCount);
    if (readFrames != frameCount) {
        logger->warning(msg);
    } else {
        logger->info(msg);
    }
}

void SaveStackWorker::setEnabledWriters(const uint &value)
{
    enabledWriters = value;
}

size_t SaveStackWorker::getReadFrames() const
{
    return readFrames;
}

void SaveStackWorker::requestStart()
{
    emit startRequested();
}

size_t SaveStackWorker::getFrameCount() const
{
    return frameCount;
}

QString SaveStackWorker::timeoutString(double delta, int i)
{
    return QString("Camera %1 timeout by %2 ms at frame %3 (timeout: %4)")
           .arg(orca->getCameraIndex())
           .arg(delta * 1e-3).arg(i + 1)
           .arg(timeout);
}

void SaveStackWorker::setFrameCount(size_t count)
{
    frameCount = count;
}

void SaveStackWorker::setOutputFile(const QString &fname)
{
    outputFile1 = fname + "_led1.tiff";
    outputFile2 = fname + "_led2.tiff";
}

void SaveStackWorker::signalTriggerCompletion()
{
    triggerCompleted = true;
}

double SaveStackWorker::getTimeout() const
{
    return timeout;
}

void SaveStackWorker::setTimeout(double value)
{
    timeout = value;
}

#include <QDir>

#include <qtlab/io/tiffwriter.h>
#include <qtlab/core/logger.h>
#include <qtlab/hw/hamamatsu/orcaflash.h>

#include "savestackworker.h"

static Logger *logger = getLogger("SaveStackWorker");

using namespace DCAM;

SaveStackWorker::SaveStackWorker(OrcaFlash *orca, QObject *parent)
    : QThread(parent), orca(orca)
{
    frameCount = 0;

    connect(orca, &OrcaFlash::captureStarted, this, [ = ](){
        start();
    });
}

void SaveStackWorker::run()
{
    int i = 0;
    void *buf;
    size_t width = 512;
    size_t height = 512;

#ifdef WITH_HARDWARE
    const int32_t nFramesInBuffer = orca->nFramesInBuffer();
    QVector<qint64> timeStamps(frameCount, 0);
#else
    size_t n = 2 * width * height;
    buf = malloc(n);
#endif

    stopped = false;

    connect(orca, &OrcaFlash::stopped, this, [ = ] () {
        stopped = true;
    });

    TIFFWriter writer1(outputFile1, true);
    TIFFWriter writer2(outputFile2, true);

    while (!stopped && i < frameCount) {
#ifdef WITH_HARDWARE
        int32_t frame = i % nFramesInBuffer;
        int32_t frameStamp = -1;

        DCAM_TIMESTAMP timeStamp;

        qint32 mask = DCAMWAIT_CAPEVENT_FRAMEREADY | DCAMWAIT_CAPEVENT_STOPPED;
        qint32 event;
        try {
            event = orca->wait(1000, mask);
        }
        catch (std::runtime_error e) {
            continue;
        }

        switch (event) {
        case DCAMWAIT_CAPEVENT_FRAMEREADY:
            try {
                orca->lockFrame(frame, &buf, &frameStamp, &timeStamp);
                timeStamps[i] = timeStamp.sec * 1e6 + timeStamp.microsec;
                if (i != 0) {
                    double delta = double(timeStamps[i]) - double(timeStamps[i - 1]);
                    if (abs(delta) > timeout) {
                        logger->critical(timeoutString(delta, i));
                    }
                    else if (abs(delta) > timeout * 0.75 || abs(delta) < timeout * 0.25) {
                        logger->warning(timeoutString(delta, i));
                    }
                }
            }
            catch (std::runtime_error) {
                continue;
            }
#else
            orca->copyLastFrame(buf, n);
            msleep(20);
#endif
            if (i % 2 == 0) {
                writer1.write((quint16 *)buf, width, height, 1);
            } else {
                writer2.write((quint16 *)buf, width, height, 1);
            }
            i++;
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

    emit captureCompleted();
    logger->info(QString("Saved %1 frames").arg(i));
}

QString SaveStackWorker::timeoutString(double delta, int i)
{
    return QString("Camera %1 timeout by %2 ms at frame %3 (timeout: %4)")
           .arg(orca->getCameraIndex())
           .arg(delta * 1e-3).arg(i + 1)
           .arg(timeout);
}

void SaveStackWorker::setFrameCount(int32_t count)
{
    frameCount = count;
}

void SaveStackWorker::setOutputFile(const QString &fname)
{
    outputFile1 = fname + "_led1.tiff";
    outputFile2 = fname + "_led2.tiff";
}

double SaveStackWorker::getTimeout() const
{
    return timeout;
}

void SaveStackWorker::setTimeout(double value)
{
    timeout = value;
}

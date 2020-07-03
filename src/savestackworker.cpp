#include <QVector>
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
}

void SaveStackWorker::run()
{
    int i = 0;
    void *buf;
    size_t width = 2048;
    size_t height = 2048;
    size_t n = 2 * width * height;

#ifdef WITH_HARDWARE
    const int32_t nFramesInBuffer = orca->nFramesInBuffer();
    QVector<qint64> timeStamps(frameCount, 0);
#else
    buf = malloc(n);
#endif

    stopped = false;

    connect(orca, &OrcaFlash::stopped, this, [ = ] () {
        stopped = true;
    });

    TIFFWriter writer(outputFile, true);

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
        catch (std::runtime_error) {
        }

        switch (event) {
        case DCAMWAIT_CAPEVENT_FRAMEREADY:
            try {
                orca->lockFrame(frame, &buf, &frameStamp, &timeStamp);
                timeStamps[i] = timeStamp.sec * 1e6 + timeStamp.microsec;
                if (i != 0) {
                    double delta = double(timeStamps[i]) - double(timeStamps[i - 1]);
                    if (abs(delta) > timeout) {
                        logger->error(QString("Camera %1 timeout by %2 ms at frame %3")
                                      .arg(orca->getCameraIndex()).arg(delta * 1e-3).arg(i + 1));
                    }
                    else if (abs(delta) > timeout * 0.75 || abs(delta) < timeout * 0.25) {
                        logger->warning(QString("Camera %1 timeout by %2 ms at frame %3")
                                        .arg(orca->getCameraIndex()).arg(delta * 1e-3).arg(i + 1));
                    }
                }
            }
            catch (std::runtime_error) {
                continue;
            }
            writer.write((quint16 *)buf, width, height, 1);
            i++;
            break;
        case DCAMERR_TIMEOUT:
            logger->warning(QString("Camera %1 timeout").arg(orca->getCameraIndex()));
            break;
        default:
            break;
        }
#else
        orca->copyLastFrame(buf, n);
        msleep(20);
        writer.write((quint16 *)buf, width, height, 1);
        i++;
#endif
    }

#ifdef WITH_HARDWARE
#else
    free(buf);
#endif

    emit captureCompleted();
    logger->info(QString("Saved %1 frames").arg(i));
}

void SaveStackWorker::setFrameCount(int32_t count)
{
    frameCount = count;
}

void SaveStackWorker::setOutputFile(const QString &fname)
{
    outputFile = fname;
}

double SaveStackWorker::getTimeout() const
{
    return timeout;
}

void SaveStackWorker::setTimeout(double value)
{
    timeout = value;
}

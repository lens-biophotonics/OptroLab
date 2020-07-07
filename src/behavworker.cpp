#include <QImage>
#include <QElapsedTimer>

#include <Spinnaker.h>
#include <SpinVideo.h>

#include <qtlab/core/logmanager.h>

#include "behavworker.h"
#include "chameleoncamera.h"

using namespace Spinnaker;

static Logger *logger = logManager().getLogger("BehavDispWorker");


BehavWorker::BehavWorker(ChameleonCamera *camera,
                         QObject *parent) : QThread(parent)
{
    this->camera = camera;

    connect(camera, &ChameleonCamera::acquisitionStarted, this, [ = ](){
        stop = false;
        start();
    });
    connect(camera, &ChameleonCamera::acquisitionStopped, this, [ = ](){
        stop = true;
    }, Qt::DirectConnection);
}

void BehavWorker::run()
{
    QImage qimg(camera->imageSize(), QImage::Format_Indexed8);
    for (int i = 0; i < 256; ++i)  // populate index
        qimg.setColor(i, qRgb(i, i, i));

    Video::SpinVideo video;
    video.SetMaximumFileSize(0);

    if (saveToFileEnabled) {
        Video::H264Option option;
        option.frameRate = frameRate;
        option.bitrate = 1000000;
        option.width = static_cast<unsigned int>(qimg.width());
        option.height = static_cast<unsigned int>(qimg.height());

        video.Open(outputFile.toStdString().c_str(), option);
    }

    QElapsedTimer timer;
    timer.start();

    size_t i = 0;

    while (true) {
        if (stop) {
            break;
        }
        ImagePtr img;
        try {
            img = camera->getNextImage(5000);
        }
        catch (Spinnaker::Exception e) {
            logger->warning(e.what());
            continue;
        }
        if (!img)
            continue;

        if (saveToFileEnabled) {
            video.Append(img);
            i++;
        }

        if (!img.IsValid())
            continue;

        if (timer.elapsed() >= 40) {  // no more than 25 fps
            memcpy(qimg.bits(), img->GetData(), img->GetBufferSize());
            logger->info(QString("%1").arg(img->GetBufferSize()));
            emit newImage(QPixmap::fromImage(qimg));
            timer.restart();
        }
        img->Release();
    }

    if (saveToFileEnabled) {
        try {
            video.Close();
        }
        catch (Spinnaker::Exception e) {
            logger->warning(e.what());
        }
    }
}

void BehavWorker::setFrameRate(double value)
{
    frameRate = value;
}

void BehavWorker::setOutputFile(const QString &value)
{
    outputFile = value;
}

void BehavWorker::setSaveToFileEnabled(bool value)
{
    saveToFileEnabled = value;
}

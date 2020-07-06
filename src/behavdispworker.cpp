#include <QImage>

#include <Spinnaker.h>

#include <qtlab/core/logmanager.h>

#include "behavdispworker.h"
#include "chameleoncamera.h"

using namespace Spinnaker;

static Logger *logger = logManager().getLogger("BehavDispWorker");


BehavDispWorker::BehavDispWorker(ChameleonCamera *camera,
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

void BehavDispWorker::run()
{
    QImage qimg(camera->imageSize(), QImage::Format_Indexed8);

    for (int i = 0; i < 256; ++i)  // populate index
        qimg.setColor(i, qRgb(i, i, i));

    while (true) {
        if (stop) {
            return;
        }
        ImagePtr imgMono, img;
        try {
            img = camera->getNextImage(5000);
        }
        catch (Spinnaker::Exception e) {
            logger->warning(e.what());
            continue;
        }
        if (!img)
            continue;
        imgMono = img->Convert(PixelFormat_Mono8, HQ_LINEAR);
        img->Release();

        memcpy(qimg.bits(), imgMono->GetData(), imgMono->GetBufferSize());
        emit newImage(QPixmap::fromImage(qimg));
    }
}

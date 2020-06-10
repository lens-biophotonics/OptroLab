#include <QImage>

#include <Spinnaker.h>

#include "behavdispworker.h"
#include "chameleoncamera.h"

using namespace Spinnaker;

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
    while (true) {
        if (stop) {
            return;
        }
        ImagePtr imgMono;
        ImagePtr img = camera->getNextImage(5000);
        if (!img)
            continue;
        imgMono = img->Convert(PixelFormat_Mono8, HQ_LINEAR);
        img->Release();

        QImage qimg(imgMono->GetWidth(), imgMono->GetHeight(), QImage::Format_Indexed8);

        for (int i = 0; i < 256; ++i)  // populate index
            qimg.setColor(i, qRgb(i, i, i));
        memcpy(qimg.bits(), imgMono->GetData(), imgMono->GetBufferSize());
        emit newImage(QPixmap::fromImage(qimg));
    }
}

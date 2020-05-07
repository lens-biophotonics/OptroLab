#include <QImage>

#include <qtlab/hw/aravis/camera.h>

#include "behavdispworker.h"

BehavDispWorker::BehavDispWorker(Aravis::Camera *camera,
                                 QObject *parent) : QThread(parent)
{
    this->camera = camera;

    connect(camera, &Aravis::Camera::captureStarted, this, [ = ](){
        start();
    });
    connect(camera, &Aravis::Camera::stopped, this, [ = ](){
        stop = true;
    }, Qt::DirectConnection);
}

void BehavDispWorker::run()
{
    stop = false;
    const Aravis::Camera::Buffer *lb = camera->getLastBuffer();
    QImage img(camera->pictureSize(), QImage::Format_Indexed8);
    for (int i = 0; i < 256; ++i)  // populate index
        img.setColor(i, qRgb(i, i, i));
    while (true) {
        msleep(40);
        if (stop) {
            return;
        }
        if (!lb->data)
            continue;
        memcpy(img.bits(), lb->data, lb->size);
        emit newImage(QPixmap::fromImage(img));
    }
}

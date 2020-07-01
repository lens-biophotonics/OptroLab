#include "displayworker.h"

#include <qtlab/hw/hamamatsu/orcaflash.h>
#include <qtlab/widgets/cameradisplay.h>
#include <qtlab/widgets/cameraplot.h>

#define BUFSIZE (2048 * 2048)

DisplayWorker::DisplayWorker(OrcaFlash *camera, CameraDisplay *cd, QObject *parent)
    : QThread(parent)
{
    qRegisterMetaType<size_t>("size_t");

    buf = new quint16[BUFSIZE];
    bufd = new double[BUFSIZE];

    orca = camera;
    this->cd = cd;

    connect(orca, &OrcaFlash::captureStarted, this, [ = ](){
        start();
    });
    connect(orca, &OrcaFlash::stopped, this, [ = ](){
        running = false;
    });
}

DisplayWorker::~DisplayWorker()
{
    delete[] buf;
    delete[] bufd;
}

void DisplayWorker::run()
{
    running = true;
    while (true) {
        msleep(250);
        if (!running) {
            break;
        }
        try {
            orca->copyLastFrame(buf, BUFSIZE * sizeof(quint16));
        }
        catch (std::exception) {
            continue;
        }
        for (int i = 0; i < BUFSIZE; ++i) {
            bufd[i] = buf[i];
        }
        emit newImage(bufd, BUFSIZE);
    }
}
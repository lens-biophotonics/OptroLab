#include "displayworker.h"

#include <qtlab/hw/hamamatsu/orcaflash.h>
#include <qtlab/widgets/cameradisplay.h>
#include <qtlab/widgets/cameraplot.h>

#include "optrode.h"
#include "tasks.h"

#define BUFSIZE (512 * 512)

using namespace DCAM;


DisplayWorker::DisplayWorker(OrcaFlash *camera, QObject *parent)
    : QThread(parent)
{
    displayWhat = DISPLAY_ALL;
    qRegisterMetaType<size_t>("size_t");

    buf = new quint16[BUFSIZE];
    bufd = new double[BUFSIZE];

    orca = camera;

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
    int triggerPeriod_ms = 1 / optrode().NITasks()->getMainTrigFreq() * 1000;

    int skipFrames = qMax(40, triggerPeriod_ms) / triggerPeriod_ms;  // 40ms is 25 fps
    while (true) {
        msleep(skipFrames * triggerPeriod_ms);
        if (displayWhat != DISPLAY_ALL) {
            msleep(triggerPeriod_ms);
        }

        int32_t frameStamp = -1;
        int exceptionCounter = 0;

        int i = 0;
        while (true) {
            if (!running) {
                return;
            }

            try {
                orca->copyFrame(buf, BUFSIZE * sizeof(quint16), -1, &frameStamp);
            }
            catch (std::exception) {
                exceptionCounter++;
                continue;
            }

            if (displayWhat == DISPLAY_LED1 && (frameStamp % 2 == 0)) {
                break;
            } else if (displayWhat == DISPLAY_LED2 && (frameStamp % 2 == 1)) {
                break;
            } else if (displayWhat == DISPLAY_ALL) {
                break;
            }

            i++;
            msleep(triggerPeriod_ms);
        }

        if (frameStamp == -1) {
            continue;
        }

        for (int i = 0; i < BUFSIZE; ++i) {
            bufd[i] = buf[i];
        }
        emit newImage(bufd, BUFSIZE);
    }
}

DisplayWorker::DISPLAY_WHAT DisplayWorker::getDisplayWhat() const
{
    return displayWhat;
}

void DisplayWorker::setDisplayWhat(const DISPLAY_WHAT &value)
{
    displayWhat = value;
}

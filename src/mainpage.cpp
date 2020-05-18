#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>

#include <qtlab/hw/aravis/camera.h>
#include <qtlab/widgets/aspectratiowidget.h>
#include <qtlab/widgets/pixmapwidget.h>

#include "controlswidget.h"
#include "behavdispworker.h"

#include "optrode.h"

#include "mainpage.h"

#define SETTING_BEHAVCAM_FLIPLR "behavCamFlipLr"
#define SETTING_BEHAVCAM_FLIPUD "behavCamFlipUd"
#define SETTING_BEHAVCAM_ROTSTEP "behavCamRotStep"

MainPage::MainPage(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

MainPage::~MainPage()
{
    saveSettings();
}

void MainPage::setupUi()
{
    QSettings settings;

    ControlsWidget *cw = new ControlsWidget();

    pmw = new PixmapWidget();
    pmw->setPixmap(QPixmap(":/res/lemmling-Simple-cartoon-mouse.svg"));

    if (settings.value(SETTING_BEHAVCAM_FLIPLR).toBool())
        pmw->fliplr();
    if (settings.value(SETTING_BEHAVCAM_FLIPUD).toBool())
        pmw->flipud();
    pmw->setRotationStep(settings.value(SETTING_BEHAVCAM_ROTSTEP).toUInt());

    QHBoxLayout * myLayout = new QHBoxLayout(this);
    myLayout->addWidget(pmw);
    myLayout->addWidget(cw);

    setLayout(myLayout);

    BehavDispWorker *worker = new BehavDispWorker(optrode().getBehaviorCamera());
    connect(worker, &BehavDispWorker::newImage,
            pmw, &PixmapWidget::setPixmap);
}

void MainPage::saveSettings()
{
    QSettings settings;
    settings.setValue(SETTING_BEHAVCAM_FLIPLR, pmw->isFlippedlr());
    settings.setValue(SETTING_BEHAVCAM_FLIPUD, pmw->isFlippedud());
    settings.setValue(SETTING_BEHAVCAM_ROTSTEP, pmw->getRotationStep());
}

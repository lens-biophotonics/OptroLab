#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QTimer>
#include <QtSvg/QSvgRenderer>

#include <qwt_plot_marker.h>

#include <qtlab/widgets/aspectratiowidget.h>
#include <qtlab/widgets/pixmapwidget.h>
#include <qtlab/widgets/timeplot.h>
#include <qtlab/widgets/cameradisplay.h>

#include "controlswidget.h"
#include "behavdispworker.h"

#include "optrode.h"
#include "chameleoncamera.h"
#include "settings.h"

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
    QSettings sett;

    pmw = new PixmapWidget();
    QSvgRenderer renderer(QString(":/res/lemmling-Simple-cartoon-mouse.svg"));
    QPixmap pm(1000, 1000);
    pm.fill(Qt::transparent);
    QPainter painter(&pm);
    renderer.render(&painter, pm.rect());

    pmw->setPixmap(pm.scaled(1280, 1024, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    if (sett.value(SETTING_BEHAVCAM_FLIPLR).toBool())
        pmw->fliplr();
    if (sett.value(SETTING_BEHAVCAM_FLIPUD).toBool())
        pmw->flipud();
    pmw->setRotationStep(sett.value(SETTING_BEHAVCAM_ROTSTEP).toUInt());

    BehavDispWorker *worker = new BehavDispWorker(optrode().getBehaviorCamera());
    connect(worker, &BehavDispWorker::newImage, pmw, &PixmapWidget::setPixmap);

    TimePlot *timePlot = new TimePlot();
    QTimer *timer = new QTimer();
    connect(timer, &QTimer::timeout, this, [ = ](){
        timePlot->appendPoint(rand());
    });
    timer->start(20);
    timePlot->setSamplingRate(50);

    QwtPlotMarker *startMarker = new QwtPlotMarker();
    startMarker->setLineStyle((QwtPlotMarker::LineStyle)(QwtPlotMarker::VLine));
    startMarker->setLinePen(Qt::red);
    startMarker->attach(timePlot);
    startMarker->setVisible(false);

    QwtPlotMarker *endMarker = new QwtPlotMarker();
    endMarker->setLineStyle((QwtPlotMarker::LineStyle)(QwtPlotMarker::VLine));
    endMarker->setLinePen(Qt::red);
    endMarker->attach(timePlot);
    endMarker->setVisible(false);

    connect(&optrode(), &Optrode::started, this, [ = ](bool freeRun){
        double Hz = settings().value(SETTINGSGROUP_ELREADOUT, SETTING_FREQ).toDouble();
        timePlot->setSamplingRate(Hz);

        if (freeRun) {
            startMarker->setVisible(false);
            endMarker->setVisible(false);
            return;
        }

        startMarker->setVisible(true);
        endMarker->setVisible(true);
        startMarker->setValue(10 * Hz, 0);
        endMarker->setValue(20 * Hz, 0);
    });

    QHBoxLayout *hLayout = new QHBoxLayout();
    CameraDisplay *camDisplay = new CameraDisplay(this);
    camDisplay->setLUTPath(
        settings().value(SETTINGSGROUP_OTHERSETTINGS, SETTING_LUTPATH)
        .toString());
    AspectRatioWidget *arw = new AspectRatioWidget(camDisplay, 1);
    hLayout->addWidget(arw, 3);
    hLayout->addWidget(pmw, 4);
    hLayout->addWidget(new ControlsWidget());

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(hLayout, 5);
    vLayout->addWidget(timePlot, 3);

    setLayout(vLayout);
}

void MainPage::saveSettings()
{
    QSettings settings;
    settings.setValue(SETTING_BEHAVCAM_FLIPLR, pmw->isFlippedlr());
    settings.setValue(SETTING_BEHAVCAM_FLIPUD, pmw->isFlippedud());
    settings.setValue(SETTING_BEHAVCAM_ROTSTEP, pmw->getRotationStep());
}

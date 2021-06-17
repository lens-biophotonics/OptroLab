#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QRadioButton>
#include <QtSvg/QSvgRenderer>

#include <qwt_plot_marker.h>

#include <qtlab/widgets/aspectratiowidget.h>
#include <qtlab/widgets/pixmapwidget.h>
#include <qtlab/widgets/timeplot.h>
#include <qtlab/widgets/cameradisplay.h>
#include <qtlab/widgets/cameraplot.h>
#include <qtlab/widgets/customspinbox.h>

#include "controlswidget.h"

#include "behavworker.h"
#include "elreadoutworker.h"
#include "displayworker.h"

#include "optrode.h"
#include "tasks.h"
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
    const Settings s = settings();

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

    connect(optrode().getBehavWorker(), &BehavWorker::newImage, pmw, &PixmapWidget::setPixmap);

    TimePlot *timePlot = new TimePlot();

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

    qRegisterMetaType<QVector<double>>("QVector<double>");
    connect(optrode().getElReadoutWorker(), &ElReadoutWorker::newData,
            timePlot, &TimePlot::appendPoints);

    double sr = 25;  // limit plotting to 25 Hz

    optrode().getElReadoutWorker()->setEmissionRate(sr);

    connect(&optrode(), &Optrode::started, this, [ = ](bool freeRun){
        Tasks *t = optrode().NITasks();
        timePlot->clear();
        timePlot->setSamplingRate(sr);
        timePlot->setBufSize(freeRun ? 22.0 : optrode().totalDuration());

        startMarker->setVisible(!freeRun);
        endMarker->setVisible(!freeRun);

        startMarker->setValue(sr * t->getStimulationInitialDelay(), 0);
        endMarker->setValue(sr * (t->getStimulationInitialDelay() + t->stimulationDuration()), 0);
    });

    DisplayWorker *dispWorker = new DisplayWorker(optrode().getOrca());

    CameraDisplay *camDisplay = new CameraDisplay(this);
    camDisplay->setPlotSize(QSize(512, 512));

    camDisplay->setLUTPath(s.value(SETTINGSGROUP_OTHERSETTINGS, SETTING_LUTPATH).toString());

    void (CameraPlot::*fp)(const double*, const size_t) = &CameraPlot::setData;
    connect(dispWorker, &DisplayWorker::newImage, camDisplay->getPlot(), fp);

    AspectRatioWidget *arw = new AspectRatioWidget(camDisplay, 1);


    QRadioButton *allRadioButton = new QRadioButton("All");
    QRadioButton *led1RadioButton = new QRadioButton("LED 1");
    QRadioButton *led2RadioButton = new QRadioButton("LED 2");

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addStretch();
    hLayout->addWidget(allRadioButton);
    hLayout->addWidget(led1RadioButton);
    hLayout->addWidget(led2RadioButton);
    hLayout->addStretch();

    posCW = new PIPositionControlWidget(this);
    posCW->setTitle("PI stages");
    posCW->appendRow(optrode().getZAxis(), "1", "Z");

    QString g = SETTINGSGROUP_ZAXIS;
    posCW->getPositionSpinbox(0)->setValue(s.value(g, SETTING_POS).toDouble());
    posCW->getVelocitySpinBox(0)->setValue(s.value(g, SETTING_VELOCITY).toDouble());
    posCW->getStepSpinBox(0)->setValue(s.value(g, SETTING_STEPSIZE).toDouble());

    QVBoxLayout *leftVLayout = new QVBoxLayout();
    leftVLayout->addWidget(arw);
    leftVLayout->addLayout(hLayout);
    leftVLayout->addWidget(posCW);

    QVBoxLayout *rightVLayout = new QVBoxLayout();
    rightVLayout->addWidget(new ControlsWidget());
    rightVLayout->addStretch();

    hLayout = new QHBoxLayout();
    hLayout->addLayout(leftVLayout, 3);
    hLayout->addWidget(pmw, 4);
    hLayout->addLayout(rightVLayout);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(hLayout, 8);
    vLayout->addWidget(timePlot, 2);

    setLayout(vLayout);

    connect(allRadioButton, &QRadioButton::clicked, [ = ](bool checked){
        if (checked) {
            dispWorker->setDisplayWhat(DisplayWorker::DISPLAY_ALL);
        }
    });

    connect(led1RadioButton, &QRadioButton::clicked, [ = ](bool checked){
        if (checked) {
            dispWorker->setDisplayWhat(DisplayWorker::DISPLAY_LED1);
        }
    });

    connect(led2RadioButton, &QRadioButton::clicked, [ = ](bool checked){
        if (checked) {
            dispWorker->setDisplayWhat(DisplayWorker::DISPLAY_LED2);
        }
    });
}

void MainPage::saveSettings()
{
    QSettings sett;
    sett.setValue(SETTING_BEHAVCAM_FLIPLR, pmw->isFlippedlr());
    sett.setValue(SETTING_BEHAVCAM_FLIPUD, pmw->isFlippedud());
    sett.setValue(SETTING_BEHAVCAM_ROTSTEP, pmw->getRotationStep());

    Settings s = settings();
    QString g = SETTINGSGROUP_ZAXIS;
    s.setValue(g, SETTING_POS, posCW->getPositionSpinbox(0)->value());
    s.setValue(g, SETTING_VELOCITY, posCW->getVelocitySpinBox(0)->value());
    s.setValue(g, SETTING_STEPSIZE, posCW->getStepSpinBox(0)->value());
}

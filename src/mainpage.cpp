#include <QHBoxLayout>

#include <qtlab/widgets/cameraplot.h>
#include <qtlab/widgets/aspectratiowidget.h>

#include "controlswidget.h"

#include "mainpage.h"

MainPage::MainPage(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void MainPage::setupUi()
{
    ControlsWidget *cw = new ControlsWidget();

    CameraPlot *cp = new CameraPlot(1280, 1024);
    cp->enableAxis(QwtPlot::xBottom, false);
    cp->enableAxis(QwtPlot::yLeft, false);
    cp->enableAxis(QwtPlot::yRight, false);

    AspectRatioWidget *arw = new AspectRatioWidget(cp, 1280, 1024);

    QHBoxLayout *myLayout = new QHBoxLayout();
    myLayout->addWidget(arw);
    myLayout->addWidget(cw);

    setLayout(myLayout);
}

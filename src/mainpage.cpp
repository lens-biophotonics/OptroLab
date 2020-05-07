#include <QHBoxLayout>
#include <QLabel>

#include <qtlab/hw/aravis/camera.h>
#include <qtlab/widgets/aspectratiowidget.h>
#include <qtlab/widgets/pixmapwidget.h>

#include "controlswidget.h"
#include "behavdispworker.h"

#include "optrod.h"

#include "mainpage.h"

MainPage::MainPage(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void MainPage::setupUi()
{
    ControlsWidget *cw = new ControlsWidget();

    PixmapWidget *pmw = new PixmapWidget();
    pmw->setPixmap(QPixmap(":/res/lemmling-Simple-cartoon-mouse.svg"));

    QHBoxLayout *myLayout = new QHBoxLayout(this);
    myLayout->addWidget(pmw);
    myLayout->addWidget(cw);

    setLayout(myLayout);

    BehavDispWorker *worker = new BehavDispWorker(optrod().getBehaviorCamera());
    connect(worker, &BehavDispWorker::newImage,
            pmw, &PixmapWidget::setPixmap);
}

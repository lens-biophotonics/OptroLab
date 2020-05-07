#include <QPushButton>
#include <QVBoxLayout>

#include "optrod.h"

#include "controlswidget.h"

ControlsWidget::ControlsWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void ControlsWidget::setupUi()
{
    QPushButton *initButton = new QPushButton("Initialize");
    QPushButton *startFreeRunButton = new QPushButton("Start free run");
    QPushButton *stopButton = new QPushButton("Stop");

    QVBoxLayout *myLayout = new QVBoxLayout();
    myLayout->addWidget(initButton);
    myLayout->addWidget(startFreeRunButton);
    myLayout->addWidget(stopButton);
    myLayout->addStretch();
    setLayout(myLayout);

    QState *us = optrod().getState(Optrod::STATE_UNINITIALIZED);
    QState *is = optrod().getState(Optrod::STATE_INITIALIZING);
    QState *rs = optrod().getState(Optrod::STATE_READY);
    QState *cs = optrod().getState(Optrod::STATE_CAPTURING);

    QList<QWidget *> wList;

    wList = {
        initButton,
    };

    for (QWidget * w : wList) {
        us->assignProperty(w, "enabled", true);
        is->assignProperty(w, "enabled", false);
    }

    wList = {
        startFreeRunButton,
        stopButton,
    };

    for (QWidget * w : wList) {
        us->assignProperty(w, "enabled", false);
        is->assignProperty(w, "enabled", false);
    }

    wList = {
        startFreeRunButton,
    };

    for (QWidget * w : wList) {
        rs->assignProperty(w, "enabled", true);
        cs->assignProperty(w, "enabled", false);
    }

    wList = {
        stopButton,
    };

    for (QWidget * w : wList) {
        rs->assignProperty(w, "enabled", false);
        cs->assignProperty(w, "enabled", true);
    }

    auto clicked = &QPushButton::clicked;
    connect(initButton, clicked, &optrod(), &Optrod::initialize);
    connect(startFreeRunButton, clicked, &optrod(), &Optrod::startFreeRun);
    connect(stopButton, clicked, &optrod(), &Optrod::stop);
}

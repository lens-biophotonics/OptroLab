#include <QPushButton>
#include <QVBoxLayout>

#include "optrode.h"

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

    QState *us = optrode().getState(Optrode::STATE_UNINITIALIZED);
    QState *is = optrode().getState(Optrode::STATE_INITIALIZING);
    QState *rs = optrode().getState(Optrode::STATE_READY);
    QState *cs = optrode().getState(Optrode::STATE_CAPTURING);

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
    connect(initButton, clicked, &optrode(), &Optrode::initialize);
    connect(startFreeRunButton, clicked, &optrode(), &Optrode::startFreeRun);
    connect(stopButton, clicked, &optrode(), &Optrode::stop);
}

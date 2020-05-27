#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QGridLayout>

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

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(initButton);
    hLayout->addWidget(startFreeRunButton);
    hLayout->addWidget(stopButton);
    hLayout->addStretch();

    QGroupBox *controlsGb = new QGroupBox("Controls");
    controlsGb->setLayout(hLayout);

    QComboBox *mainTrigPhysChanComboBox = new QComboBox();
    QDoubleSpinBox *mainTrigFreqComboBox = new QDoubleSpinBox();
    mainTrigFreqComboBox->setSuffix("Hz");
    mainTrigFreqComboBox->setRange(0, 100e6);

    int row = 0;

    QGridLayout *grid = new QGridLayout();
    grid->addWidget(new QLabel("Physical channel"), row, 0);
    grid->addWidget(mainTrigPhysChanComboBox, row++, 1);
    grid->addWidget(new QLabel("Frequency"), row, 0);
    grid->addWidget(mainTrigFreqComboBox, row++, 1);
    QGroupBox *mainTrigGb = new QGroupBox("Main Trigger");
    mainTrigGb->setLayout(grid);

    QComboBox *electrodePhysChanComboBox = new QComboBox();
    QDoubleSpinBox *electrodeSampRateComboBox = new QDoubleSpinBox();

    row = 0;
    grid = new QGridLayout();
    grid->addWidget(new QLabel("Physical channel"), row, 0);
    grid->addWidget(electrodePhysChanComboBox, row++, 1);
    grid->addWidget(new QLabel("Sampling rate"), row, 0);
    grid->addWidget(electrodeSampRateComboBox, row++, 1);
    QGroupBox *electrodeGb = new QGroupBox("Electrode readout");
    electrodeGb->setLayout(grid);

    QVBoxLayout *myLayout = new QVBoxLayout();
    myLayout->addWidget(mainTrigGb);
    myLayout->addWidget(electrodeGb);
    myLayout->addWidget(controlsGb);
    setLayout(myLayout);

    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

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

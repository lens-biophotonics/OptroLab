#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QListView>
#include <QGridLayout>

#include <qtlab/hw/ni/natinst.h>

#include "optrode.h"

#include "controlswidget.h"
#include "tasks.h"

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

    Tasks *t = optrode().NITasks();

    // main Trigger
    QComboBox *mainTrigPhysChanComboBox = new QComboBox();
    mainTrigPhysChanComboBox->addItems(NI::getAOPhysicalChans());
    mainTrigPhysChanComboBox->setCurrentText(t->getMainTrigPhysChan());
    QDoubleSpinBox *mainTrigFreqSpinBox = new QDoubleSpinBox();
    mainTrigFreqSpinBox->setSuffix("Hz");
    mainTrigFreqSpinBox->setRange(0, 100e6);
    mainTrigFreqSpinBox->setValue(t->getMainTrigFreq());

    int row = 0;

    QGridLayout *grid = new QGridLayout();
    grid->addWidget(new QLabel("Physical channel"), row, 0);
    grid->addWidget(mainTrigPhysChanComboBox, row++, 1);
    grid->addWidget(new QLabel("Frequency"), row, 0);
    grid->addWidget(mainTrigFreqSpinBox, row++, 1);
    QGroupBox *mainTrigGb = new QGroupBox("Main Trigger");
    mainTrigGb->setLayout(grid);

    // electrode readout
    QComboBox *electrodePhysChanComboBox = new QComboBox();
    electrodePhysChanComboBox->addItems(NI::getAIPhysicalChans());
    electrodePhysChanComboBox->setCurrentText(t->getElectrodeReadoutPhysChan());
    QDoubleSpinBox *electrodeSampRateSpinBox = new QDoubleSpinBox();
    electrodeSampRateSpinBox->setSuffix("Hz");
    electrodeSampRateSpinBox->setValue(t->getElectrodeReadoutRate());

    row = 0;
    grid = new QGridLayout();
    grid->addWidget(new QLabel("Physical channel"), row, 0);
    grid->addWidget(electrodePhysChanComboBox, row++, 1);
    grid->addWidget(new QLabel("Sampling rate"), row, 0);
    grid->addWidget(electrodeSampRateSpinBox, row++, 1);
    QGroupBox *electrodeGb = new QGroupBox("Electrode readout");
    electrodeGb->setLayout(grid);

    // shutter pulse
    QComboBox *shutterPhysChanComboBox = new QComboBox();
    shutterPhysChanComboBox->addItems(NI::getCOPhysicalChans());
    shutterPhysChanComboBox->setCurrentText(t->getShutterPulseCounter());
    QDoubleSpinBox *shutterDelay = new QDoubleSpinBox();
    shutterDelay->setSuffix("s");
    shutterDelay->setRange(0, 3600);
    shutterDelay->setValue(t->getShutterInitialDelay());
    QSpinBox *shutterDutySpinBox = new QSpinBox();
    shutterDutySpinBox->setSuffix("%");
    shutterDutySpinBox->setRange(0, 100);
    shutterDutySpinBox->setValue(t->getShutterPulseDuty());
    QDoubleSpinBox *shutterFreqSpinBox = new QDoubleSpinBox();
    shutterFreqSpinBox->setSuffix("Hz");
    shutterFreqSpinBox->setRange(0, 100e6);
    shutterFreqSpinBox->setValue(t->getShutterPulseFrequency());
    QComboBox *shutterTermComboBox = new QComboBox();
    QListView *view = new QListView();
    view->setFixedWidth(350);
    shutterTermComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    shutterTermComboBox->setView(view);
    shutterTermComboBox->addItems(NI::getTerminals());
    shutterTermComboBox->setMinimumContentsLength(15);
    shutterTermComboBox->setCurrentText(t->getShutterPulseTerm());
    QSpinBox *shutterNPulsesSpinBox = new QSpinBox();
    shutterNPulsesSpinBox->setRange(0, 100000);
    shutterNPulsesSpinBox->setValue(t->getShutterPulseNPulses());

    row = 0;
    grid = new QGridLayout();
    grid->addWidget(new QLabel("Physical channel"), row, 0);
    grid->addWidget(shutterPhysChanComboBox, row++, 1);
    grid->addWidget(new QLabel("Terminal"), row, 0);
    grid->addWidget(shutterTermComboBox, row++, 1);
    grid->addWidget(new QLabel("Initial delay"), row, 0);
    grid->addWidget(shutterDelay, row++, 1);
    grid->addWidget(new QLabel("Frequency"), row, 0);
    grid->addWidget(shutterFreqSpinBox, row++, 1);
    grid->addWidget(new QLabel("Duty cycle"), row, 0);
    grid->addWidget(shutterDutySpinBox, row++, 1);
    grid->addWidget(new QLabel("Number of pulses"), row, 0);
    grid->addWidget(shutterNPulsesSpinBox, row++, 1);

    QGroupBox *shutterGb = new QGroupBox("Shutter pulse");
    shutterGb->setLayout(grid);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(mainTrigGb);
    vLayout->addWidget(electrodeGb);
    vLayout->addWidget(shutterGb);

    QVBoxLayout *myLayout = new QVBoxLayout();
    myLayout->addLayout(vLayout);
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
    connect(startFreeRunButton, clicked, this, [ = ](){
        Tasks *t = optrode().NITasks();
        t->setMainTrigFreq(mainTrigFreqSpinBox->value());
        t->setMainTrigPhysChan(mainTrigPhysChanComboBox->currentText());

        t->setShutterPulseDuty(shutterDutySpinBox->value() / 100.);
        t->setShutterPulseTerm(shutterTermComboBox->currentText());
        t->setShutterInitialDelay(shutterDelay->value());
        t->setShutterPulseCounter(shutterPhysChanComboBox->currentText());
        t->setShutterPulseFrequency(shutterFreqSpinBox->value());
        t->setShutterPulseNPulses(shutterNPulsesSpinBox->value());

        t->setElectrodeReadoutPhysChan(electrodePhysChanComboBox->currentText());
        t->setElectrodeReadoutRate(electrodeSampRateSpinBox->value());

        optrode().startFreeRun();
    });
    connect(stopButton, clicked, &optrode(), &Optrode::stop);
}

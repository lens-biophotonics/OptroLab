#include <functional>

#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QListView>
#include <QGridLayout>
#include <QProgressBar>
#include <QLineEdit>
#include <QFileDialog>

#include <qtlab/hw/ni/natinst.h>

#include "optrode.h"

#include "controlswidget.h"
#include "elreadoutworker.h"
#include "tasks.h"

ControlsWidget::ControlsWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void ControlsWidget::setupUi()
{
    Tasks *t = optrode().NITasks();

    // main Trigger

    QComboBox *mainTrigPhysChanComboBox = new QComboBox();
    mainTrigPhysChanComboBox->addItems(NI::getAOPhysicalChans());
    mainTrigPhysChanComboBox->setCurrentText(t->getMainTrigPhysChan());
    QSpinBox *mainTrigFreqSpinBox = new QSpinBox();
    mainTrigFreqSpinBox->setSuffix("Hz");
    mainTrigFreqSpinBox->setRange(1, 90);
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
    electrodeSampRateSpinBox->setRange(0, 50000);
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
    QSpinBox *shutterDutySpinBox = new QSpinBox();
    shutterDutySpinBox->setSuffix("%");
    shutterDutySpinBox->setRange(0, 100);
    shutterDutySpinBox->setValue(t->getShutterPulseDuty() * 100);
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

    row = 0;
    grid = new QGridLayout();
    grid->addWidget(new QLabel("Physical channel"), row, 0);
    grid->addWidget(shutterPhysChanComboBox, row++, 1);
    grid->addWidget(new QLabel("Terminal"), row, 0);
    grid->addWidget(shutterTermComboBox, row++, 1);
    grid->addWidget(new QLabel("Frequency"), row, 0);
    grid->addWidget(shutterFreqSpinBox, row++, 1);
    grid->addWidget(new QLabel("Duty cycle"), row, 0);
    grid->addWidget(shutterDutySpinBox, row++, 1);

    QGroupBox *shutterGb = new QGroupBox("Shutter pulse");
    shutterGb->setLayout(grid);


    // Timing

    QDoubleSpinBox *shutterDelaySpinBox = new QDoubleSpinBox();
    shutterDelaySpinBox->setSuffix("s");
    shutterDelaySpinBox->setRange(0, 3600);
    shutterDelaySpinBox->setValue(t->getShutterInitialDelay());

    QSpinBox *postStimulationSpinBox = new QSpinBox();
    postStimulationSpinBox->setSuffix("s");
    postStimulationSpinBox->setRange(0, 3600);
    postStimulationSpinBox->setValue(optrode().getPostStimulation());

    QDoubleSpinBox *stimulationSpinBox = new QDoubleSpinBox();
    stimulationSpinBox->setRange(0, 3600);
    stimulationSpinBox->setSuffix("s");
    stimulationSpinBox->setValue(t->stimulationDuration());

    row = 0;
    grid = new QGridLayout();
    grid->addWidget(new QLabel("Baseline"), row, 0);
    grid->addWidget(shutterDelaySpinBox, row++, 1);
    grid->addWidget(new QLabel("Stimulation"), row, 0);
    grid->addWidget(stimulationSpinBox, row++, 1);
    grid->addWidget(new QLabel("Post stimul."), row, 0);
    grid->addWidget(postStimulationSpinBox, row++, 1);

    QGroupBox *timingGb = new QGroupBox("Timing");
    timingGb->setLayout(grid);


    // Controls

    QPushButton *initButton = new QPushButton("Initialize");
    QPushButton *startFreeRunButton = new QPushButton("Start free run");
    QPushButton *startButton = new QPushButton("Start");
    QPushButton *stopButton = new QPushButton("Stop");
    QProgressBar *progressBar = new QProgressBar();
    progressBar->setFormat("%p%");

    QTimer *timer = new QTimer();
    connect(&optrode(), &Optrode::started, this, [ = ](bool freeRun) {
        if (freeRun)
            return;
        progressBar->reset();
        int maximum = optrode().totalDuration()
                      * optrode().NITasks()->getElectrodeReadoutRate();
        progressBar->setRange(0, maximum);
        timer->start(1000);
    });
    connect(&optrode(), &Optrode::stopped, this, [ = ](){
        timer->stop();
        if (!optrode().isFreeRunEnabled())
            progressBar->setValue(progressBar->maximum());
    });
    connect(timer, &QTimer::timeout, this, [ = ](){
        progressBar->setValue(optrode().getElReadoutWorker()->getTotRead());
    });

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(initButton);
    vLayout->addWidget(startFreeRunButton);
    vLayout->addWidget(startButton);
    vLayout->addWidget(stopButton);
    vLayout->addStretch();
    vLayout->addWidget(progressBar);

    QGroupBox *controlsGb = new QGroupBox("Controls");
    controlsGb->setLayout(vLayout);


    // output files
    row = 0;
    int col = 0;
    grid = new QGridLayout();
    grid->addWidget(new QLabel("Path"), row, col++);
    QLineEdit *outputPathLineEdit = new QLineEdit();
    outputPathLineEdit->setText(optrode().getOutputDir());
    grid->addWidget(outputPathLineEdit, row, col++);
    QPushButton *outputPathPushButton = new QPushButton("...");
    grid->addWidget(outputPathPushButton, row++, col);
    col = 0;
    grid->addWidget(new QLabel("Run name"), row, col++);
    QLineEdit *runNameLineEdit = new QLineEdit();
    runNameLineEdit->setText(optrode().getRunName());
    grid->addWidget(runNameLineEdit, row++, col++);

    QGroupBox *outputGb = new QGroupBox("Output");
    outputGb->setLayout(grid);

    vLayout = new QVBoxLayout();
    vLayout->addWidget(mainTrigGb);
    vLayout->addWidget(electrodeGb);
    vLayout->addWidget(shutterGb);

    QVBoxLayout *vLayout2 = new QVBoxLayout();
    vLayout2->addWidget(timingGb);
    vLayout2->addWidget(controlsGb);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addLayout(vLayout);
    hLayout->addLayout(vLayout2);

    QVBoxLayout *myLayout = new QVBoxLayout();
    myLayout->addLayout(hLayout);
    myLayout->addWidget(outputGb);

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
        startButton,
    };

    for (QWidget * w : wList) {
        us->assignProperty(w, "enabled", false);
        is->assignProperty(w, "enabled", false);
    }

    wList = {
        stopButton,
    };

    for (QWidget * w : wList) {
        rs->assignProperty(w, "enabled", false);
        cs->assignProperty(w, "enabled", true);
    }

    wList = {
        startFreeRunButton,
        startButton,
        mainTrigGb,
        electrodeGb,
        shutterGb,
        outputGb,
        timingGb,
    };

    for (QWidget * w : wList) {
        rs->assignProperty(w, "enabled", true);
        cs->assignProperty(w, "enabled", false);
    }

    std::function<void()> applyValues = [ = ](){
        Tasks *t = optrode().NITasks();
        t->setMainTrigFreq(mainTrigFreqSpinBox->value());
        t->setMainTrigPhysChan(mainTrigPhysChanComboBox->currentText());

        t->setShutterPulseDuty(shutterDutySpinBox->value() / 100.);
        t->setShutterPulseTerm(shutterTermComboBox->currentText());
        t->setShutterPulseCounter(shutterPhysChanComboBox->currentText());
        t->setShutterPulseFrequency(shutterFreqSpinBox->value());
        t->setStimulationDuration(stimulationSpinBox->value());

        t->setElectrodeReadoutPhysChan(electrodePhysChanComboBox->currentText());
        t->setElectrodeReadoutRate(electrodeSampRateSpinBox->value());

        t->setShutterInitialDelay(shutterDelaySpinBox->value());
        optrode().setPostStimulation(postStimulationSpinBox->value());

        optrode().setOutputDir(outputPathLineEdit->text());
        optrode().setRunName(runNameLineEdit->text());
    };

    auto clicked = &QPushButton::clicked;
    connect(initButton, clicked, &optrode(), &Optrode::initialize);
    connect(startFreeRunButton, clicked, &optrode(), [ = ](){
        applyValues();
        optrode().startFreeRun();
    });
    connect(startButton, clicked, &optrode(), [ = ](){
        applyValues();
        optrode().start();
    });
    connect(stopButton, clicked, &optrode(), &Optrode::stop);

    connect(outputPathPushButton, &QPushButton::clicked, this, [ = ](){
        QFileDialog dialog;
        dialog.setDirectory(outputPathLineEdit->text());
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setOption(QFileDialog::ShowDirsOnly, true);

        if (!dialog.exec())
            return;

        QString path = dialog.selectedFiles().at(0);
        outputPathLineEdit->setText(path);
    });
}

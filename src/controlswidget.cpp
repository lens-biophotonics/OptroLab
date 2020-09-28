#include <functional>

#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
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
#include "chameleoncamera.h"

ControlsWidget::ControlsWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void ControlsWidget::setupUi()
{
    Tasks *t = optrode().NITasks();

    // trigger

    QComboBox *mainTrigPhysChanComboBox = new QComboBox();
    mainTrigPhysChanComboBox->addItems(NI::getCOPhysicalChans());
    mainTrigPhysChanComboBox->setCurrentText(t->getMainTrigPhysChan());
    QComboBox *mainTrigTermComboBox = new QComboBox();
    mainTrigTermComboBox->addItems(NI::getTerminals());
    mainTrigTermComboBox->setCurrentText(t->getMainTrigTerm());

    int row = 0;

    QGridLayout *grid = new QGridLayout();
    grid->addWidget(new QLabel("Physical channel"), row, 0);
    grid->addWidget(mainTrigPhysChanComboBox, row++, 1);
    grid->addWidget(new QLabel("Terminal"), row, 0);
    grid->addWidget(mainTrigTermComboBox, row++, 1);
    QGroupBox *trigGb = new QGroupBox("Trigger");
    trigGb->setLayout(grid);


    // LEDs
    QSpinBox *LEDFreqSpinBox = new QSpinBox();
    LEDFreqSpinBox->setSuffix("Hz");
    LEDFreqSpinBox->setRange(1, 43);
    LEDFreqSpinBox->setValue(t->getLEDFreq());

    QComboBox *LED1PhysChanComboBox = new QComboBox();
    LED1PhysChanComboBox->addItems(NI::getCOPhysicalChans());
    LED1PhysChanComboBox->setCurrentText(t->getLED1PhysChan());
    QComboBox *LED1TermComboBox = new QComboBox();
    LED1TermComboBox->addItems(NI::getTerminals());
    LED1TermComboBox->setCurrentText(t->getLED1Term());

    QComboBox *LED2PhysChanComboBox = new QComboBox();
    LED2PhysChanComboBox->addItems(NI::getCOPhysicalChans());
    LED2PhysChanComboBox->setCurrentText(t->getLED2PhysChan());
    QComboBox *LED2TermComboBox = new QComboBox();
    LED2TermComboBox->addItems(NI::getTerminals());
    LED2TermComboBox->setCurrentText(t->getLED2Term());

    QCheckBox *LED1CheckBox = new QCheckBox("LED 1");
    QCheckBox *LED2CheckBox = new QCheckBox("LED 2");

    LED1CheckBox->setChecked(t->getLED1Enabled());
    LED2CheckBox->setChecked(t->getLED2Enabled());

    grid = new QGridLayout();
    grid->addWidget(new QLabel("LED 1 Phys chan"), row, 0);
    grid->addWidget(LED1PhysChanComboBox, row++, 1);
    grid->addWidget(new QLabel("LED 1 Terminal"), row, 0);
    grid->addWidget(LED1TermComboBox, row++, 1);
    grid->addWidget(new QLabel("LED 2 Phys chan"), row, 0);
    grid->addWidget(LED2PhysChanComboBox, row++, 1);
    grid->addWidget(new QLabel("LED 2 Terminal"), row, 0);
    grid->addWidget(LED2TermComboBox, row++, 1);
    grid->addWidget(new QLabel("Frequency"), row, 0);
    grid->addWidget(LEDFreqSpinBox, row++, 1);
    grid->addWidget(LED1CheckBox, row, 0);
    grid->addWidget(LED2CheckBox, row, 1);
    QGroupBox *LEDGb = new QGroupBox("LEDs");
    LEDGb->setLayout(grid);

    // electrode readout

    QComboBox *electrodePhysChanComboBox = new QComboBox();
    electrodePhysChanComboBox->addItems(NI::getAIPhysicalChans());
    electrodePhysChanComboBox->setCurrentText(t->getElectrodeReadoutPhysChan());
    QDoubleSpinBox *electrodeSampRateSpinBox = new QDoubleSpinBox();
    electrodeSampRateSpinBox->setSuffix("Hz");
    electrodeSampRateSpinBox->setRange(25, 50000);
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

    QCheckBox *stimulationCheckBox = new QCheckBox("Stimulation");

    connect(stimulationCheckBox, &QCheckBox::toggled, stimulationSpinBox, [ = ](bool checked) {
        stimulationSpinBox->setEnabled(checked);
        postStimulationSpinBox->setEnabled(checked);
    });

    stimulationCheckBox->setChecked(t->getShutterPulseEnabled());

    row = 0;
    grid = new QGridLayout();
    grid->addWidget(new QLabel("Baseline"), row, 1);
    grid->addWidget(shutterDelaySpinBox, row++, 2);
    grid->addWidget(stimulationCheckBox, row, 1);
    grid->addWidget(stimulationSpinBox, row++, 2);
    grid->addWidget(new QLabel("Post stimul."), row, 1);
    grid->addWidget(postStimulationSpinBox, row++, 2);

    QGroupBox *timingGb = new QGroupBox("Timing");
    timingGb->setLayout(grid);


    // Controls

    QPushButton *initButton = new QPushButton("Initialize");
    QPushButton *startFreeRunButton = new QPushButton("Start free run");
    QPushButton *startButton = new QPushButton("Start");
    QPushButton *stopButton = new QPushButton("Stop");
    QLabel *successLabel = new QLabel();

    QProgressBar *progressBar = new QProgressBar();
    progressBar->setFormat("%p%");

    QTimer *timer = new QTimer();
    connect(&optrode(), &Optrode::started, this, [ = ](bool freeRun) {
        successLabel->clear();
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
        if (!optrode().isFreeRunEnabled()) {
            progressBar->setValue(progressBar->maximum());
            if (optrode().isSuccess()) {
                successLabel->setText("SUCCESS");
                successLabel->setStyleSheet("QLabel {color: green};");
            } else {
                successLabel->setText("ERRORS");
                successLabel->setStyleSheet("QLabel {color: red};");
            }
        }
    });

    connect(&optrode(), &Optrode::pleaseWait, this, [ = ](){
        timer->stop();
        successLabel->setText("PLEASE WAIT");
        successLabel->setStyleSheet("QLabel {color: DarkOrange};");
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
    vLayout->addWidget(successLabel);
    vLayout->addWidget(progressBar);

    QGroupBox *controlsGb = new QGroupBox("Controls");
    controlsGb->setLayout(vLayout);


    // ROI

    QRect roi = optrode().getBehaviorCamera()->getROI();
    QSpinBox *ROIxSpinBox = new QSpinBox();
    ROIxSpinBox->setRange(0, 1280 - 1);
    ROIxSpinBox->setValue(roi.x());
    QSpinBox *ROIySpinBox = new QSpinBox();
    ROIySpinBox->setRange(0, 1024 - 1);
    ROIySpinBox->setValue(roi.y());
    QSpinBox *ROIWidthSpinBox = new QSpinBox();
    ROIWidthSpinBox->setRange(0, 1280);
    ROIWidthSpinBox->setValue(roi.width());
    QSpinBox *ROIHeightSpinBox = new QSpinBox();
    ROIHeightSpinBox->setRange(0, 1024);
    ROIHeightSpinBox->setValue(roi.height());

    row = 0;
    grid = new QGridLayout();
    grid->addWidget(new QLabel("X"), row, 0);
    grid->addWidget(ROIxSpinBox, row++, 1);
    grid->addWidget(new QLabel("Y"), row, 0);
    grid->addWidget(ROIySpinBox, row++, 1);
    grid->addWidget(new QLabel("Width"), row, 0);
    grid->addWidget(ROIWidthSpinBox, row++, 1);
    grid->addWidget(new QLabel("Height"), row, 0);
    grid->addWidget(ROIHeightSpinBox, row++, 1);

    QGroupBox *ROIGb = new QGroupBox("BehavCam ROI");
    ROIGb->setLayout(grid);

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
    vLayout->addWidget(trigGb);
    vLayout->addWidget(LEDGb);
    vLayout->addWidget(electrodeGb);
    vLayout->addWidget(shutterGb);

    QVBoxLayout *vLayout2 = new QVBoxLayout();
    vLayout2->addWidget(timingGb);
    vLayout2->addWidget(ROIGb);
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
        trigGb,
        LEDGb,
        ROIGb,
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
        t->setMainTrigPhysChan(mainTrigPhysChanComboBox->currentText());
        t->setMainTrigTerm(mainTrigTermComboBox->currentText());

        t->setLEDFreq(LEDFreqSpinBox->value());
        t->setLED1PhysChan(LED1PhysChanComboBox->currentText());
        t->setLED1Term(LED1TermComboBox->currentText());
        t->setLED2PhysChan(LED2PhysChanComboBox->currentText());
        t->setLED2Term(LED2TermComboBox->currentText());

        t->setLED1Enabled(LED1CheckBox->isChecked());
        t->setLED2Enabled(LED2CheckBox->isChecked());

        t->setShutterPulseDuty(shutterDutySpinBox->value() / 100.);
        t->setShutterPulseTerm(shutterTermComboBox->currentText());
        t->setShutterPulseCounter(shutterPhysChanComboBox->currentText());
        t->setShutterPulseFrequency(shutterFreqSpinBox->value());
        t->setStimulationDuration(stimulationSpinBox->value());
        t->setShutterPulseEnabled(stimulationCheckBox->isChecked());

        t->setElectrodeReadoutPhysChan(electrodePhysChanComboBox->currentText());
        t->setElectrodeReadoutRate(electrodeSampRateSpinBox->value());

        t->setShutterInitialDelay(shutterDelaySpinBox->value());
        optrode().setPostStimulation(postStimulationSpinBox->value());

        optrode().setOutputDir(outputPathLineEdit->text());
        optrode().setRunName(runNameLineEdit->text());

        QRect roi;
        roi.setX(ROIxSpinBox->value());
        roi.setY(ROIySpinBox->value());
        roi.setWidth(ROIWidthSpinBox->value());
        roi.setHeight(ROIHeightSpinBox->value());
        optrode().getBehaviorCamera()->setROI(roi);
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

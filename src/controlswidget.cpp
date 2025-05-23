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
#include <QRadioButton>
#include <QMessageBox>

#include <qtlab/hw/ni/natinst.h>

#include "optrode.h"

#include "controlswidget.h"
#include "elreadoutworker.h"
#include "tasks.h"
#include "dds.h"
#include "chameleoncamera.h"
#include "savestackworker.h"
#include "camdisplay.h"

ControlsWidget::ControlsWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void ControlsWidget::setupUi()
{
    Tasks *t = optrode().NITasks();

    // trigger

    QComboBox *mainTrigTermComboBox = new QComboBox();
    mainTrigTermComboBox->addItems(NI::getTerminals());
    mainTrigTermComboBox->setCurrentText(t->getMainTrigTerm());

    int row = 0;

    QGridLayout *grid = new QGridLayout();
    grid->addWidget(new QLabel("Terminal"), row, 0);
    grid->addWidget(mainTrigTermComboBox, row++, 1);
    QGroupBox *trigGb = new QGroupBox("Trigger");
    trigGb->setLayout(grid);


    // LEDs
    QSpinBox *LEDFreqSpinBox = new QSpinBox();
    LEDFreqSpinBox->setSuffix("Hz");
    LEDFreqSpinBox->setRange(1, 43);
    LEDFreqSpinBox->setValue(t->getLEDFreq());

    QComboBox *LED1TermComboBox = new QComboBox();
    LED1TermComboBox->addItems(NI::getTerminals());
    LED1TermComboBox->setCurrentText(t->getLED1Term());

    QComboBox *LED2TermComboBox = new QComboBox();
    LED2TermComboBox->addItems(NI::getTerminals());
    LED2TermComboBox->setCurrentText(t->getLED2Term());

    QCheckBox *LED1CheckBox = new QCheckBox("LED 1");
    QCheckBox *LED2CheckBox = new QCheckBox("LED 2");

    LED1CheckBox->setChecked(t->getLED1Enabled());
    LED2CheckBox->setChecked(t->getLED2Enabled());

    grid = new QGridLayout();
    grid->addWidget(new QLabel("LED 1 Terminal"), row, 0);
    grid->addWidget(LED1TermComboBox, row++, 1);
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
    electrodeGb->setCheckable(true);
    electrodeGb->setChecked(t->getElectrodeReadoutEnabled());
    electrodeGb->setLayout(grid);


    // stimulation

    QDoubleSpinBox *stimulationHighTimeSpinBox = new QDoubleSpinBox();
    stimulationHighTimeSpinBox->setSuffix("s");
    stimulationHighTimeSpinBox->setRange(0, 100);
    stimulationHighTimeSpinBox->setDecimals(3);
    stimulationHighTimeSpinBox->setValue(t->getStimulationHighTime());
    QDoubleSpinBox *stimulationLowTimeSpinBox = new QDoubleSpinBox();
    stimulationLowTimeSpinBox->setSuffix("s");
    stimulationLowTimeSpinBox->setRange(0, 100);
    stimulationLowTimeSpinBox->setDecimals(3);
    stimulationLowTimeSpinBox->setValue(t->getStimulationLowTime());
    QComboBox *stimulationTermComboBox = new QComboBox();
    QListView *view = new QListView();
    view->setFixedWidth(350);
    stimulationTermComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    stimulationTermComboBox->setView(view);
    stimulationTermComboBox->addItems(NI::getTerminals());
    stimulationTermComboBox->setMinimumContentsLength(15);
    stimulationTermComboBox->setCurrentText(t->getStimulationTerm());

    QComboBox *ddsDevComboBox = new QComboBox();
    ddsDevComboBox->addItems(NI::getSysDevNames());
    ddsDevComboBox->setCurrentText(t->getDDS()->getDevName());

    QCheckBox *continuousStimulationCheckBox = new QCheckBox("Always on");
    connect(continuousStimulationCheckBox, &QCheckBox::toggled, [ = ](bool checked){
        stimulationHighTimeSpinBox->setEnabled(!checked);
        stimulationLowTimeSpinBox->setEnabled(!checked);
    });

    continuousStimulationCheckBox->setChecked(t->getContinuousStimulation());

    QRadioButton *pulseRadio = new QRadioButton("Pulse");
    QRadioButton *aodRadio = new QRadioButton("AOD");
    pulseRadio->setChecked(!t->isAODEnabled());
    aodRadio->setChecked(t->isAODEnabled());

    row = 0;
    grid = new QGridLayout();
    grid->addWidget(new QLabel("Terminal"), row, 0);
    grid->addWidget(stimulationTermComboBox, row++, 1);
    grid->addWidget(new QLabel("DDS"), row, 0);
    grid->addWidget(ddsDevComboBox, row++, 1);
    grid->addWidget(continuousStimulationCheckBox, row++, 1);
    grid->addWidget(new QLabel("High time"), row, 0);
    grid->addWidget(stimulationHighTimeSpinBox, row++, 1);
    grid->addWidget(new QLabel("Low time"), row, 0);
    grid->addWidget(stimulationLowTimeSpinBox, row++, 1);
    grid->addWidget(pulseRadio, row, 0);
    grid->addWidget(aodRadio, row++, 1);

    QGroupBox *stimulationGb = new QGroupBox("Stimulation");
    stimulationGb->setLayout(grid);

    // aux stimulation

    QComboBox *auxStimulationTermComboBox = new QComboBox();
    view = new QListView();
    view->setFixedWidth(350);
    auxStimulationTermComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    auxStimulationTermComboBox->setView(view);
    auxStimulationTermComboBox->addItems(NI::getTerminals());
    auxStimulationTermComboBox->setMinimumContentsLength(15);
    auxStimulationTermComboBox->setCurrentText(t->getAuxStimulationTerm());

    QDoubleSpinBox *auxStimulationHighTimeSpinBox = new QDoubleSpinBox();
    auxStimulationHighTimeSpinBox->setSuffix("s");
    auxStimulationHighTimeSpinBox->setRange(0, 100);
    auxStimulationHighTimeSpinBox->setDecimals(3);
    auxStimulationHighTimeSpinBox->setValue(t->getAuxStimulationHighTime());

    QSpinBox *auxStimulationNPulsesSpinBox = new QSpinBox();
    auxStimulationNPulsesSpinBox->setRange(1, 10000);
    auxStimulationNPulsesSpinBox->setValue(t->getAuxStimulationNPulses());

    QDoubleSpinBox *auxStimulationDelaySpinBox = new QDoubleSpinBox();
    auxStimulationDelaySpinBox->setSuffix("s");
    auxStimulationDelaySpinBox->setRange(0, 100);
    auxStimulationDelaySpinBox->setDecimals(3);
    auxStimulationDelaySpinBox->setValue(t->getAuxStimulationDelay());

    row = 0;
    grid = new QGridLayout();
    grid->addWidget(new QLabel("Terminal"), row, 0);
    grid->addWidget(auxStimulationTermComboBox, row++, 1);
    grid->addWidget(new QLabel("High time"), row, 0);
    grid->addWidget(auxStimulationHighTimeSpinBox, row++, 1);
    grid->addWidget(new QLabel("N Pulses"), row, 0);
    grid->addWidget(auxStimulationNPulsesSpinBox, row++, 1);
    grid->addWidget(new QLabel("Delay (from start)"), row, 0);
    grid->addWidget(auxStimulationDelaySpinBox, row++, 1);

    QGroupBox *auxStimulationGb = new QGroupBox("Aux Stimulation");
    auxStimulationGb->setCheckable(true);
    auxStimulationGb->setChecked(t->getAuxStimulationEnabled());
    auxStimulationGb->setLayout(grid);

    // Timing

    QDoubleSpinBox *baselineSpinBox = new QDoubleSpinBox();
    baselineSpinBox->setSuffix("s");
    baselineSpinBox->setRange(0, 3600);
    baselineSpinBox->setValue(t->getStimulationInitialDelay());

    QSpinBox *postStimulationSpinBox = new QSpinBox();
    postStimulationSpinBox->setSuffix("s");
    postStimulationSpinBox->setRange(0, 3600);
    postStimulationSpinBox->setValue(optrode().getPostStimulation());
    postStimulationSpinBox->setEnabled(false);

    QDoubleSpinBox *stimulationSpinBox = new QDoubleSpinBox();
    stimulationSpinBox->setRange(0, 3600);
    stimulationSpinBox->setSuffix("s");
    stimulationSpinBox->setValue(t->stimulationDuration());
    stimulationSpinBox->setEnabled(false);

    QCheckBox *stimulationCheckBox = new QCheckBox("Stimulation");

    auto updateStimulationUi = [ = ](bool checked) {
        stimulationSpinBox->setEnabled(checked);
        stimulationGb->setEnabled(checked);
        auxStimulationGb->setEnabled(checked);
        postStimulationSpinBox->setEnabled(checked);
    };

    connect(stimulationCheckBox, &QCheckBox::toggled, stimulationSpinBox, updateStimulationUi);
    updateStimulationUi(t->getStimulationEnabled());

    stimulationCheckBox->setChecked(t->getStimulationEnabled());

    row = 0;
    grid = new QGridLayout();
    grid->addWidget(new QLabel("Baseline"), row, 1);
    grid->addWidget(baselineSpinBox, row++, 2);
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
    QProgressBar *multiRunProgressBar = new QProgressBar();
    multiRunProgressBar->setFormat("%v/%m");

    QTimer *timer = new QTimer();
    connect(&optrode(), &Optrode::started, this, [ = ](bool freeRun) {
        successLabel->clear();
        if (freeRun)
            return;
        progressBar->reset();
        progressBar->setRange(0, optrode().getSSWorker()->getFrameCount());
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
            if (optrode().isMultiRunEnabled()) {
                multiRunProgressBar->setValue(multiRunProgressBar->value() + 1);
            }
        }
    });

    connect(&optrode(), &Optrode::pleaseWait, this, [ = ](){
        successLabel->setText("PLEASE WAIT");
        successLabel->setStyleSheet("QLabel {color: DarkOrange};");
    });
    connect(timer, &QTimer::timeout, this, [ = ](){
        progressBar->setValue(optrode().getSSWorker()->getReadFrames());
    });

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(initButton);
    vLayout->addWidget(startFreeRunButton);
    vLayout->addWidget(startButton);
    vLayout->addWidget(stopButton);
    vLayout->addStretch();
    vLayout->addWidget(successLabel);
    vLayout->addWidget(progressBar);
    vLayout->addWidget(multiRunProgressBar);

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

    // multiple runs
    QSpinBox *nRunsSpinBox = new QSpinBox();
    nRunsSpinBox->setRange(2, 10000);
    nRunsSpinBox->setValue(optrode().getNRuns());
    grid = new QGridLayout();
    grid->addWidget(new QLabel("Number of runs"), 0, 0);
    grid->addWidget(nRunsSpinBox, 0, 1);
    QGroupBox *multiRunGb = new QGroupBox("Multiple runs");
    multiRunGb->setCheckable(true);
    multiRunGb->setChecked(optrode().isMultiRunEnabled());
    multiRunGb->setLayout(grid);

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

    QCheckBox *saveElReadoutCheckBox = new QCheckBox("Electrode");
    saveElReadoutCheckBox->setChecked(optrode().isSaveElectrodeEnabled());
    QCheckBox *saveBehavCheckBox = new QCheckBox("Behavior");
    saveBehavCheckBox->setChecked(optrode().isSaveBehaviorEnabled());

    QHBoxLayout *saveLayout = new QHBoxLayout();
    saveLayout->addWidget(saveElReadoutCheckBox);
    saveLayout->addWidget(saveBehavCheckBox);

    grid->addWidget(new QLabel("Save"), ++row, 0);
    grid->addLayout(saveLayout, row, 1);

    QGroupBox *outputGb = new QGroupBox("Output");
    outputGb->setLayout(grid);

    vLayout = new QVBoxLayout();
    vLayout->addWidget(trigGb);
    vLayout->addWidget(LEDGb);
    vLayout->addWidget(electrodeGb);
    vLayout->addWidget(stimulationGb);
    vLayout->addWidget(auxStimulationGb);

    QVBoxLayout *vLayout2 = new QVBoxLayout();
    vLayout2->addWidget(timingGb);
    vLayout2->addWidget(ROIGb);
    vLayout2->addWidget(controlsGb);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addLayout(vLayout);
    hLayout->addLayout(vLayout2);

    QVBoxLayout *myLayout = new QVBoxLayout();
    myLayout->addLayout(hLayout);
    myLayout->addWidget(multiRunGb);
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
        stimulationGb,
        outputGb,
        timingGb,
        multiRunGb,
    };

    for (QWidget * w : wList) {
        rs->assignProperty(w, "enabled", true);
        cs->assignProperty(w, "enabled", false);
    }

    std::function<void()> applyValues = [ = ](){
        Tasks *t = optrode().NITasks();
        t->setMainTrigTerm(mainTrigTermComboBox->currentText());

        t->setLEDFreq(LEDFreqSpinBox->value());
        t->setLED1Term(LED1TermComboBox->currentText());
        t->setLED2Term(LED2TermComboBox->currentText());

        t->setLED1Enabled(LED1CheckBox->isChecked());
        t->setLED2Enabled(LED2CheckBox->isChecked());

        t->setStimulationTerm(stimulationTermComboBox->currentText());
        t->getDDS()->setDevName(ddsDevComboBox->currentText());
        t->setStimulationHighTime(stimulationHighTimeSpinBox->value());
        t->setStimulationLowTime(stimulationLowTimeSpinBox->value());
        t->setStimulationDuration(stimulationSpinBox->value());
        t->setContinuousStimulation(continuousStimulationCheckBox->isChecked());
        t->setStimulationEnabled(stimulationCheckBox->isChecked());
        t->setAODEnabled(aodRadio->isChecked());
        if (camDisplay->getPoints().size() > 0) {
            t->setPoint(camDisplay->getPoints().at(0));
        }
        t->setAuxStimulationEnabled(auxStimulationGb->isChecked());
        t->setAuxStimulationTerm(auxStimulationTermComboBox->currentText());
        t->setAuxStimulationHighTime(auxStimulationHighTimeSpinBox->value());
        t->setAuxStimulationNPulses(auxStimulationNPulsesSpinBox->value());
        t->setAuxStimulationDelay(auxStimulationDelaySpinBox->value());
        t->setElectrodeReadoutPhysChan(electrodePhysChanComboBox->currentText());
        t->setElectrodeReadoutRate(electrodeSampRateSpinBox->value());
        t->setElectrodeReadoutEnabled(electrodeGb->isChecked());

        t->setStimulationInitialDelay(baselineSpinBox->value());
        optrode().setPostStimulation(postStimulationSpinBox->value());

        optrode().setOutputDir(outputPathLineEdit->text());
        optrode().setRunName(runNameLineEdit->text());

        optrode().setSaveElectrodeEnabled(saveElReadoutCheckBox->isChecked());
        optrode().setSaveBehaviorEnabled(saveBehavCheckBox->isChecked());

        optrode().setMultiRunEnabled(multiRunGb->isChecked());
        optrode().setNRuns(nRunsSpinBox->value());

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
    connect(startButton, clicked, [ = ](){
        if (aodRadio->isChecked() && camDisplay->getPoints().size() == 0) {
            QMessageBox::critical(nullptr, "Error",
                                  "No stimulation point selected: use SHIFT+click to select one");
            return;
        }
        applyValues();

        if (QFileInfo(optrode().outputFileFullPath() + ".yaml").exists())
        {
            QMessageBox msgBox;
            QString msg("A measurement with this run name (%1) already exists.");
            msg = msg.arg(optrode().getRunName());
            msgBox.setText(msg);
            msgBox.setInformativeText("Do you want to overwrite existing files?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            int ret = msgBox.exec();
            if (ret == QMessageBox::No) {
                return;
            }
        }

        multiRunProgressBar->reset();
        if (multiRunGb->isChecked()) {
            multiRunProgressBar->setValue(0);
            multiRunProgressBar->setRange(0, nRunsSpinBox->value());
        }
        optrode().start();
    });
    connect(stopButton, clicked, &optrode(), &Optrode::multiRunStop);

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

void ControlsWidget::setCamDisplay(CamDisplay *value)
{
    camDisplay = value;
}

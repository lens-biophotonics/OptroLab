#include <QAction>
#include <QMenuBar>
#include <QMessageBox>
#include <QTextStream>
#include <QSettings>
#include <QCloseEvent>
#include <QThread>
#include <QSerialPortInfo>

#include <Spinnaker.h>
#include "mainwindow.h"
#include "centralwidget.h"
#include "ddsdialog.h"

#include "optrode.h"
#include "settings.h"
#include "tasks.h"
#include "dds.h"

#include "version.h"
#include "chameleoncamera.h"

using namespace Spinnaker;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    loadSettings();

    setupUi();

    QThread *thread = new QThread();
    thread->setObjectName("Optrode_thread");
    optrode().moveToThread(thread);
    thread->start();
}

void MainWindow::setupUi()
{
    setWindowIcon(QIcon(":/res/lemmling-Simple-cartoon-mouse.svg"));
    connect(&optrode(), &Optrode::error, this, [ = ](QString s) {
        QMessageBox::critical(nullptr, "Error", s);
    });

    QAction *quitAction = new QAction(this);
    quitAction->setText("&Quit");
    quitAction->setObjectName("quitAction");
    quitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);

    QAction *saveSettingsAction = new QAction(this);
    saveSettingsAction->setText("Save settings");
    saveSettingsAction->setShortcut(QKeySequence("Ctrl+S"));
    connect(saveSettingsAction, &QAction::triggered,
            this, &MainWindow::saveSettings);

    QAction *aboutAction = new QAction(this);
    aboutAction->setText("&About...");
    aboutAction->setObjectName("aboutAction");
    aboutAction->setShortcut(Qt::Key_F1);

    QMenuBar *menuBar = new QMenuBar();
    setMenuBar(menuBar);

    QAction *DDSDialogAction = new QAction("DDS...");
    DDSDialog *ddsDialog = new DDSDialog(this);

    connect(DDSDialogAction, &QAction::triggered, [ = ](){
        optrode().NITasks()->getDDS()->setIOUDCLKInternal(true);
        optrode().NITasks()->getDDS()->setWriteMode(DDS::WRITE_MODE_TO_NI_TASK);
        ddsDialog->show();
    });

    QMenu *fileMenu = menuBar->addMenu("&File");
    fileMenu->addAction(saveSettingsAction);
    fileMenu->addAction(quitAction);

    menuBar->addAction(DDSDialogAction);

    QMenu *helpMenu = menuBar->addMenu("?");
    helpMenu->addAction(aboutAction);

    setCentralWidget(new CentralWidget(this));

    QMetaObject::connectSlotsByName(this);
}


void MainWindow::on_aboutAction_triggered() const
{
    QMessageBox msgBox;
    QString text;
    QTextStream ts(&text);

    ts << QString("<b>%1</b> ").arg(PROGRAM_NAME);
    ts << "<i>Version</i>:&nbsp;" << getProgramVersionString() << "<br>";
    ts << "<i>Author</i>:<br>";
    ts << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Giacomo Mazzamuto<br>";
    ts << "<i>Date</i>:&nbsp; April 2020<br>";
    ts << "Qt version: " << qVersion() << "<br>";

    SystemPtr system = Spinnaker::System::GetInstance();
    const LibraryVersion spinnakerLibraryVersion = system->GetLibraryVersion();

    ts << "Spinnaker version: " << spinnakerLibraryVersion.major
       << "." << spinnakerLibraryVersion.minor
       << "." << spinnakerLibraryVersion.type
       << "." << spinnakerLibraryVersion.build;

    msgBox.setText(text);
    msgBox.setWindowTitle(QString("About %1").arg(PROGRAM_NAME));

    msgBox.setIconPixmap(QPixmap(":/res/logos.png"));
    msgBox.exec();
}

void MainWindow::saveSettings() const
{
    Settings s = settings();

    Tasks *t = optrode().NITasks();

    QString g = SETTINGSGROUP_MAINTRIG;
    s.setValue(g, SETTING_TERM, t->getMainTrigTerm());

    g = SETTINGSGROUP_BEHAVCAMROI;
    s.setValue(g, SETTING_ROI, optrode().getBehaviorCamera()->getROI());

    g = SETTINGSGROUP_LED1;
    s.setValue(g, SETTING_FREQ, t->getLEDFreq());
    s.setValue(g, SETTING_TERM, t->getLED1Term());
    s.setValue(g, SETTING_ENABLED, t->getLED1Enabled());

    g = SETTINGSGROUP_LED2;
    s.setValue(g, SETTING_TERM, t->getLED2Term());
    s.setValue(g, SETTING_ENABLED, t->getLED2Enabled());

    g = SETTINGSGROUP_ELREADOUT;
    s.setValue(g, SETTING_PHYSCHAN, t->getElectrodeReadoutPhysChan());
    s.setValue(g, SETTING_FREQ, t->getElectrodeReadoutRate());
    s.setValue(g, SETTING_ENABLED, t->getElectrodeReadoutEnabled());

    g = SETTINGSGROUP_STIMULATION;
    s.setValue(g, SETTING_LOW_TIME, t->getStimulationLowTime());
    s.setValue(g, SETTING_HIGH_TIME, t->getStimulationHighTime());
    s.setValue(g, SETTING_TERM, t->getStimulationTerm());
    s.setValue(g, SETTING_ENABLED, t->getStimulationEnabled());
    s.setValue(g, SETTING_ALWAYS_ON, t->getContinuousStimulation());
    s.setValue(g, SETTING_AOD_ENABLED, t->isAODEnabled());

    g = SETTINGSGROUP_TIMING;
    s.setValue(g, SETTING_INITIALDELAY, t->getStimulationInitialDelay());
    s.setValue(g, SETTING_STIMDURATION, t->stimulationDuration());
    s.setValue(g, SETTING_POSTSTIMULATION, optrode().getPostStimulation());

    g = SETTINGSGROUP_ACQUISITION;
    s.setValue(g, SETTING_OUTPUTPATH, optrode().getOutputDir());
    s.setValue(g, SETTING_RUNNAME, optrode().getRunName());
    s.setValue(g, SETTING_SAVEELECTRODE, optrode().isSaveElectrodeEnabled());
    s.setValue(g, SETTING_SAVEBEHAVIOR, optrode().isSaveBehaviorEnabled());

    g = SETTINGSGROUP_ZAXIS;
    PIDevice *dev = optrode().getZAxis();
    s.setValue(g, SETTING_BAUD, dev->getBaud());
    s.setValue(g, SETTING_DEVICENUMBER, dev->getDeviceNumber());

    s.saveSettings();

    QSettings settings;

    settings.beginGroup("MainWindow");
    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());
    settings.endGroup();
}

void MainWindow::loadSettings()
{
    const Settings s = settings();

    Tasks *t = optrode().NITasks();
    QString g = SETTINGSGROUP_MAINTRIG;
    t->setMainTrigTerm(s.value(g, SETTING_TERM).toString());

    g = SETTINGSGROUP_LED1;
    t->setLEDFreq(s.value(g, SETTING_FREQ).toDouble());
    t->setLED1Term(s.value(g, SETTING_TERM).toString());
    t->setLED1Enabled(s.value(g, SETTING_ENABLED).toBool());

    g = SETTINGSGROUP_LED2;
    t->setLED2Term(s.value(g, SETTING_TERM).toString());
    t->setLED2Enabled(s.value(g, SETTING_ENABLED).toBool());

    g = SETTINGSGROUP_ELREADOUT;
    t->setElectrodeReadoutPhysChan(s.value(g, SETTING_PHYSCHAN).toString());
    t->setElectrodeReadoutRate(s.value(g, SETTING_FREQ).toDouble());
    t->setElectrodeReadoutEnabled(s.value(g, SETTING_ENABLED).toBool());

    g = SETTINGSGROUP_STIMULATION;
    t->setStimulationHighTime(s.value(g, SETTING_HIGH_TIME).toDouble());
    t->setStimulationLowTime(s.value(g, SETTING_LOW_TIME).toDouble());
    t->setStimulationTerm(s.value(g, SETTING_TERM).toString());
    t->setStimulationEnabled(s.value(g, SETTING_ENABLED).toBool());
    t->setContinuousStimulation(s.value(g, SETTING_ALWAYS_ON).toBool());
    t->setAODEnabled(s.value(g, SETTING_AOD_ENABLED).toBool());

    g = SETTINGSGROUP_TIMING;
    t->setStimulationInitialDelay(s.value(g, SETTING_INITIALDELAY).toDouble());
    t->setStimulationDuration(s.value(g, SETTING_STIMDURATION).toDouble());
    optrode().setPostStimulation(s.value(g, SETTING_POSTSTIMULATION).toDouble());

    g = SETTINGSGROUP_ACQUISITION;
    optrode().setOutputDir(s.value(g, SETTING_OUTPUTPATH).toString());
    optrode().setRunName(s.value(g, SETTING_RUNNAME).toString());
    optrode().setSaveElectrodeEnabled(s.value(g, SETTING_SAVEELECTRODE).toBool());
    optrode().setSaveBehaviorEnabled(s.value(g, SETTING_SAVEBEHAVIOR).toBool());

    g = SETTINGSGROUP_BEHAVCAMROI;
    optrode().getBehaviorCamera()->setROI(s.value(g, SETTING_ROI).toRect());

    g = SETTINGSGROUP_ZAXIS;
    PIDevice *dev = optrode().getZAxis();
    dev->setBaud(s.value(g, SETTING_BAUD).toUInt());
    dev->setDeviceNumber(s.value(g, SETTING_DEVICENUMBER).toUInt());
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        if (info.manufacturer() == "PI" || info.description().startsWith("PI")) {
            dev->setPortName(info.portName());
            break;
        }
    }

    QSettings settings;

    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    restoreState(settings.value("mainWindowState").toByteArray());
    settings.endGroup();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
#ifdef WITH_HARDWARE
    QMessageBox::StandardButton ret = QMessageBox::question(
        this, QString("Closing %1").arg(PROGRAM_NAME),
        QString("Are you sure you want to close %1?").arg(PROGRAM_NAME),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (ret != QMessageBox::Yes) {
        e->ignore();
        return;
    }
#endif

    /* This will trigger saveSettings() in the relevant widgets (e.g. MainPage). This must happen
     * before MainWindow::saveSettings() is called. */
    delete centralWidget();

    saveSettings();

    /* uninitialize() is called in the receiver's thread (i.e. optrod's).
     * This ensures that QTimers can be correctly stopped (they can't be
     * stopped from a different thread), etc.
     * The invokation is blocking, so that there is no risk the object is
     * destroyed before uninitialize() is called.
     */

    QMetaObject::invokeMethod(&optrode(), "uninitialize",
                              Qt::BlockingQueuedConnection);
    QMainWindow::closeEvent(e);
    Spinnaker::System::GetInstance()->ReleaseInstance();
}

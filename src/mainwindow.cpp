#include <QAction>
#include <QMenuBar>
#include <QMessageBox>
#include <QTextStream>
#include <QSettings>
#include <QCloseEvent>
#include <QThread>

#include <Spinnaker.h>
#include "mainwindow.h"
#include "centralwidget.h"

#include "optrode.h"
#include "settings.h"
#include "tasks.h"

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

    QMenu *fileMenu = menuBar->addMenu("&File");
    fileMenu->addAction(saveSettingsAction);
    fileMenu->addAction(quitAction);

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
    s.setValue(SETTINGSGROUP_MAINTRIG, SETTING_PHYSCHAN, t->getMainTrigPhysChan());
    s.setValue(SETTINGSGROUP_MAINTRIG, SETTING_TERM, t->getMainTrigTerm());

    s.setValue(SETTINGSGROUP_BEHAVCAMROI, SETTING_ROI, optrode().getBehaviorCamera()->getROI());

    s.setValue(SETTINGSGROUP_LED1, SETTING_FREQ, t->getLEDFreq());
    s.setValue(SETTINGSGROUP_LED1, SETTING_PHYSCHAN, t->getLED1PhysChan());
    s.setValue(SETTINGSGROUP_LED1, SETTING_TERM, t->getLED1Term());
    s.setValue(SETTINGSGROUP_LED2, SETTING_PHYSCHAN, t->getLED2PhysChan());
    s.setValue(SETTINGSGROUP_LED2, SETTING_TERM, t->getLED2Term());

    s.setValue(SETTINGSGROUP_ELREADOUT, SETTING_PHYSCHAN, t->getElectrodeReadoutPhysChan());
    s.setValue(SETTINGSGROUP_ELREADOUT, SETTING_FREQ, t->getElectrodeReadoutRate());

    s.setValue(SETTINGSGROUP_SHUTTER, SETTING_PHYSCHAN, t->getShutterPulseCounter());
    s.setValue(SETTINGSGROUP_SHUTTER, SETTING_DUTY, t->getShutterPulseDuty());
    s.setValue(SETTINGSGROUP_SHUTTER, SETTING_FREQ, t->getShutterPulseFrequency());
    s.setValue(SETTINGSGROUP_SHUTTER, SETTING_TERM, t->getShutterPulseTerm());
    s.setValue(SETTINGSGROUP_SHUTTER, SETTING_ENABLED, t->getShutterPulseEnabled());

    s.setValue(SETTINGSGROUP_TIMING, SETTING_INITIALDELAY, t->getShutterInitialDelay());
    s.setValue(SETTINGSGROUP_TIMING, SETTING_STIMDURATION, t->stimulationDuration());
    s.setValue(SETTINGSGROUP_TIMING, SETTING_POSTSTIMULATION, optrode().getPostStimulation());

    s.setValue(SETTINGSGROUP_ACQUISITION, SETTING_OUTPUTPATH, optrode().getOutputDir());
    s.setValue(SETTINGSGROUP_ACQUISITION, SETTING_RUNNAME, optrode().getRunName());

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
    t->setMainTrigPhysChan(s.value(SETTINGSGROUP_MAINTRIG, SETTING_PHYSCHAN).toString());
    t->setMainTrigTerm(s.value(SETTINGSGROUP_MAINTRIG, SETTING_TERM).toString());

    t->setLEDFreq(s.value(SETTINGSGROUP_LED1, SETTING_FREQ).toDouble());
    t->setLED1PhysChan(s.value(SETTINGSGROUP_LED1, SETTING_PHYSCHAN).toString());
    t->setLED1Term(s.value(SETTINGSGROUP_LED1, SETTING_TERM).toString());
    t->setLED2PhysChan(s.value(SETTINGSGROUP_LED2, SETTING_PHYSCHAN).toString());
    t->setLED2Term(s.value(SETTINGSGROUP_LED2, SETTING_TERM).toString());

    t->setElectrodeReadoutPhysChan(s.value(SETTINGSGROUP_ELREADOUT, SETTING_PHYSCHAN).toString());
    t->setElectrodeReadoutRate(s.value(SETTINGSGROUP_ELREADOUT, SETTING_FREQ).toDouble());

    t->setShutterPulseCounter(s.value(SETTINGSGROUP_SHUTTER, SETTING_PHYSCHAN).toString());
    t->setShutterPulseDuty(s.value(SETTINGSGROUP_SHUTTER, SETTING_DUTY).toDouble());
    t->setShutterPulseFrequency(s.value(SETTINGSGROUP_SHUTTER, SETTING_FREQ).toDouble());
    t->setShutterPulseTerm(s.value(SETTINGSGROUP_SHUTTER, SETTING_TERM).toString());
    t->setShutterPulseEnabled(s.value(SETTINGSGROUP_SHUTTER, SETTING_ENABLED).toBool());

    t->setShutterInitialDelay(s.value(SETTINGSGROUP_TIMING, SETTING_INITIALDELAY).toDouble());
    t->setStimulationDuration(s.value(SETTINGSGROUP_TIMING, SETTING_STIMDURATION).toDouble());
    optrode().setPostStimulation(s.value(SETTINGSGROUP_TIMING, SETTING_POSTSTIMULATION).toDouble());

    optrode().setOutputDir(s.value(SETTINGSGROUP_ACQUISITION, SETTING_OUTPUTPATH).toString());
    optrode().setRunName(s.value(SETTINGSGROUP_ACQUISITION, SETTING_RUNNAME).toString());

    optrode().getBehaviorCamera()->setROI(s.value(SETTINGSGROUP_BEHAVCAMROI, SETTING_ROI).toRect());

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

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

    QState *rs = optrode().getState(Optrode::STATE_READY);
    QState *cs = optrode().getState(Optrode::STATE_CAPTURING);
    QState *us = optrode().getState(Optrode::STATE_UNINITIALIZED);

    rs->assignProperty(DDSDialogAction, "enabled", true);
    cs->assignProperty(DDSDialogAction, "enabled", false);
    us->assignProperty(DDSDialogAction, "enabled", false);

    DDSDialog *ddsDialog = new DDSDialog(this);

    connect(DDSDialogAction, &QAction::triggered, [ = ](){
        optrode().NITasks()->clearTasks();
        optrode().NITasks()->getDDS()->initTask();
        optrode().NITasks()->getDDS()->setWriteMode(DDS::WRITE_MODE_TO_NI_TASK);
        ddsDialog->show();
    });

    cs->assignProperty(ddsDialog, "visible", false);
    us->assignProperty(ddsDialog, "visible", false);

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
    QSettings settings;

    settings.beginGroup("MainWindow");
    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());
    settings.endGroup();
}

void MainWindow::loadSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    restoreState(settings.value("mainWindowState").toByteArray());
    settings.endGroup();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
#ifndef DEMO_MODE
    QMessageBox::StandardButton ret
        = QMessageBox::question(this,
                                QString("Closing %1").arg(PROGRAM_NAME),
                                QString("Are you sure you want to close %1?").arg(PROGRAM_NAME),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No);

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

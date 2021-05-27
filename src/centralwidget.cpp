#include <QHBoxLayout>

#include <qtlab/core/logmanager.h>
#include <qtlab/widgets/logwidget.h>

#include "mainpage.h"
#include "settingspage.h"
#include "centralwidget.h"

CentralWidget::CentralWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void CentralWidget::setupUi()
{
    tabWidget = new QTabWidget();

    tabWidget->addTab(new MainPage(), "Main");
    tabWidget->addTab(new SettingsPage(), "Settings");
    tabWidget->addTab(new LogWidget(), "Messages");
    logManager().flushMessages();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(tabWidget);
    setLayout(layout);
}

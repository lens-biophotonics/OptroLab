#include <QHBoxLayout>
#include <QPushButton>
#include <QSpinBox>

#include <qtlab/hw/pi-widgets/picontrollersettingswidget.h>

#include "settingspage.h"
#include "optrode.h"

#include "dds.h"

DDS *dds;

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent)
{
    dds = new DDS();

    setupUi();
}

void SettingsPage::setupUi()
{
    QBoxLayout *hLayout = new QHBoxLayout();
    QBoxLayout *vLayout = new QVBoxLayout();

    hLayout->addWidget(new PIControllerSettingsWidget(optrode().getZAxis()));
    hLayout->addStretch();

    vLayout->addLayout(hLayout);
    vLayout->addStretch();

    setLayout(vLayout);
}

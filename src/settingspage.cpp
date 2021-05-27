#include <QHBoxLayout>

#include <qtlab/widgets/picontrollersettingswidget.h>

#include "settingspage.h"
#include "optrode.h"

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent)
{
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

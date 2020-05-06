#include <QPushButton>
#include <QVBoxLayout>

#include "optrod.h"

#include "controlswidget.h"

ControlsWidget::ControlsWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void ControlsWidget::setupUi()
{
    QPushButton *initButton = new QPushButton("Initialize");
    connect(initButton, &QPushButton::clicked, &optrod(), &Optrod::initialize);

    QState *state = optrod().getState(Optrod::STATE_UNINITIALIZED);
    state->assignProperty(initButton, "enabled", true);

    state = optrod().getState(Optrod::STATE_READY);
    state->assignProperty(initButton, "enabled", false);

    QVBoxLayout *myLayout = new QVBoxLayout();
    myLayout->addWidget(initButton);

    setLayout(myLayout);
}

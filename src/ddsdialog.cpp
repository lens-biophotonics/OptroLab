#include <QPushButton>
#include <QGridLayout>
#include <QLabel>

#include <qwt_slider.h>

#include <qtlab/widgets/customspinbox.h>

#include "ddsdialog.h"
#include "optrode.h"
#include "dds.h"
#include "tasks.h"


DDSDialog::DDSDialog(QWidget *parent) : QDialog(parent)
{
    setupUi();
}

void DDSDialog::setupUi()
{
    setWindowTitle("DDS");

    // frequency
    QwtSlider *freqSlider1 = new QwtSlider(Qt::Horizontal, this);
    QwtSlider *freqSlider2 = new QwtSlider(Qt::Horizontal, this);

    freqSlider1->setScalePosition(QwtSlider::NoScale);
    freqSlider1->setLowerBound(0);
    freqSlider1->setUpperBound(150);
    freqSlider1->setMinimumWidth(300);

    freqSlider2->setScalePosition(QwtSlider::NoScale);
    freqSlider2->setLowerBound(0);
    freqSlider2->setUpperBound(150);

    DoubleSpinBox *freqSpinBox1 = new DoubleSpinBox();
    freqSpinBox1->setRange(freqSlider1->minimum(), freqSlider1->maximum());
    freqSpinBox1->setDecimals(3);
    freqSpinBox1->setSuffix("MHz");

    DoubleSpinBox *freqSpinBox2 = new DoubleSpinBox();
    freqSpinBox2->setRange(freqSlider2->minimum(), freqSlider2->maximum());
    freqSpinBox2->setDecimals(3);
    freqSpinBox2->setSuffix("MHz");


    // amplitude
    QwtSlider *ampSlider = new QwtSlider(Qt::Horizontal, this);

    ampSlider->setScalePosition(QwtSlider::NoScale);
    ampSlider->setLowerBound(0);
    ampSlider->setUpperBound(4095);

    SpinBox *ampSpinBox = new SpinBox();
    ampSpinBox->setRange(ampSlider->minimum(), ampSlider->maximum());

    QPushButton *masterResetPushButton = new QPushButton("Master reset");
    masterResetPushButton->setAutoDefault(false);

    QGridLayout *grid = new QGridLayout();
    int row = 0;
    int col = 0;
    grid->addWidget(new QLabel("Frequency"), row++, col);
    grid->addWidget(freqSlider1, row, col++);
    grid->addWidget(freqSpinBox1, row++, col++);
    col = 0;
    grid->addWidget(freqSlider2, row, col++);
    grid->addWidget(freqSpinBox2, row++, col++);
    col = 0;
    grid->addWidget(new QLabel("Amplitude"), row++, col);
    grid->addWidget(ampSlider, row, col++);
    grid->addWidget(ampSpinBox, row++, col++);
    col = 0;
    grid->addWidget(masterResetPushButton, row++, col, 1, 2);

    setLayout(grid);

    QList<QWidget *> wList;
    wList << freqSlider1 << freqSlider2 << freqSpinBox1 << freqSpinBox2;
    wList << ampSlider << ampSpinBox;
    wList << masterResetPushButton;

    QState *rs = optrode().getState(Optrode::STATE_READY);
    QState *cs = optrode().getState(Optrode::STATE_CAPTURING);
    QState *us = optrode().getState(Optrode::STATE_UNINITIALIZED);

    for (QWidget *w : wList) {
        rs->assignProperty(w, "enabled", true);
        cs->assignProperty(w, "enabled", false);
        us->assignProperty(w, "enabled", false);
    }

    DDS *dds = optrode().NITasks()->getDDS();

    // frequency
    connect(freqSlider1, &QwtSlider::valueChanged, this, [ = ](double value){
        freqSpinBox1->setValue(value);
        dds->setFrequency1(value, freqSpinBox2->value());
        dds->udclkPulse();
    });

    connect(freqSlider2, &QwtSlider::valueChanged, this, [ = ](double value){
        freqSpinBox2->setValue(value);
        dds->setFrequency1(freqSpinBox1->value(), value);
        dds->udclkPulse();
    });

    connect(freqSpinBox1, &DoubleSpinBox::returnPressed, this, [ = ](){
        freqSlider1->setValue(freqSpinBox1->value());
    });

    connect(freqSpinBox2, &DoubleSpinBox::returnPressed, this, [ = ](){
        freqSlider2->setValue(freqSpinBox2->value());
    });

    // amplitude
    connect(ampSlider, &QwtSlider::valueChanged, this, [ = ](double value){
        ampSpinBox->setValue(value);
        dds->setOSKI(value, value);
        dds->udclkPulse();
    });

    connect(ampSpinBox, &SpinBox::returnPressed, this, [ = ](){
        ampSlider->setValue(ampSpinBox->value());
    });

    connect(masterResetPushButton, &QPushButton::clicked,
            optrode().NITasks(), &Tasks::ddsMasterReset);
}

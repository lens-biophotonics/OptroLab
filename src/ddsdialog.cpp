#include <QPushButton>
#include <QGridLayout>
#include <QLabel>

#include <qwt_slider.h>

#include <qtlab/widgets/customspinbox.h>

#include "ddsdialog.h"
#include "optrode.h"
#include "dds.h"


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
    QwtSlider *ampSlider1 = new QwtSlider(Qt::Horizontal, this);
    QwtSlider *ampSlider2 = new QwtSlider(Qt::Horizontal, this);

    ampSlider1->setScalePosition(QwtSlider::NoScale);
    ampSlider1->setLowerBound(0);
    ampSlider1->setUpperBound(4095);

    ampSlider2->setScalePosition(QwtSlider::NoScale);
    ampSlider2->setLowerBound(0);
    ampSlider2->setUpperBound(4095);

    SpinBox *ampSpinBox1 = new SpinBox();
    ampSpinBox1->setRange(ampSlider1->minimum(), ampSlider1->maximum());

    SpinBox *ampSpinBox2 = new SpinBox();
    ampSpinBox2->setRange(ampSlider2->minimum(), ampSlider2->maximum());

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
    grid->addWidget(ampSlider1, row, col++);
    grid->addWidget(ampSpinBox1, row++, col++);
    col = 0;
    grid->addWidget(ampSlider2, row, col++);
    grid->addWidget(ampSpinBox2, row++, col++);
    col = 0;
    grid->addWidget(masterResetPushButton, row++, col, 1, 2);

    setLayout(grid);

    QList<QWidget *> wList;
    wList << freqSlider1 << freqSlider2 << freqSpinBox1 << freqSpinBox2;

    QState *rs = optrode().getState(Optrode::STATE_READY);
    QState *cs = optrode().getState(Optrode::STATE_CAPTURING);
    QState *us = optrode().getState(Optrode::STATE_UNINITIALIZED);

    for (QWidget *w : wList) {
        rs->assignProperty(w, "enabled", true);
        cs->assignProperty(w, "enabled", false);
        us->assignProperty(w, "enabled", false);
    }

    DDS *dds = optrode().getDDS();

    // frequency
    connect(freqSlider1, &QwtSlider::valueChanged, this, [ = ](double value){
        freqSpinBox1->setValue(value);
        dds->setFrequency1(value, freqSpinBox2->value());
    });

    connect(freqSlider2, &QwtSlider::valueChanged, this, [ = ](double value){
        freqSpinBox2->setValue(value);
        dds->setFrequency1(freqSpinBox1->value(), value);
    });

    connect(freqSpinBox1, &DoubleSpinBox::returnPressed, this, [ = ](){
        dds->setFrequency1(freqSpinBox1->value(), freqSpinBox2->value());
    });

    connect(freqSpinBox2, &DoubleSpinBox::returnPressed, this, [ = ](){
        dds->setFrequency1(freqSpinBox1->value(), freqSpinBox2->value());
    });

    // amplitude
    connect(ampSlider1, &QwtSlider::valueChanged, this, [ = ](double value){
        ampSpinBox1->setValue(value);
        dds->setOSKI(value, ampSpinBox2->value());
    });

    connect(ampSlider2, &QwtSlider::valueChanged, this, [ = ](double value){
        ampSpinBox2->setValue(value);
        dds->setOSKI(ampSpinBox1->value(), value);
    });

    connect(ampSpinBox1, &SpinBox::returnPressed, this, [ = ](){
        dds->setOSKI(ampSpinBox1->value(), ampSpinBox2->value());
    });

    connect(ampSpinBox2, &SpinBox::returnPressed, this, [ = ](){
        dds->setOSKI(ampSpinBox1->value(), ampSpinBox2->value());
    });

    connect(masterResetPushButton, &QPushButton::clicked, &optrode(), &Optrode::ddsMasterReset);
}

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>

#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_symbol.h>

#include <qtlab/widgets/cameraplot.h>

#include "camdisplay.h"

class MapPicker : public QwtPlotPicker
{
public:
    MapPicker(QWidget *canvas) :
        QwtPlotPicker(canvas)
    {
        setTrackerMode(AlwaysOn);
    }

    virtual QwtText trackerTextF(const QPointF &pos) const
    {
        QColor bg(Qt::white);
        bg.setAlpha(200);

        QwtText text = QString("%1, %2").arg(pos.x()).arg(pos.y());
        text.setBackgroundBrush(QBrush(bg));
        return text;
    }
};

CamDisplay::CamDisplay(QWidget *parent) : CameraDisplay(parent)
{
    MapPicker *picker = new MapPicker(getPlot()->canvas());
    picker->setStateMachine(new QwtPickerClickPointMachine());
    picker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::ShiftModifier);

    connect(picker, qOverload<const QPointF &>(&MapPicker::selected), this, &CamDisplay::addPoint);

    menu->addSeparator();

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Add marker");

    QGridLayout *grid = new QGridLayout();
    int row = 0;
    grid->addWidget(new QLabel("X"), row, 0);
    QSpinBox *xSpinBox = new QSpinBox();
    grid->addWidget(xSpinBox, row++, 1);

    grid->addWidget(new QLabel("Y"), row, 0);
    QSpinBox *ySpinBox = new QSpinBox();
    grid->addWidget(ySpinBox, row++, 1);

    QPushButton *setPushButton = new QPushButton("Set marker");
    grid->addWidget(setPushButton, row++, 0, 1, 2);

    grid->setColumnStretch(1, 2);

    dialog->setLayout(grid);

    connect(setPushButton, &QPushButton::clicked, [ = ](){
        addPoint(QPoint(xSpinBox->value(), ySpinBox->value()));
    });

    QAction *setMarker = new QAction("Set marker...");
    connect(setMarker, &QAction::triggered, [ = ](){
        xSpinBox->setRange(0, plot->getPlotSize().width());
        ySpinBox->setRange(0, plot->getPlotSize().height());
        dialog->show();
    });

    menu->addAction(setMarker);

    QAction *clearMarkersAction = new QAction("Clear markers");
    connect(clearMarkersAction, &QAction::triggered, this, &CamDisplay::clearMarkers);
    menu->addAction(clearMarkersAction);
}

void CamDisplay::addPoint(const QPointF &p)
{
    if (points.size() == 1) {
        clearMarkers();
    }
    QwtPlotMarker *marker = new QwtPlotMarker();

    QwtSymbol *symbol = new QwtSymbol(QwtSymbol::Cross);
    symbol->setSize(20);
    symbol->setPen(QColor(0xff, 0xaa, 0x00));
    marker->setSymbol(symbol);

    marker->setLineStyle((QwtPlotMarker::LineStyle)(QwtPlotMarker::NoLine));
    marker->setValue(p);
    marker->setLabelAlignment((Qt::Alignment) (Qt::AlignLeft | Qt::AlignTop));
    marker->attach(plot);
    plot->replot();

    points.append(p);
    markers.append(marker);
}

QVector<QPointF> CamDisplay::getPoints() const
{
    return points;
}

void CamDisplay::clearMarkers()
{
    points.clear();
    for (QwtPlotMarker *m : markers) {
        m->detach();
    }
    markers.clear();
    getPlot()->replot();
}

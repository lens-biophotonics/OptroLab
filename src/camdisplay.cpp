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

    connect(picker, qOverload<const QPointF &>(&MapPicker::selected), this, [ = ](QPointF p) {
        if (points.size() == 1) {
            clearMarkers();
        }
        addPoint(p);
    });

    menu->addSeparator();

    QAction *clearMarkersAction = new QAction("Clear markers");
    connect(clearMarkersAction, &QAction::triggered, this, &CamDisplay::clearMarkers);
    menu->addAction(clearMarkersAction);
}

void CamDisplay::addPoint(const QPointF &p)
{
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

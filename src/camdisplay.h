#ifndef CAMDISPLAY_H
#define CAMDISPLAY_H

#include <qtlab/widgets/cameradisplay.h>


class CamDisplay : public CameraDisplay
{
public:
    CamDisplay(QWidget *parent);

    void addPoint(const QPointF &p);
    QVector<QPointF> getPoints() const;

private:
    QVector<QPointF> points;
    QVector<QwtPlotMarker *> markers;

    void clearMarkers();
};

#endif // CAMDISPLAY_H

#ifndef CONTROLSWIDGET_H
#define CONTROLSWIDGET_H

#include <QWidget>

class CamDisplay;

class ControlsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ControlsWidget(QWidget *parent = nullptr);

    void setCamDisplay(CamDisplay *value);

signals:

private:
    void setupUi();

    CamDisplay *camDisplay;
};

#endif // CONTROLSWIDGET_H

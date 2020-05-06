#ifndef CONTROLSWIDGET_H
#define CONTROLSWIDGET_H

#include <QWidget>

class ControlsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ControlsWidget(QWidget *parent = nullptr);

signals:

private:
    void setupUi();

};

#endif // CONTROLSWIDGET_H

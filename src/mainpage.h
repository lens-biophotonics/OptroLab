#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <qtlab/hw/pi-widgets/pipositioncontrolwidget.h>

#include <QWidget>

class PixmapWidget;

class MainPage : public QWidget
{
    Q_OBJECT
public:
    explicit MainPage(QWidget *parent = nullptr);
    virtual ~MainPage();

signals:

private:
    void setupUi();
    void saveSettings();
    PixmapWidget *pmw;
    PIPositionControlWidget *posCW;
};

#endif // MAINPAGE_H

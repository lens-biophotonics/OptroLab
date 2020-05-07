#ifndef MAINPAGE_H
#define MAINPAGE_H

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
};

#endif // MAINPAGE_H

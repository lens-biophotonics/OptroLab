#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>

class MainPage : public QWidget
{
    Q_OBJECT
public:
    explicit MainPage(QWidget *parent = nullptr);

signals:

private:
    void setupUi();
};

#endif // MAINPAGE_H

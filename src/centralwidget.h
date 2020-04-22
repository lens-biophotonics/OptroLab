#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>
#include <QTabWidget>

class CentralWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CentralWidget(QWidget *parent = nullptr);

signals:

public slots:

private:
    QTabWidget *tabWidget;

    void setupUi();
};

#endif // CENTRALWIDGET_H

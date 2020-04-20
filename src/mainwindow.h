#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

signals:

public slots:

private slots:
    void on_aboutAction_triggered() const;

private:
    void setupUi();
};

#endif // MAINWINDOW_H

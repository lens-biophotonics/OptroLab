#ifndef DDSDIALOG_H
#define DDSDIALOG_H

#include <QDialog>


class DDSDialog : public QDialog
{
    Q_OBJECT

public:
    DDSDialog(QWidget *parent = nullptr);

private:
    void setupUi();
};

#endif // DDSDIALOG_H

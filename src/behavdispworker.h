#ifndef BEHAVDISPWORKER_H
#define BEHAVDISPWORKER_H

#include <QThread>
#include <QPixmap>

class ChameleonCamera;


class BehavDispWorker : public QThread
{
    Q_OBJECT
public:
    explicit BehavDispWorker(ChameleonCamera *camera, QObject *parent = nullptr);

protected:
    virtual void run();

signals:
    void newImage(const QPixmap &pm);

private:
    ChameleonCamera *camera;
    bool stop;
};

#endif // BEHAVDISPWORKER_H

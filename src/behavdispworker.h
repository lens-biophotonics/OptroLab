#ifndef BEHAVDISPWORKER_H
#define BEHAVDISPWORKER_H

#include <QThread>
#include <QPixmap>

namespace Aravis {
class Camera;
}


class BehavDispWorker : public QThread
{
    Q_OBJECT
public:
    explicit BehavDispWorker(Aravis::Camera *camera, QObject *parent = nullptr);

protected:
    virtual void run();

signals:
    void newImage(const QPixmap &pm);

private:
    Aravis::Camera *camera;
    bool stop;
};

#endif // BEHAVDISPWORKER_H

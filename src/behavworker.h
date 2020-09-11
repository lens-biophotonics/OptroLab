#ifndef BEHAVWORKER_H
#define BEHAVWORKER_H

#include <QThread>
#include <QPixmap>

class ChameleonCamera;


class BehavWorker : public QThread
{
    Q_OBJECT
public:
    explicit BehavWorker(ChameleonCamera *camera, QObject *parent = nullptr);

    void setSaveToFileEnabled(bool value);

    void setOutputFile(const QString &value);

    void setFrameCount(int value);

protected:
    virtual void run();

signals:
    void newImage(const QPixmap &pm);
    void captureCompleted(bool ok);

private:
    ChameleonCamera *camera;
    QString outputFile;
    bool stop, saveToFileEnabled = false;
    size_t frameCount;
};

#endif // BEHAVWORKER_H

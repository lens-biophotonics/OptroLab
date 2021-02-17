#ifndef BEHAVWORKER_H
#define BEHAVWORKER_H

#include <QPixmap>

class ChameleonCamera;


class BehavWorker : public QObject
{
    Q_OBJECT
public:
    explicit BehavWorker(ChameleonCamera *camera, QObject *parent = nullptr);

    void setSaveToFileEnabled(bool value);

    void setOutputFile(const QString &value);

    void setFrameCount(int value);

signals:
    void newImage(const QPixmap &pm);
    void captureCompleted(bool ok);

private:
    void start();
    ChameleonCamera *camera;
    QString outputFile;
    bool stop, saveToFileEnabled = false;
    size_t frameCount;
};

#endif // BEHAVWORKER_H

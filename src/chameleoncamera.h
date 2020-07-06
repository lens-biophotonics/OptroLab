#ifndef CHAMELEONCAMERA_H
#define CHAMELEONCAMERA_H

#include <Spinnaker.h>
#include <SpinGenApi/SpinnakerGenApi.h>

#include <QObject>

class ChameleonCamera : public QObject
{
    Q_OBJECT
public:
    explicit ChameleonCamera(QObject *parent = nullptr);
    virtual ~ChameleonCamera();

    void open(uint index);
    void logDeviceInfo();
    Spinnaker::ImagePtr getNextImage(uint64_t timeout);
    QSize imageSize();

public slots:
    void startAcquisition();
    void stopAcquisition();

signals:
    void acquisitionStarted();
    void acquisitionStopped();
private:
    Spinnaker::CameraPtr pCam;
    bool capturing;

    void setupAcquisitionMode();
};

#endif // CHAMELEONCAMERA_H

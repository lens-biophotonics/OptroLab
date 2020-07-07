#include <stdexcept>

#include <QSize>

#include <qtlab/core/logmanager.h>

#include <SpinGenApi/SpinnakerGenApi.h>

#include "chameleoncamera.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;

static Logger *logger = logManager().getLogger("ChameleonCamera");
static SystemPtr sys = System::GetInstance();

ChameleonCamera::ChameleonCamera(QObject *parent) : QObject(parent)
{
    pCam = nullptr;
    capturing = false;
}

ChameleonCamera::~ChameleonCamera()
{
    if (capturing)
        stopAcquisition();
    if (pCam)
        pCam->DeInit();
}

void ChameleonCamera::open(uint index)
{
    CameraList camList = sys->GetCameras();
    pCam = camList.GetByIndex(index);
    camList.Clear();
    pCam->Init();
    setupAcquisitionMode();
}

void ChameleonCamera::logDeviceInfo()
{
    FeatureList_t features;
    CCategoryPtr category = pCam->GetTLDeviceNodeMap().GetNode("DeviceInformation");
    if (IsAvailable(category) && IsReadable(category))
    {
        category->GetFeatures(features);

        FeatureList_t::const_iterator it;
        for (it = features.begin(); it != features.end(); ++it)
        {
            CNodePtr pfeatureNode = *it;
            CValuePtr pValue = (CValuePtr)pfeatureNode;
            logger->info(QString("%1: %2")
                         .arg(pfeatureNode->GetName().c_str())
                         .arg(IsReadable(pValue) ? pValue->ToString().c_str() : "Node not readable"));
        }
    }
}

ImagePtr ChameleonCamera::getNextImage(uint64_t timeout)
{
    return pCam->GetNextImage(timeout);
}

QSize ChameleonCamera::imageSize()
{
    return QSize(pCam->Width.GetValue(), pCam->Height.GetValue());
}

void ChameleonCamera::startAcquisition()
{
    pCam->BeginAcquisition();
    capturing = true;
    emit acquisitionStarted();
}

void ChameleonCamera::stopAcquisition()
{
    pCam->EndAcquisition();
    capturing = false;
    emit acquisitionStopped();
}

void ChameleonCamera::setupAcquisitionMode()
{
    pCam->AcquisitionMode.SetValue(AcquisitionMode_Continuous);
    pCam->PixelFormat.SetValue(PixelFormat_Mono8);

    pCam->TriggerMode.SetValue(TriggerMode_Off);
#ifdef WITH_HARDWARE
    pCam->TriggerSelector.SetValue(TriggerSelector_FrameStart);
    pCam->TriggerSource.SetValue(TriggerSource_Line2);
    pCam->TriggerActivation.SetValue(TriggerActivation_RisingEdge);
    pCam->TriggerMode.SetValue(TriggerMode_On);
#endif
}

#include <stdexcept>

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
    pCam->GetTLDeviceNodeMap();
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
    ImagePtr img = nullptr;
    try {
        img = pCam->GetNextImage(timeout);
    }
    catch (Spinnaker::Exception e) {
    }
    return img;
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
    CEnumerationPtr ptrAcquisitionMode = pCam->GetNodeMap().GetNode("AcquisitionMode");
    if (!IsAvailable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode)) {
        throw std::runtime_error("ChameleonCamera: Unable to write to AcquisitionMode node");
    }
    CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
    ptrAcquisitionMode->SetIntValue(ptrAcquisitionModeContinuous->GetValue());

    CEnumerationPtr ptrPixelFormat = pCam->GetNodeMap().GetNode("PixelFormat");
    if (!IsAvailable(ptrPixelFormat) || !IsWritable(ptrPixelFormat)) {
        throw std::runtime_error("ChameleonCamera: Unable to write to PixelFormat node");
    }
    CEnumEntryPtr ptrPxfmtMono8 = ptrPixelFormat->GetEntryByName("Mono8");
    ptrPixelFormat->SetIntValue(ptrPxfmtMono8->GetValue());
}
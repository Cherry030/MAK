#pragma once

#include <vl/interactionEncoder.h>

class extDeviceState;

#define DtDECLARE_extDeviceState_PARAM_ENCODER(paramName) \
   DtDECLARE_PARAM_ENCODER(extDeviceState, paramName)

#define DtDEFINE_SIMPLE_extDeviceState_PARAM_ENCODER(paramName, netType, inspector) \
   DtDEFINE_SIMPLE_PARAM_ENCODER(extDeviceStateEncoder, extDeviceState, paramName, netType, inspector)

class extDeviceStateEncoder : public DtInteractionEncoder
{
public:

    //! Constructor
    extDeviceStateEncoder(DtExerciseConn* exConn, DtInterClassDesc* classDesc);

    //! Destructor
    virtual ~extDeviceStateEncoder();

protected:
   // DtDECLARE_extDeviceState_PARAM_ENCODER(publisherType);
   // DtDECLARE_extDeviceState_PARAM_ENCODER(deviceID);
    DtDECLARE_extDeviceState_PARAM_ENCODER(PublisherType);
    DtDECLARE_extDeviceState_PARAM_ENCODER(DeviceID);

    DtDECLARE_extDeviceState_PARAM_ENCODER(TreadmillStatus);
    DtDECLARE_extDeviceState_PARAM_ENCODER(ManipulatorStatus);
    DtDECLARE_extDeviceState_PARAM_ENCODER(MotionTrackingStatus);
    DtDECLARE_extDeviceState_PARAM_ENCODER(MockupGunStatus);
    DtDECLARE_extDeviceState_PARAM_ENCODER(HapticSuiteStatus);
    DtDECLARE_extDeviceState_PARAM_ENCODER(WearablePatchStatus);
    DtDECLARE_extDeviceState_PARAM_ENCODER(MoneuverAssistantStatus);

    DtDECLARE_extDeviceState_PARAM_ENCODER(MotionDriver);
    DtDECLARE_extDeviceState_PARAM_ENCODER(MotionGunner);
    DtDECLARE_extDeviceState_PARAM_ENCODER(MotionBoarding);
    DtDECLARE_extDeviceState_PARAM_ENCODER(DriverSide);
    DtDECLARE_extDeviceState_PARAM_ENCODER(GunnerSide);
    DtDECLARE_extDeviceState_PARAM_ENCODER(BoardingSide);
};


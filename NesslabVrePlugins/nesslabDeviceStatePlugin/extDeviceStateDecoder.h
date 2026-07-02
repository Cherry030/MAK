#pragma once

#include <vl/interactionDecoder.h>

class extDeviceState;

#define DtDECLARE_extDeviceState_PARAM_DECODER(paramName) \
   DtDECLARE_PARAM_DECODER(extDeviceState, paramName)

#define DtDEFINE_SIMPLE_extDeviceState_PARAM_DECODER(paramName, netType, mutator) \
   DtDEFINE_SIMPLE_PARAM_DECODER(extDeviceStateDecoder, extDeviceState, paramName, netType, mutator)

#define DtDEFINE_SIMPLE_extDeviceState_PARAM_DECODER_WITH_CAST( paramName, netType, mutator, castExpr) \
   DtDEFINE_SIMPLE_PARAM_DECODER_WITH_CAST( extDeviceStateDecoder, extDeviceState, paramName, netType, mutator, castExpr)

class extDeviceStateDecoder : public DtInteractionDecoder
{
public:

    //! Constructor
    extDeviceStateDecoder(DtExerciseConn* exConn, DtInterClassDesc* classDesc);

    //! Destructor
    virtual ~extDeviceStateDecoder();

protected:
    DtDECLARE_extDeviceState_PARAM_DECODER(publisherType);
    DtDECLARE_extDeviceState_PARAM_DECODER(deviceID);
    DtDECLARE_extDeviceState_PARAM_DECODER(TreadmillStatus);
    DtDECLARE_extDeviceState_PARAM_DECODER(ManipulatorStatus);
    DtDECLARE_extDeviceState_PARAM_DECODER(MotionTrackingStatus);
    DtDECLARE_extDeviceState_PARAM_DECODER(MockupGunStatus);
    DtDECLARE_extDeviceState_PARAM_DECODER(HapticSuiteStatus);
    DtDECLARE_extDeviceState_PARAM_DECODER(WearablePatchStatus);
    DtDECLARE_extDeviceState_PARAM_DECODER(MoneuverAssistantStatus);

    DtDECLARE_extDeviceState_PARAM_DECODER(MotionDriver);
    DtDECLARE_extDeviceState_PARAM_DECODER(MotionGunner);
    DtDECLARE_extDeviceState_PARAM_DECODER(MotionBoarding);
    DtDECLARE_extDeviceState_PARAM_DECODER(DriverSide);
    DtDECLARE_extDeviceState_PARAM_DECODER(GunnerSide);
    DtDECLARE_extDeviceState_PARAM_DECODER(BoardingSide);

    //DtDECLARE_extDeviceState_PARAM_DECODER(DeviceStatus);
};


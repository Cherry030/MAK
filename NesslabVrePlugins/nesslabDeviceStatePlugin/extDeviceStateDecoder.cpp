#include "extDeviceStateDecoder.h"
#include "extDeviceState.h"

extDeviceStateDecoder::extDeviceStateDecoder(
	DtExerciseConn* exConn, DtInterClassDesc* classDesc) :
	DtInteractionDecoder(exConn, classDesc)
{
	DtADD_PARAM_DECODER(publisherType);
	DtADD_PARAM_DECODER(deviceID);

	DtADD_PARAM_DECODER(TreadmillStatus);
	DtADD_PARAM_DECODER(ManipulatorStatus);
	DtADD_PARAM_DECODER(MotionTrackingStatus);
	DtADD_PARAM_DECODER(MockupGunStatus);
	DtADD_PARAM_DECODER(HapticSuiteStatus);
	DtADD_PARAM_DECODER(WearablePatchStatus);
	DtADD_PARAM_DECODER(MoneuverAssistantStatus);

	DtADD_PARAM_DECODER(MotionDriver);
	DtADD_PARAM_DECODER(MotionGunner);
	DtADD_PARAM_DECODER(MotionBoarding);
	DtADD_PARAM_DECODER(DriverSide);
	DtADD_PARAM_DECODER(GunnerSide);
	DtADD_PARAM_DECODER(BoardingSide);
}

extDeviceStateDecoder::~extDeviceStateDecoder()
{
}

DtDEFINE_SIMPLE_extDeviceState_PARAM_DECODER(publisherType, DtNetU16, setPublisherType);
DtDEFINE_SIMPLE_extDeviceState_PARAM_DECODER(deviceID, DtNetU16, setDeviceID);

DtDEFINE_SIMPLE_extDeviceState_PARAM_DECODER(TreadmillStatus, DtNet8Bits, setTreadmill);
DtDEFINE_SIMPLE_extDeviceState_PARAM_DECODER(ManipulatorStatus, DtNet8Bits, setMainpulator);
DtDEFINE_SIMPLE_extDeviceState_PARAM_DECODER(MotionTrackingStatus, DtNet8Bits, setMotionTracking);
DtDEFINE_SIMPLE_extDeviceState_PARAM_DECODER(MockupGunStatus, DtNet8Bits, setMockupGun);
DtDEFINE_SIMPLE_extDeviceState_PARAM_DECODER(HapticSuiteStatus, DtNet8Bits, setHapticSuite);
DtDEFINE_SIMPLE_extDeviceState_PARAM_DECODER(WearablePatchStatus, DtNet8Bits, setWearblePatch);
DtDEFINE_SIMPLE_extDeviceState_PARAM_DECODER(MoneuverAssistantStatus , DtNet8Bits, setMoneuverAssistant);

DtDEFINE_SIMPLE_extDeviceState_PARAM_DECODER(MotionDriver, DtNet8Bits, setMotionDriver);
DtDEFINE_SIMPLE_extDeviceState_PARAM_DECODER(MotionGunner, DtNet8Bits, setMotionGunner);
DtDEFINE_SIMPLE_extDeviceState_PARAM_DECODER(MotionBoarding, DtNet8Bits, setMotionBoarding);
DtDEFINE_SIMPLE_extDeviceState_PARAM_DECODER(DriverSide, DtNet8Bits, setDriverSide);
DtDEFINE_SIMPLE_extDeviceState_PARAM_DECODER(GunnerSide, DtNet8Bits, setGunnerSide);
DtDEFINE_SIMPLE_extDeviceState_PARAM_DECODER(BoardingSide, DtNet8Bits, setBoardingSide);

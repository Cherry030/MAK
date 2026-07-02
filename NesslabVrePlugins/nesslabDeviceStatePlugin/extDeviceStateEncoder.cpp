#include "extDeviceStateEncoder.h"
#include "extDeviceState.h"

extDeviceStateEncoder::extDeviceStateEncoder(
	DtExerciseConn* exConn, DtInterClassDesc* classDesc) :
	DtInteractionEncoder(exConn, classDesc)
{
	DtADD_PARAM_ENCODER(PublisherType);
	DtADD_PARAM_ENCODER(DeviceID);

	DtADD_PARAM_ENCODER(TreadmillStatus);
	DtADD_PARAM_ENCODER(ManipulatorStatus);
	DtADD_PARAM_ENCODER(MotionTrackingStatus);
	DtADD_PARAM_ENCODER(MockupGunStatus);
	DtADD_PARAM_ENCODER(HapticSuiteStatus);
	DtADD_PARAM_ENCODER(WearablePatchStatus);
	DtADD_PARAM_ENCODER(MoneuverAssistantStatus);

	DtADD_PARAM_ENCODER(MotionDriver);
	DtADD_PARAM_ENCODER(MotionGunner);
	DtADD_PARAM_ENCODER(MotionBoarding);
	DtADD_PARAM_ENCODER(DriverSide);
	DtADD_PARAM_ENCODER(GunnerSide);
	DtADD_PARAM_ENCODER(BoardingSide);
	//DtADD_PARAM_ENCODER(DeviceStatus);
}

extDeviceStateEncoder::~extDeviceStateEncoder()
{
}

DtDEFINE_SIMPLE_extDeviceState_PARAM_ENCODER(PublisherType, DtNetU16, publisherType);
DtDEFINE_SIMPLE_extDeviceState_PARAM_ENCODER(DeviceID, DtNetU16, deviceID);

DtDEFINE_SIMPLE_extDeviceState_PARAM_ENCODER(TreadmillStatus, DtNet8Bits, getTreadmillStatus);
DtDEFINE_SIMPLE_extDeviceState_PARAM_ENCODER(ManipulatorStatus, DtNet8Bits, getMainpulatorStatus);
DtDEFINE_SIMPLE_extDeviceState_PARAM_ENCODER(MotionTrackingStatus, DtNet8Bits, getMotionTrackingStatus);
DtDEFINE_SIMPLE_extDeviceState_PARAM_ENCODER(MockupGunStatus, DtNet8Bits, getMockupGunStatus);
DtDEFINE_SIMPLE_extDeviceState_PARAM_ENCODER(HapticSuiteStatus, DtNet8Bits, getHapticSuiteStatus);
DtDEFINE_SIMPLE_extDeviceState_PARAM_ENCODER(WearablePatchStatus, DtNet8Bits, getWearblePatchStatus);
DtDEFINE_SIMPLE_extDeviceState_PARAM_ENCODER(MoneuverAssistantStatus, DtNet8Bits, getMoneuverAssistantStatus);

DtDEFINE_SIMPLE_extDeviceState_PARAM_ENCODER(MotionDriver, DtNet8Bits, getMotionDriverStatus);
DtDEFINE_SIMPLE_extDeviceState_PARAM_ENCODER(MotionGunner, DtNet8Bits, getMotionGunnerStatus);
DtDEFINE_SIMPLE_extDeviceState_PARAM_ENCODER(MotionBoarding, DtNet8Bits, getMotionBoardingStatus);
DtDEFINE_SIMPLE_extDeviceState_PARAM_ENCODER(DriverSide, DtNet8Bits, getDriverSideStatus);
DtDEFINE_SIMPLE_extDeviceState_PARAM_ENCODER(GunnerSide, DtNet8Bits, getGunnerSideStatus);
DtDEFINE_SIMPLE_extDeviceState_PARAM_ENCODER(BoardingSide, DtNet8Bits, getBoardingSideStatus);
//DtDEFINE_SIMPLE_extDeviceState_PARAM_ENCODER(DeviceStatus, DtNetInt32, DeviceStatus);
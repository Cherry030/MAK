#include "extDeviceState.h"

#include <vl/interactionFactory.h>

// Header files for encoder and decoder.
#include "extDeviceStateEncoder.h"
#include "extDeviceStateDecoder.h"

extDeviceState::extDeviceState() :
	DtInteractionWithEncDec()
{
	m_usPublisherType = m_usDeivceID = 0;

	m_Treadmill = m_Mainpulator = m_MotionTracking = m_MockupGun = m_HapticSuite = m_WearblePatch = m_MoneuverAssistant = 0;

	m_MotionDriver = m_MotionGunner = m_MotionBoarding = m_DriverSide = m_GunnerSide = m_BoardingSide = 0;
}

extDeviceState::~extDeviceState()
{
}

extDeviceState::extDeviceState(const extDeviceState& orig) :
	DtInteractionWithEncDec()
{
	m_usPublisherType = orig.publisherType();
	m_usDeivceID = orig.deviceID();
	m_Treadmill = orig.getTreadmillStatus();
	m_Mainpulator = orig.getMainpulatorStatus();
	m_MotionTracking = orig.getMotionTrackingStatus();
	m_MockupGun = orig.getMockupGunStatus();
	m_HapticSuite = orig.getHapticSuiteStatus();
	m_WearblePatch = orig.getWearblePatchStatus();
	m_MoneuverAssistant = orig.getMoneuverAssistantStatus();
	//m_nDeviceStatus = orig.DeviceStatus();
}

extDeviceState& extDeviceState::operator=(const extDeviceState& orig)
{
	if (this == &orig)
	{
		return *this;
	}

	DtInteractionWithEncDec::operator=(orig);

	m_usPublisherType = orig.publisherType();
	m_usDeivceID = orig.deviceID();
	m_Treadmill = orig.getTreadmillStatus();
	m_Mainpulator = orig.getMainpulatorStatus();
	m_MotionTracking = orig.getMotionTrackingStatus();
	m_MockupGun = orig.getMockupGunStatus();
	m_HapticSuite = orig.getHapticSuiteStatus();
	m_WearblePatch = orig.getWearblePatchStatus();
	m_MoneuverAssistant = orig.getMoneuverAssistantStatus();
	//m_nDeviceStatus = orig.DeviceStatus();

	return *this;
}

const char* extDeviceState::name() const
{
	return "DeviceState";
}

void extDeviceState::setPublisherType(unsigned short usPublisherType)
{
	m_usPublisherType = usPublisherType;
}

unsigned short extDeviceState::publisherType() const
{
	return m_usPublisherType;
}

void extDeviceState::setDeviceID(unsigned short usDeivceID)
{
	m_usDeivceID = usDeivceID;
}

unsigned short extDeviceState::deviceID() const
{
	return m_usDeivceID;
}

void extDeviceState::setTreadmill(byte status)
{
	m_Treadmill = status;
}

byte extDeviceState::getTreadmillStatus() const
{
	return m_Treadmill;
}

void extDeviceState::setMainpulator(byte status)
{
	m_Mainpulator = status;
}

byte extDeviceState::getMainpulatorStatus() const
{
	return m_Mainpulator;
}

void extDeviceState::setMotionTracking(byte status)
{
	m_MotionTracking = status;
}

byte extDeviceState::getMotionTrackingStatus() const
{
	return m_MotionTracking;
}

void extDeviceState::setMockupGun(byte status)
{
	m_MockupGun = status;
}

byte extDeviceState::getMockupGunStatus() const
{
	return m_MockupGun;
}

void extDeviceState::setHapticSuite(byte status)
{
	m_HapticSuite = status;
}

byte extDeviceState::getHapticSuiteStatus() const
{
	return m_HapticSuite;
}

void extDeviceState::setWearblePatch(byte status)
{
	m_WearblePatch = status;
}

byte extDeviceState::getWearblePatchStatus() const
{
	return m_WearblePatch;
}

void extDeviceState::setMoneuverAssistant(byte status)
{
	m_MoneuverAssistant = status;
}

byte extDeviceState::getMoneuverAssistantStatus() const
{
	return m_MoneuverAssistant;
}

void extDeviceState::setMotionDriver(byte status)
{
	m_MotionDriver = status;
}

byte extDeviceState::getMotionDriverStatus() const
{
	return m_MotionDriver;
}

void extDeviceState::setMotionGunner(byte status)
{
	m_MotionGunner = status;
}

byte extDeviceState::getMotionGunnerStatus() const
{
	return m_MotionGunner;
}

void extDeviceState::setMotionBoarding(byte status)
{
	m_MotionBoarding = status;
}

byte extDeviceState::getMotionBoardingStatus() const
{
	return m_MotionBoarding;
}

void extDeviceState::setDriverSide(byte status)
{
	m_DriverSide = status;
}

byte extDeviceState::getDriverSideStatus() const
{
	return m_DriverSide;
}

void extDeviceState::setGunnerSide(byte status)
{
	m_GunnerSide = status;
}

byte extDeviceState::getGunnerSideStatus() const
{
	return m_GunnerSide;
}

void extDeviceState::setBoardingSide(byte status)
{
	m_BoardingSide = status;
}

byte extDeviceState::getBoardingSideStatus() const
{
	return m_BoardingSide;
}

//void extDeviceState::setDeviceStatus(int nDeviceStatus)
//{
//	m_nDeviceStatus = nDeviceStatus;
//}
//
//int extDeviceState::DeviceStatus() const
//{
//	return m_nDeviceStatus;
//}

DtInteraction* extDeviceState::create()
{
	return  new extDeviceState();
}

void extDeviceState::addCallback(DtExerciseConn* conn, extDeviceStateCB cb, void* usr)
{
	conn->addInteractionCallbackByName("DeviceState", (DtReceiveInteractionCb)cb, usr);
	conn->interactionFactory()->addCreator("DeviceState", extDeviceState::create);
}

void extDeviceState::removeCallback(DtExerciseConn* conn, extDeviceStateCB cb, void* usr)
{
	conn->removeInteractionCallbackByName("DeviceState", (DtReceiveInteractionCb)cb, usr);
}

const char* extDeviceState::interactionClassToUse(DtExerciseConn* exConn) const
{
	return "DeviceState";
}

DtInteractionDecoder* extDeviceState::createDecoder(DtExerciseConn* exConn) const
{
	return new extDeviceStateDecoder(exConn, classDesc());
}

DtInteractionEncoder* extDeviceState::createEncoder(DtExerciseConn* exConn) const
{
	return new extDeviceStateEncoder(exConn, classDesc());
}

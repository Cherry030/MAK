#pragma once

#include <vl/interactionWithEncDec.h>
#include <vl/exerciseConnHLA.h>

class extDeviceState;

typedef void (*extDeviceStateCB)(extDeviceState* inter, void* usr);

class extDeviceState : public DtInteractionWithEncDec
{
public:
	extDeviceState();
	~extDeviceState();
	extDeviceState(const extDeviceState& orig);

	extDeviceState& operator=(const extDeviceState& orig);

	virtual const char* name() const;

	// parameter 별로 get/set 함수 구현
	virtual void setPublisherType(unsigned short usPublisherType);
	virtual unsigned short publisherType() const;

	virtual void setDeviceID(unsigned short usDeivceID);
	virtual unsigned short deviceID() const;

	// 1과제 상태 정보
	virtual void setTreadmill(byte status);
	virtual byte getTreadmillStatus() const;

	virtual void setMainpulator(byte status);
	virtual byte getMainpulatorStatus() const;

	virtual void setMotionTracking(byte status);
	virtual byte getMotionTrackingStatus() const;

	virtual void setMockupGun(byte status);
	virtual byte getMockupGunStatus() const;

	virtual void setHapticSuite(byte status);
	virtual byte getHapticSuiteStatus() const;

	virtual void setWearblePatch(byte status);
	virtual byte getWearblePatchStatus() const;

	virtual void setMoneuverAssistant(byte status);
	virtual byte getMoneuverAssistantStatus() const;


	// 2과제 상태 정보
	virtual void setMotionDriver(byte status);
	virtual byte getMotionDriverStatus() const;

	virtual void setMotionGunner(byte status);
	virtual byte getMotionGunnerStatus() const;

	virtual void setMotionBoarding(byte status);
	virtual byte getMotionBoardingStatus() const;

	virtual void setDriverSide(byte status);
	virtual byte getDriverSideStatus() const;

	virtual void setGunnerSide(byte status);
	virtual byte getGunnerSideStatus() const;

	virtual void setBoardingSide(byte status);
	virtual byte getBoardingSideStatus() const;

	//virtual void setDeviceStatus(int nDeviceStatus);
	//virtual int DeviceStatus() const;

public:
	static DtInteraction* create();

	static void addCallback(DtExerciseConn* conn, extDeviceStateCB cb, void* usr);
	static void removeCallback(DtExerciseConn* conn, extDeviceStateCB cb, void* usr);


protected:

	//! Virtual function override.  Returns the name of the FOM class to use to
	//! represent this interaction when sending.  It is called from within
	//! setExConn, when that function is called by DtExerciseConn::send.
	//! Implementing this function is required unless you would like to rely on
	//! the default behavior - which is to obtain the class to use from the FOM
	//! Mapper.  If you choose this option, you must configure the FomMapper so
	//! that it can correctly provide this information.
	virtual const char* interactionClassToUse(DtExerciseConn* exConn) const;

	//! Virtual function override.  Creates and returns a decoder to be used by
	//! setFromPhvps to decode a ParameterHandleValuePairSet into this
	//! interaction.  Implementing this function is required, unless you would
	//! like to rely on the default behavior - which is to obtain a new instance
	//! of the right decoder from the FOM Mapper.  If you choose this option,
	//! you must configure the FOM Mapper so that it can correctly provide this
	//! object.
	virtual DtInteractionDecoder* createDecoder(DtExerciseConn* exConn) const;


	//! Virtual function override.  Creates and returns an encoder to be used by
	//! phvps to encode the data in this interaction into a
	//! ParameterHandleValuePairSet.  Implementing this function is required,
	//! unless you would like to rely on the default behavior - which is to
	//! obtain a new instance of the right encoder from the FOM Mapper.  If you
	//! choose this option, you must configure the FOM Mapper so that it can
	//! correctly provide this object.
	virtual DtInteractionEncoder* createEncoder(DtExerciseConn* exConn) const;


protected:
	unsigned short m_usPublisherType;
	unsigned short m_usDeivceID;
	
	// 1 과제 상태정보
	byte m_Treadmill;
	byte m_Mainpulator;
	byte m_MotionTracking;
	byte m_MockupGun;
	byte m_HapticSuite;
	byte m_WearblePatch;
	byte m_MoneuverAssistant;

	// 2 과제 상태정보
	byte m_MotionDriver;
	byte m_MotionGunner;
	byte m_MotionBoarding;
	byte m_DriverSide;
	byte m_GunnerSide;
	byte m_BoardingSide;



	//int m_nDeviceStatus;
};


#pragma once

#include <vl/interactionWithEncDec.h>
#include <vl/exerciseConnHLA.h>

class extTraineeSensor;

typedef void (*extTraineeSensorCB)(extTraineeSensor* inter, void* usr);


class extTraineeSensor : public DtInteractionWithEncDec
{
public:
	extTraineeSensor();
	~extTraineeSensor();
	extTraineeSensor(const extTraineeSensor& orig);

	extTraineeSensor& operator=(const extTraineeSensor& orig);

	virtual const char* name() const;

	// parameter 별로 get/set 함수 구현
	virtual void setPublisherType(unsigned short usPublisherType);
	virtual unsigned short PublisherType() const;

	virtual void setTraineeID(std::string usTraineeID);
	virtual std::string TraineeID() const;

	virtual void setDeviceID(unsigned short nDeviceID);
	virtual unsigned short deviceID() const;

	virtual void setHeartbeat(int nHeartBeat);
	virtual int Heartbeat() const;

	virtual void setStress(int nStress);
	virtual int Stress() const;

	virtual void setConcentrativeness(int nConcentrativeness);
	virtual int Concentrativeness() const;

	virtual void setFatigue(int nFatigue);
	virtual int Fatigue() const;





public:
	static DtInteraction* create();

	static void addCallback(DtExerciseConn* conn, extTraineeSensorCB cb, void* usr);
	static void removeCallback(DtExerciseConn* conn, extTraineeSensorCB cb, void* usr);


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
	std::string m_usTraineeID;
	unsigned short m_usDeviceID;
	int m_nHeartbeat;
	int m_nStress;
	int m_nConcentrativeness;
	int m_nFatigue;
};


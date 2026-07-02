#pragma once

#include <vl/interactionWithEncDec.h>
#include <vl/exerciseConnHLA.h>

class extTrainingCtrl;

typedef void (*extTrainingCtrlCB)(extTrainingCtrl* inter, void* usr);

class extTrainingCtrl : public DtInteractionWithEncDec
{
public:
	extTrainingCtrl();
	~extTrainingCtrl();
	extTrainingCtrl(const extTrainingCtrl& orig);

	extTrainingCtrl& operator=(const extTrainingCtrl& orig);

	virtual const char* name() const;

	// parameter 별로 get/set 함수 구현
	virtual void setRealWorldTime(double dTime);
	virtual double getRealWorldTime() const;

	virtual void setSimulationTime(double dTime);
	virtual double getSimulationTime() const;

	virtual void setControlMessage(byte CtrlMsg);
	virtual byte getControlMessage() const;


public:
	static DtInteraction* create();

	static void addCallback(DtExerciseConn* conn, extTrainingCtrlCB cb, void* usr);
	static void removeCallback(DtExerciseConn* conn, extTrainingCtrlCB cb, void* usr);


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
	double m_dRealWorldTime;
	double m_dSimulationTime;
	byte m_ControlMessage;
};


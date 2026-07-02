#pragma once

#include <vl/interactionWithEncDec.h>
#include <vl/exerciseConnHLA.h>
#include "MyNetType.h"

class extLogInInfo;

typedef void (*extLogInInfoCB)(extLogInInfo* inter, void* usr);


class extLogInInfo : public DtInteractionWithEncDec
{
public:
	extLogInInfo();
	~extLogInInfo();
	extLogInInfo(const extLogInInfo& orig);

	extLogInInfo& operator=(const extLogInInfo& orig);

	virtual const char* name() const;

	// parameter ���� get/set �Լ� ����
	virtual void setPublisherType(unsigned short usPublisherType);
	virtual unsigned short publisherType() const;

	virtual void setDeviceID(unsigned short nDeviceID);
	virtual unsigned short deviceID() const;

	virtual void setServiceNumber(DtNetLogInInfo10 nServiceNumber);
	virtual DtNetLogInInfo10 serviceNumber() const;

	virtual void setLogInName(DtNetLogInInfo10 nName);
	virtual DtNetLogInInfo10 logInName() const;

	virtual void setAffiliation(DtNetLogInInfo10 nAffiliation);
	virtual DtNetLogInInfo10 affiliation() const;




public:
	static DtInteraction* create();

	static void addCallback(DtExerciseConn* conn, extLogInInfoCB cb, void* usr);
	static void removeCallback(DtExerciseConn* conn, extLogInInfoCB cb, void* usr);


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
	unsigned short m_usDeviceID;
	DtNetLogInInfo10 m_nServiceNumber;
	DtNetLogInInfo10 m_nName;
	DtNetLogInInfo10 m_nAffiliation;
};


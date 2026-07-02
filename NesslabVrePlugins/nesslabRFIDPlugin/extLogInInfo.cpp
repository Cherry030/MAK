#include "extLogInInfo.h"

#include <vl/interactionFactory.h>

// Header files for encoder and decoder.
#include "extLogInInfoEncoder.h"
#include "extLogInInfoDecoder.h"

extLogInInfo::extLogInInfo() :
	DtInteractionWithEncDec()
{
	m_usPublisherType = m_usDeviceID = 0;
	m_nServiceNumber = m_nName = m_nAffiliation = DtNetLogInInfo10();
}

extLogInInfo::~extLogInInfo()
{
}

extLogInInfo::extLogInInfo(const extLogInInfo& orig) :
	DtInteractionWithEncDec()
{
	m_usPublisherType = orig.publisherType();
	m_usDeviceID = orig.deviceID();
	m_nServiceNumber = orig.serviceNumber();
	m_nName = orig.logInName();
	m_nAffiliation = orig.affiliation();
}

extLogInInfo& extLogInInfo::operator=(const extLogInInfo& orig)
{
	if (this == &orig)
	{
		return *this;
	}

	DtInteractionWithEncDec::operator=(orig);

	m_usPublisherType = orig.publisherType();
	m_usDeviceID = orig.deviceID();
	m_nServiceNumber = orig.serviceNumber();
	m_nName = orig.logInName();
	m_nAffiliation = orig.affiliation();

	return *this;
}

const char* extLogInInfo::name() const
{
	return "LogInInfo";
}

void extLogInInfo::setPublisherType(unsigned short usPublisherType)
{
	m_usPublisherType = usPublisherType;
}

unsigned short extLogInInfo::publisherType() const
{
	return m_usPublisherType;
}

//260319 : SBE v1.4 ����
void extLogInInfo::setDeviceID(unsigned short nDeviceID)
{
	m_usDeviceID = nDeviceID;
}

unsigned short extLogInInfo::deviceID() const
{
	return m_usDeviceID;
}

void extLogInInfo::setServiceNumber(DtNetLogInInfo10 nServiceNumber)
{
	m_nServiceNumber = nServiceNumber;
}

DtNetLogInInfo10 extLogInInfo::serviceNumber() const
{
	return m_nServiceNumber;
}


void extLogInInfo::setLogInName(DtNetLogInInfo10 nName)
{
	m_nName = nName;
}

DtNetLogInInfo10 extLogInInfo::logInName() const
{
	return m_nName;
}

void extLogInInfo::setAffiliation(DtNetLogInInfo10 nAffiliation)
{
	m_nAffiliation = nAffiliation;
}

DtNetLogInInfo10 extLogInInfo::affiliation() const
{
	return m_nAffiliation;
}


DtInteraction* extLogInInfo::create()
{
	return new extLogInInfo();
}

void extLogInInfo::addCallback(DtExerciseConn* conn, extLogInInfoCB cb, void* usr)
{
	conn->addInteractionCallbackByName("LogInInfo", (DtReceiveInteractionCb)cb, usr);
	conn->interactionFactory()->addCreator("LogInInfo", extLogInInfo::create);
}

void extLogInInfo::removeCallback(DtExerciseConn* conn, extLogInInfoCB cb, void* usr)
{
	conn->removeInteractionCallbackByName("LogInInfo", (DtReceiveInteractionCb)cb, usr);
}

const char* extLogInInfo::interactionClassToUse(DtExerciseConn* exConn) const
{
	return "LogInInfo";
}

DtInteractionDecoder* extLogInInfo::createDecoder(DtExerciseConn* exConn) const
{
	return new extLogInInfoDecoder(exConn, classDesc());
}

DtInteractionEncoder* extLogInInfo::createEncoder(DtExerciseConn* exConn) const
{
	return new extLogInInfoEncoder(exConn, classDesc());
}

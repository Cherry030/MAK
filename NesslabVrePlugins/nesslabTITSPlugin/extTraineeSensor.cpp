#include "extTraineeSensor.h"

#include <vl/interactionFactory.h>

// Header files for encoder and decoder.
#include "extTraineeSensorEncoder.h"
#include "extTraineeSensorDecoder.h"

extTraineeSensor::extTraineeSensor() :
	DtInteractionWithEncDec()
{
	m_usPublisherType = m_usDeviceID = m_nHeartbeat = m_nStress = m_nConcentrativeness = m_nFatigue = 0;
	m_usTraineeID = "";
}

extTraineeSensor::~extTraineeSensor()
{
}

extTraineeSensor::extTraineeSensor(const extTraineeSensor& orig) :
	DtInteractionWithEncDec()
{
	m_usPublisherType = orig.PublisherType();
	m_usTraineeID = orig.TraineeID();
	m_usDeviceID = orig.deviceID();
	m_nHeartbeat = orig.Heartbeat();
	m_nStress = orig.Stress();
	m_nConcentrativeness = orig.Concentrativeness();
	m_nFatigue = orig.Fatigue();
}

extTraineeSensor& extTraineeSensor::operator=(const extTraineeSensor& orig)
{
	if (this == &orig)
	{
		return *this;
	}

	DtInteractionWithEncDec::operator=(orig);

	m_usPublisherType = orig.PublisherType();
	m_usTraineeID = orig.TraineeID();
	m_usDeviceID = orig.deviceID();
	m_nHeartbeat = orig.Heartbeat();
	m_nStress = orig.Stress();
	m_nConcentrativeness = orig.Concentrativeness();
	m_nFatigue = orig.Fatigue();

	return *this;
}

const char* extTraineeSensor::name() const
{
	return "TraineeSensor";
}

void extTraineeSensor::setPublisherType(unsigned short usPublisherType)
{
	m_usPublisherType = usPublisherType;
}

unsigned short extTraineeSensor::PublisherType() const
{
	return m_usPublisherType;
}

void extTraineeSensor::setTraineeID(std::string usTraineeID)
{
	m_usTraineeID = usTraineeID;
}

std::string extTraineeSensor::TraineeID() const
{
	return m_usTraineeID;
}

//260319 : SBE v1.4 Àû¿ë
void extTraineeSensor::setDeviceID(unsigned short nDeviceID)
{
	m_usDeviceID = nDeviceID;
}

unsigned short extTraineeSensor::deviceID() const
{
	return m_usDeviceID;
}


void extTraineeSensor::setHeartbeat(int nHeartBeat)
{
	m_nHeartbeat = nHeartBeat;
}

int extTraineeSensor::Heartbeat() const
{
	return m_nHeartbeat;
}

void extTraineeSensor::setStress(int nStress)
{
	m_nStress = nStress;
}

int extTraineeSensor::Stress() const
{
	return m_nStress;
}

void extTraineeSensor::setConcentrativeness(int nConcentrativeness)
{
	m_nConcentrativeness = nConcentrativeness;
}

int extTraineeSensor::Concentrativeness() const
{
	return m_nConcentrativeness;
}

void extTraineeSensor::setFatigue(int nFatigue)
{
	m_nFatigue = nFatigue;
}

int extTraineeSensor::Fatigue() const
{
	return m_nFatigue;
}

DtInteraction* extTraineeSensor::create()
{
	return new extTraineeSensor();
}

void extTraineeSensor::addCallback(DtExerciseConn* conn, extTraineeSensorCB cb, void* usr)
{
	conn->addInteractionCallbackByName("TraineeSensor", (DtReceiveInteractionCb)cb, usr);
	conn->interactionFactory()->addCreator("TraineeSensor", extTraineeSensor::create);
}

void extTraineeSensor::removeCallback(DtExerciseConn* conn, extTraineeSensorCB cb, void* usr)
{
	conn->removeInteractionCallbackByName("TraineeSensor", (DtReceiveInteractionCb)cb, usr);
}

const char* extTraineeSensor::interactionClassToUse(DtExerciseConn* exConn) const
{
	return "TraineeSensor";
}

DtInteractionDecoder* extTraineeSensor::createDecoder(DtExerciseConn* exConn) const
{
	return new extTraineeSensorDecoder(exConn, classDesc());
}

DtInteractionEncoder* extTraineeSensor::createEncoder(DtExerciseConn* exConn) const
{
	return new extTraineeSensorEncoder(exConn, classDesc());
}

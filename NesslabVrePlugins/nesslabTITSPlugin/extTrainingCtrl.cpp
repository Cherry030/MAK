#include "extTrainingCtrl.h"

#include <vl/interactionFactory.h>

// Header files for encoder and decoder.
#include "extTrainingCtrlDecoder.h"
#include "extTrainingCtrlEncoder.h"

extTrainingCtrl::extTrainingCtrl() :
	DtInteractionWithEncDec()
{
}

extTrainingCtrl::~extTrainingCtrl()
{
}

extTrainingCtrl::extTrainingCtrl(const extTrainingCtrl& orig) :
	DtInteractionWithEncDec(orig)
{
	m_dRealWorldTime = orig.getRealWorldTime();
	m_dSimulationTime = orig.getSimulationTime();
	m_ControlMessage = orig.getControlMessage();
}

extTrainingCtrl& extTrainingCtrl::operator=(const extTrainingCtrl& orig)
{
	if (this == &orig)
	{
		return *this;
	}

	DtInteractionWithEncDec::operator=(orig);

	m_dRealWorldTime = orig.getRealWorldTime();
	m_dSimulationTime = orig.getSimulationTime();
	m_ControlMessage = orig.getControlMessage();

	return *this;
}

const char* extTrainingCtrl::name() const
{
	return "TrainingCtrl";
}

void extTrainingCtrl::setRealWorldTime(double dTime)
{
	m_dRealWorldTime = dTime;
}

double extTrainingCtrl::getRealWorldTime() const
{
	return m_dRealWorldTime;
}

void extTrainingCtrl::setSimulationTime(double dTime)
{
	m_dSimulationTime = dTime;
}

double extTrainingCtrl::getSimulationTime() const
{
	return m_dSimulationTime;
}

void extTrainingCtrl::setControlMessage(byte CtrlMsg)
{
	m_ControlMessage = CtrlMsg;
}

byte extTrainingCtrl::getControlMessage() const
{
	return m_ControlMessage;
}

DtInteraction* extTrainingCtrl::create()
{
	return new extTrainingCtrl();
}

void extTrainingCtrl::addCallback(DtExerciseConn* conn, extTrainingCtrlCB cb, void* usr)
{
	conn->addInteractionCallbackByName("TrainingCtrl", (DtReceiveInteractionCb)cb, usr);

	conn->interactionFactory()->addCreator("TrainingCtrl", extTrainingCtrl::create);

}

void extTrainingCtrl::removeCallback(DtExerciseConn* conn, extTrainingCtrlCB cb, void* usr)
{
	conn->removeInteractionCallbackByName("TrainingCtrl", (DtReceiveInteractionCb)cb, usr);
}

const char* extTrainingCtrl::interactionClassToUse(DtExerciseConn* exConn) const
{
	return "TrainingCtrl";
}

DtInteractionDecoder* extTrainingCtrl::createDecoder(DtExerciseConn* exConn) const
{
	return new extTrainingCtrlDecoder(exConn, classDesc());;
}

DtInteractionEncoder* extTrainingCtrl::createEncoder(DtExerciseConn* exConn) const
{
	return new extTrainingCtrlEncoder(exConn, classDesc());;
}

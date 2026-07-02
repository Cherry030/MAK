#include "extLogInInfoEncoder.h"
#include "extLogInInfo.h"

extLogInInfoEncoder::extLogInInfoEncoder(
	DtExerciseConn* exConn, DtInterClassDesc* classDesc) :
	DtInteractionEncoder(exConn, classDesc)
{
	DtADD_PARAM_ENCODER(publisherType);
	DtADD_PARAM_ENCODER(deviceID);
	DtADD_PARAM_ENCODER(serviceNumber);
	DtADD_PARAM_ENCODER(logInName);
	DtADD_PARAM_ENCODER(affiliation);
}

extLogInInfoEncoder::~extLogInInfoEncoder()
{
}

DtDEFINE_SIMPLE_extLogInInfo_PARAM_ENCODER(publisherType, DtNetU16, publisherType);
DtDEFINE_SIMPLE_extLogInInfo_PARAM_ENCODER(deviceID, DtNetU16, deviceID);
DtDEFINE_SIMPLE_extLogInInfo_PARAM_ENCODER(serviceNumber, DtNetLogInInfo10, serviceNumber);
DtDEFINE_SIMPLE_extLogInInfo_PARAM_ENCODER(logInName, DtNetLogInInfo10, logInName);
DtDEFINE_SIMPLE_extLogInInfo_PARAM_ENCODER(affiliation, DtNetLogInInfo10, affiliation);

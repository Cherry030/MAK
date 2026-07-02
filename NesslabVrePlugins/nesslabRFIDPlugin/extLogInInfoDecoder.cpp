#include "extLogInInfoDecoder.h"
#include "extLogInInfo.h"

extLogInInfoDecoder::extLogInInfoDecoder
(DtExerciseConn* exConn, DtInterClassDesc* classDesc) :
	DtInteractionDecoder(exConn, classDesc)
{
	DtADD_PARAM_DECODER(publisherType);
	DtADD_PARAM_DECODER(deviceID);
	DtADD_PARAM_DECODER(serviceNumber);
	DtADD_PARAM_DECODER(logInName);
	DtADD_PARAM_DECODER(affiliation);
}

extLogInInfoDecoder::~extLogInInfoDecoder()
{
}
DtDEFINE_SIMPLE_extLogInInfo_PARAM_DECODER(publisherType, DtNetU16, setPublisherType);
DtDEFINE_SIMPLE_extLogInInfo_PARAM_DECODER(deviceID, DtNetU16, setDeviceID);
DtDEFINE_SIMPLE_extLogInInfo_PARAM_DECODER(serviceNumber, DtNetLogInInfo10, setServiceNumber);
DtDEFINE_SIMPLE_extLogInInfo_PARAM_DECODER(logInName, DtNetLogInInfo10, setLogInName);
DtDEFINE_SIMPLE_extLogInInfo_PARAM_DECODER(affiliation, DtNetLogInInfo10, setAffiliation);
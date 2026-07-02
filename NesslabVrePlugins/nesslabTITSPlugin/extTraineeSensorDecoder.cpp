#include "extTraineeSensorDecoder.h"
#include "extTraineeSensor.h"

extTraineeSensorDecoder::extTraineeSensorDecoder
(DtExerciseConn* exConn, DtInterClassDesc* classDesc) :
	DtInteractionDecoder(exConn, classDesc)
{
	DtADD_PARAM_DECODER(PublisherType);
	//DtADD_PARAM_DECODER(TraineeID);
	DtADD_PARAM_DECODER(deviceID);
	DtADD_PARAM_DECODER(Heartbeat);
	DtADD_PARAM_DECODER(Stress);
	DtADD_PARAM_DECODER(Concentrativeness);
	DtADD_PARAM_DECODER(Fatigue);
}

extTraineeSensorDecoder::~extTraineeSensorDecoder()
{
}
DtDEFINE_SIMPLE_extTraineeSensor_PARAM_DECODER(PublisherType, DtNetU16, setPublisherType);
//DtDEFINE_SIMPLE_extTraineeSensor_PARAM_DECODER(TraineeID, DtNetU16, setTraineeID);
DtDEFINE_SIMPLE_extTraineeSensor_PARAM_DECODER(deviceID, DtNetU16, setDeviceID);
DtDEFINE_SIMPLE_extTraineeSensor_PARAM_DECODER(Heartbeat, DtNetInt32, setHeartbeat);
DtDEFINE_SIMPLE_extTraineeSensor_PARAM_DECODER(Stress, DtNetInt32, setStress);
DtDEFINE_SIMPLE_extTraineeSensor_PARAM_DECODER(Concentrativeness, DtNetInt32, setConcentrativeness);
DtDEFINE_SIMPLE_extTraineeSensor_PARAM_DECODER(Fatigue, DtNetInt32, setFatigue);

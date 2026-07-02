#include "extTraineeSensorEncoder.h"
#include "extTraineeSensor.h"

extTraineeSensorEncoder::extTraineeSensorEncoder(
	DtExerciseConn* exConn, DtInterClassDesc* classDesc) :
	DtInteractionEncoder(exConn, classDesc)
{
	DtADD_PARAM_ENCODER(PublisherType);
	//DtADD_PARAM_ENCODER(TraineeID);
	DtADD_PARAM_ENCODER(deviceID);
	DtADD_PARAM_ENCODER(Heartbeat);
	DtADD_PARAM_ENCODER(Stress);
	DtADD_PARAM_ENCODER(Concentrativeness);
	DtADD_PARAM_ENCODER(Fatigue);
}

extTraineeSensorEncoder::~extTraineeSensorEncoder()
{
}

DtDEFINE_SIMPLE_extTraineeSensor_PARAM_ENCODER(PublisherType, DtNetU16, PublisherType);
//DtDEFINE_SIMPLE_extTraineeSensor_PARAM_ENCODER(TraineeID, DtNetU16, TraineeID);
DtDEFINE_SIMPLE_extTraineeSensor_PARAM_ENCODER(deviceID, DtNetU16, deviceID);
DtDEFINE_SIMPLE_extTraineeSensor_PARAM_ENCODER(Heartbeat, DtNetInt32, Heartbeat);
DtDEFINE_SIMPLE_extTraineeSensor_PARAM_ENCODER(Stress, DtNetInt32, Stress);
DtDEFINE_SIMPLE_extTraineeSensor_PARAM_ENCODER(Concentrativeness, DtNetInt32, Concentrativeness);
DtDEFINE_SIMPLE_extTraineeSensor_PARAM_ENCODER(Fatigue, DtNetInt32, Fatigue);

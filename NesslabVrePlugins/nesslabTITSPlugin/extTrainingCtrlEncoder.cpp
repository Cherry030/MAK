#include "extTrainingCtrlEncoder.h"
#include "extTrainingCtrl.h"

extTrainingCtrlEncoder::extTrainingCtrlEncoder(
	DtExerciseConn* exConn, DtInterClassDesc* classDesc) :
	DtInteractionEncoder(exConn, classDesc)
{
	DtADD_PARAM_ENCODER(RealWorldTime);
	DtADD_PARAM_ENCODER(SimulationTime);
	DtADD_PARAM_ENCODER(ControlMessage);
}

extTrainingCtrlEncoder::~extTrainingCtrlEncoder()
{
}

DtDEFINE_SIMPLE_extTrainingCtrl_PARAM_ENCODER(RealWorldTime, double, getRealWorldTime);
DtDEFINE_SIMPLE_extTrainingCtrl_PARAM_ENCODER(SimulationTime, double, getSimulationTime);
DtDEFINE_SIMPLE_extTrainingCtrl_PARAM_ENCODER(ControlMessage, DtNet8Bits, getControlMessage);

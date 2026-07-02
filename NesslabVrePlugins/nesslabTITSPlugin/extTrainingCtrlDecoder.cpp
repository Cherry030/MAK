#include "extTrainingCtrlDecoder.h"
#include "extTrainingCtrl.h"

extTrainingCtrlDecoder::extTrainingCtrlDecoder(
	DtExerciseConn* exConn, DtInterClassDesc* classDesc) :
	DtInteractionDecoder(exConn, classDesc)
{
	DtADD_PARAM_DECODER(RealWorldTime);
	DtADD_PARAM_DECODER(SimulationTime);
	DtADD_PARAM_DECODER(ControlMessage);
}

extTrainingCtrlDecoder::~extTrainingCtrlDecoder()
{
}

DtDEFINE_SIMPLE_extTrainingCtrl_PARAM_DECODER(RealWorldTime, double, setRealWorldTime);
DtDEFINE_SIMPLE_extTrainingCtrl_PARAM_DECODER(SimulationTime, double, setSimulationTime);
DtDEFINE_SIMPLE_extTrainingCtrl_PARAM_DECODER(ControlMessage, DtNet8Bits, setControlMessage);

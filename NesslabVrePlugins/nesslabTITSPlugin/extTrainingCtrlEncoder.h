#pragma once

#include <vl/interactionEncoder.h>

class extTrainingCtrl;

#define DtDECLARE_extTrainingCtrl_PARAM_ENCODER(paramName) \
   DtDECLARE_PARAM_ENCODER(extTrainingCtrl, paramName)

#define DtDEFINE_SIMPLE_extTrainingCtrl_PARAM_ENCODER(paramName, netType, inspector) \
   DtDEFINE_SIMPLE_PARAM_ENCODER(extTrainingCtrlEncoder, extTrainingCtrl, paramName, netType, inspector)

class extTrainingCtrlEncoder : public DtInteractionEncoder
{
public:

    //! Constructor
    extTrainingCtrlEncoder(DtExerciseConn* exConn, DtInterClassDesc* classDesc);

    //! Destructor
    virtual ~extTrainingCtrlEncoder();

protected:
    DtDECLARE_extTrainingCtrl_PARAM_ENCODER(RealWorldTime);
    DtDECLARE_extTrainingCtrl_PARAM_ENCODER(SimulationTime);
    DtDECLARE_extTrainingCtrl_PARAM_ENCODER(ControlMessage);
};


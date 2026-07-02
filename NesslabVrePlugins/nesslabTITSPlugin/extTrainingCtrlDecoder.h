#pragma once

#include <vl/interactionDecoder.h>

class extTrainingCtrl;

#define DtDECLARE_extTrainingCtrl_PARAM_DECODER(paramName) \
   DtDECLARE_PARAM_DECODER(extTrainingCtrl, paramName)

#define DtDEFINE_SIMPLE_extTrainingCtrl_PARAM_DECODER(paramName, netType, mutator) \
   DtDEFINE_SIMPLE_PARAM_DECODER(extTrainingCtrlDecoder, extTrainingCtrl, paramName, netType, mutator)

#define DtDEFINE_SIMPLE_extTrainingCtrl_PARAM_DECODER_WITH_CAST( paramName, netType, mutator, castExpr) \
   DtDEFINE_SIMPLE_PARAM_DECODER_WITH_CAST( extTrainingCtrlDecoder, extTrainingCtrl, paramName, netType, mutator, castExpr)

class extTrainingCtrlDecoder : public DtInteractionDecoder
{
public:

    //! Constructor
    extTrainingCtrlDecoder(DtExerciseConn* exConn, DtInterClassDesc* classDesc);

    //! Destructor
    virtual ~extTrainingCtrlDecoder();

protected:
    DtDECLARE_extTrainingCtrl_PARAM_DECODER(RealWorldTime);
    DtDECLARE_extTrainingCtrl_PARAM_DECODER(SimulationTime);
    DtDECLARE_extTrainingCtrl_PARAM_DECODER(ControlMessage);
};


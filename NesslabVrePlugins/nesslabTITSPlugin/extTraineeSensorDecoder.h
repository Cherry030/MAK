#pragma once

#include <vl/interactionDecoder.h>

class extTraineeSensor;

#define DtDECLARE_extTraineeSensor_PARAM_DECODER(paramName) \
   DtDECLARE_PARAM_DECODER(extTraineeSensor, paramName)

#define DtDEFINE_SIMPLE_extTraineeSensor_PARAM_DECODER(paramName, netType, mutator) \
   DtDEFINE_SIMPLE_PARAM_DECODER(extTraineeSensorDecoder, extTraineeSensor, paramName, netType, mutator)

#define DtDEFINE_SIMPLE_extTraineeSensor_PARAM_DECODER_WITH_CAST( paramName, netType, mutator, castExpr) \
   DtDEFINE_SIMPLE_PARAM_DECODER_WITH_CAST( extTraineeSensorDecoder, extTraineeSensor, paramName, netType, mutator, castExpr)


class extTraineeSensorDecoder : public DtInteractionDecoder
{
public:

    //! Constructor
    extTraineeSensorDecoder(DtExerciseConn* exConn, DtInterClassDesc* classDesc);

    //! Destructor
    virtual ~extTraineeSensorDecoder();

protected:
    DtDECLARE_extTraineeSensor_PARAM_DECODER(PublisherType);
    //DtDECLARE_extTraineeSensor_PARAM_DECODER(TraineeID);
    DtDECLARE_extTraineeSensor_PARAM_DECODER(deviceID);
    DtDECLARE_extTraineeSensor_PARAM_DECODER(Heartbeat);
    DtDECLARE_extTraineeSensor_PARAM_DECODER(Stress);
    DtDECLARE_extTraineeSensor_PARAM_DECODER(Concentrativeness);
    DtDECLARE_extTraineeSensor_PARAM_DECODER(Fatigue);

};

#pragma once

#include <vl/interactionEncoder.h>

class extTraineeSensor;

#define DtDECLARE_extTraineeSensor_PARAM_ENCODER(paramName) \
   DtDECLARE_PARAM_ENCODER(extTraineeSensor, paramName)

#define DtDEFINE_SIMPLE_extTraineeSensor_PARAM_ENCODER(paramName, netType, inspector) \
   DtDEFINE_SIMPLE_PARAM_ENCODER(extTraineeSensorEncoder, extTraineeSensor, paramName, netType, inspector)

class extTraineeSensorEncoder : public DtInteractionEncoder
{
public:

    //! Constructor
    extTraineeSensorEncoder(DtExerciseConn* exConn, DtInterClassDesc* classDesc);

    //! Destructor
    virtual ~extTraineeSensorEncoder();

protected:
    DtDECLARE_extTraineeSensor_PARAM_ENCODER(PublisherType);
    //DtDECLARE_extTraineeSensor_PARAM_ENCODER(TraineeID);
    DtDECLARE_extTraineeSensor_PARAM_ENCODER(deviceID);
    DtDECLARE_extTraineeSensor_PARAM_ENCODER(Heartbeat);
    DtDECLARE_extTraineeSensor_PARAM_ENCODER(Stress);
    DtDECLARE_extTraineeSensor_PARAM_ENCODER(Concentrativeness);
    DtDECLARE_extTraineeSensor_PARAM_ENCODER(Fatigue);
};


#pragma once

#include <vl/interactionEncoder.h>
#include "MyNetType.h"

class extLogInInfo;

#define DtDECLARE_extLogInInfo_PARAM_ENCODER(paramName) \
   DtDECLARE_PARAM_ENCODER(extLogInInfo, paramName)

#define DtDEFINE_SIMPLE_extLogInInfo_PARAM_ENCODER(paramName, netType, inspector) \
   DtDEFINE_SIMPLE_PARAM_ENCODER(extLogInInfoEncoder, extLogInInfo, paramName, netType, inspector)

class extLogInInfoEncoder : public DtInteractionEncoder
{
public:

    //! Constructor
    extLogInInfoEncoder(DtExerciseConn* exConn, DtInterClassDesc* classDesc);

    //! Destructor
    virtual ~extLogInInfoEncoder();

protected:
    DtDECLARE_extLogInInfo_PARAM_ENCODER(publisherType);
    DtDECLARE_extLogInInfo_PARAM_ENCODER(deviceID);
    DtDECLARE_extLogInInfo_PARAM_ENCODER(serviceNumber);
    DtDECLARE_extLogInInfo_PARAM_ENCODER(logInName);
    DtDECLARE_extLogInInfo_PARAM_ENCODER(affiliation);
};


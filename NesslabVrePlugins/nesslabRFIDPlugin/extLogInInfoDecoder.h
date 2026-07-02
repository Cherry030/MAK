#pragma once

#include <vl/interactionDecoder.h>

#include "MyNetType.h"

class extLogInInfo;

#define DtDECLARE_extLogInInfo_PARAM_DECODER(paramName) \
   DtDECLARE_PARAM_DECODER(extLogInInfo, paramName)

#define DtDEFINE_SIMPLE_extLogInInfo_PARAM_DECODER(paramName, netType, mutator) \
   DtDEFINE_SIMPLE_PARAM_DECODER(extLogInInfoDecoder, extLogInInfo, paramName, netType, mutator)

#define DtDEFINE_SIMPLE_extLogInInfo_PARAM_DECODER_WITH_CAST( paramName, netType, mutator, castExpr) \
   DtDEFINE_SIMPLE_PARAM_DECODER_WITH_CAST( extLogInInfoDecoder, extLogInInfo, paramName, netType, mutator, castExpr)

typedef DtNetU8 DtNetByteArray10[10];


class extLogInInfoDecoder : public DtInteractionDecoder
{
public:

    //! Constructor
    extLogInInfoDecoder(DtExerciseConn* exConn, DtInterClassDesc* classDesc);

    //! Destructor
    virtual ~extLogInInfoDecoder();

protected:
    DtDECLARE_extLogInInfo_PARAM_DECODER(publisherType);
    DtDECLARE_extLogInInfo_PARAM_DECODER(deviceID);
    DtDECLARE_extLogInInfo_PARAM_DECODER(serviceNumber);
    DtDECLARE_extLogInInfo_PARAM_DECODER(logInName);
    DtDECLARE_extLogInInfo_PARAM_DECODER(affiliation);

};

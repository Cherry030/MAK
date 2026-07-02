/*******************************************************************************
** Copyright (c) 2020 MAK Technologies
** All rights reserved.
*******************************************************************************/

// exampleHumanArtPartActuator.h

// DtExampleHumanArtPartActuator; acutator that demonstrates publishing
// human character articulated parts representing joints on a DI-Guy
// character.  It rotates the left shoulder of a given character. 
#pragma once
#pragma warning(disable: 4251)
#pragma warning(disable: 4786)
#pragma warning(disable: 4290)

#include <vrfobjcore/actuatorComponent.h>
#include <vrfobjcore/platformLocalObjectFacade.h>
#include <vrvCore/DtDe.h>
#include "vrfutil/rwDIGuyCharacterInfo.h"

//260119
#include "Nesslab/nesslabCommon.h"

// 260504 RTI
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <wchar.h>
#include <RTI/RTI1516.h>
#include "rtiFedAmb1516e.h"

class DtVrfMovingObjectStateRepository;
using namespace makVrfEvents;
namespace ysu_to_vris_packets
{
    struct HandposeLabelPacket;
}

namespace nesslab_backend_plugins
{
    constexpr char RTI_NETWORK_PLUGIN_TYPE[] = "vre-rti-network-actuator";
    class RTINetworkPlugin : public DtActuatorComponent
    {

    public:

        RTINetworkPlugin(const DtString& name,
            DtLocalObject* owner,
            DtSimulationServices* simManager,
            DtComponentDescriptor* desc = 0,
            DtReaderWriterRegistry* parentRegistry = 0);

        RTINetworkPlugin() = delete;
        RTINetworkPlugin(const RTINetworkPlugin& orig) = delete;
        const RTINetworkPlugin& operator=(const RTINetworkPlugin& orig) = delete;
        virtual ~RTINetworkPlugin();
        virtual bool init() override;
        virtual const char* type() const override;
        virtual void tick() override;

        static DtSimComponent* creator(const DtString& name,
            DtLocalObject* owner,
            DtSimulationServices* simManager,
            DtComponentDescriptor* desc = 0,
            DtReaderWriterRegistry* parentRegistry = 0);


    protected:
        DtPlatformLocalObjectFacade myPlatformLocalObjectFacade;

        makVrf::DtArticulatedPartStateRepository* myLeftShoulder;
        makVrf::DtArticulatedPartStateRepository* myRightShoulder;
        makVrf::DtArticulatedPartStateRepository* myLeftWrist;


        double myAnimationInterval;
        double myTimeInInterval;

        DtVrfMovingObjectStateRepository* myLocalState;


    public:
        void SetFOMData();

        void SetLogInInfoData(std::vector<unsigned char> data);
    };

}
void InitInterClass();
typedef std::map<std::wstring, rti1516e::ParameterHandle> DtParamNameHandleMap;
extern DtSimpleData theAmbData;
extern rti1516e::FederateHandle theFederateHandle;
extern rti1516e::RTIambassador* rtiAmb;

extern std::vector<std::wstring> vecInterClassName;

// Receive Data : Target Info
extern std::wstring InterClass_TargetInfoReply;
extern rti1516e::InteractionClassHandle InterClassHandle_TargetInfoReply;
extern DtParamNameHandleMap ParamNameHandleMap_TargetInfoReply;
extern rti1516e::ParameterHandleValueMap paramValues_TargetInfoReply;

// Send Data    : Login Info
extern std::wstring InterClass_LogInInfo;
extern rti1516e::InteractionClassHandle InterClassHandle_LogInInfo;
extern DtParamNameHandleMap ParamNameHandleMap_LogInInfo;
extern rti1516e::ParameterHandleValueMap paramValues_LogInInfo;



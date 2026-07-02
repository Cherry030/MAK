//#define IS_NESSLAB
#include "rtiNetworkPlugin.h"


#include <vrvCore/DtDe.h>
#include <vrvCore/DtAccessoryManager.h>

#include <vrvUtil/signalslib.h>

#include <boost/bind/bind.hpp>

// Define export macro for vrvVrl includes
#define DL_DLL_IGCONVRLINK DT_DLL_EXAMPLEACCESSORYTEMPLATE

// Define the protocol specific macros to compile for HLA13
#define DT_PROTOCOL_NAMESPACE vrvHla13
// #define DtHLA 1
// #define RTI_USES_STD_FSTREAM 1


// Nesslab
#include "Nesslab/nesslabTcpServer.h"
#include "Nesslab//nesslabTcpClient.h"


using namespace std;
using namespace makVrf;
using namespace makVrfEvents;
using namespace nesslab_common;

using namespace rti1516e;

namespace ysu_to_vris_packets
{
#pragma pack(push, 1)
    struct HandposeLabelPacket
    {
        PacketHeader header;

        BYTE handIndex = 0;              // 왼손 → 0 오른손 → 1
        BYTE handLabel = 0;              // 수신호 라벨

        PacketTrailer trailer;

        HandposeLabelPacket()
        {
            header.deviceID = 0;        //IHRS( 0x56 )  SHRS( 0x66 )
            header.msgID = 0;           //IHRS( 0x59 )  SHRS( 0x6c )
            header.length = sizeof(HandposeLabelPacket); //0x09
        }
    };
#pragma pack(pop)
}

namespace {

    bool mapInteractionParameters(
        RTIambassador& rtiAmb,
        const wstring& className,
        InteractionClassHandle& classHandle,
        DtParamNameHandleMap& paramMap,
        const vector<wstring>& paramNames)
    {
        wstring lastParam;
        try
        {
            classHandle = rtiAmb.getInteractionClassHandle(className);
            for (const wstring& p : paramNames)
            {
                lastParam = p;
                paramMap[p] = rtiAmb.getParameterHandle(classHandle, p);
            }
        }
        catch (Exception& ex)
        {
            wcout << L"RTI Exception: " << ex.what() << endl
                << L"\tclass=" << className << L" param=" << lastParam << endl;
            return false;
        }
        return true;
    }

    bool subscribeInteraction(RTIambassador& rtiAmb, const wstring& className, InteractionClassHandle h)
    {
        try
        {
            rtiAmb.subscribeInteractionClass(h);
            rtiAmb.evokeMultipleCallbacks(0.1, 0.2);
        }
        catch (Exception& ex)
        {
            wcout << L"RTI Exception: " << ex.what() << endl
                << L"\tCould not subscribe to " << className << endl;
            return false;
        }
        wcout << L"Subscribed to interaction: " << className << L" handle " << h.toString() << endl;
        return true;
    }

    bool publishInteraction(RTIambassador& rtiAmb, const wstring& className, InteractionClassHandle h)
    {
        try
        {
            rtiAmb.publishInteractionClass(h);
            rtiAmb.evokeMultipleCallbacks(0.1, 0.2);
        }
        catch (Exception& ex)
        {
            wcout << L"RTI Exception: " << ex.what() << endl
                << L"\tCould not publish " << className << endl;
            return false;
        }
        wcout << L"Published interaction: " << className << L" handle " << h.toString() << endl;
        return true;
    }

} // namespace
DtSimpleData theAmbData;
FederateHandle theFederateHandle;

RTIambassador* rtiAmb = 0;

std::vector<std::wstring> vecInterClassName;

std::wstring InterClass_TargetInfoReply = L"TargetInfoReply";
InteractionClassHandle InterClassHandle_TargetInfoReply;
DtParamNameHandleMap ParamNameHandleMap_TargetInfoReply;
ParameterHandleValueMap paramValues_TargetInfoReply;

std::wstring InterClass_LogInInfo = L"LogInInfo";
InteractionClassHandle InterClassHandle_LogInInfo;
DtParamNameHandleMap ParamNameHandleMap_LogInInfo;
ParameterHandleValueMap paramValues_LogInInfo;

bool connect(RTIambassador& rtiAmb, MyFederateAmbassador& fedAmb, const std::wstring& localDesignator)
{
    wcout << L"Connect to RTI" << endl;
    try
    {
        rtiAmb.connect(fedAmb, HLA_EVOKED, localDesignator);
    }
    catch (Exception& ex)
    {
        wcout << L"RTI Exception: " << ex.what() << endl;
        return false;
    }
    rtiAmb.evokeMultipleCallbacks(0.1, 0.2);

    wcout << L"Connected." << endl;
    return true;
}

void listFederations(RTIambassador& rtiAmb)
{
    try
    {
        int count = 0;
        theAmbData.listFederationsReturned = false;
        rtiAmb.listFederationExecutions();
        wcout << L"Waiting for reportFederationExecutions\n";
        while (count < 100 && !theAmbData.listFederationsReturned)
        {
            rtiAmb.evokeMultipleCallbacks(0.1, 0.5);
            count++;
        }

        if (!theAmbData.listFederationsReturned)
        {
            wcout << L"\nFailed to get reportFederationExecutions callback\n";
        }
    }
    catch (RTIinternalError& ex)
    {
        (void)ex;
    }
    catch (Exception& ex)
    {
        wcout << L"RTI Exception: " << ex.what() << endl
            << L"\tCould not list federation executions!" << endl;
    }
}

std::vector<std::wstring> fomModules;
void createFedEx(RTIambassador& rtiAmb, std::wstring const& fedName, std::wstring const& fedFile)
{
    (void)fedFile;
    wcout << L"createFederationExecution " << fedName << endl;
    try
    {
        fomModules.clear();
        fomModules.push_back(L"RPR_FOM_v2.0_1516-2010.xml");
        fomModules.push_back(L"NETN-BASE.xml");
        fomModules.push_back(L"NETN-Physical.xml");
        fomModules.push_back(L"NETN-METOC.xml");
        fomModules.push_back(L"NETN-MRM.xml");
        fomModules.push_back(L"NETN-ETR.xml");
        fomModules.push_back(L"MAK-Physical-2_evolved.xml");
        fomModules.push_back(L"MAK-Aerodrome-1_evolved.xml");
        fomModules.push_back(L"MAK-METOC-3_evolved.xml");
        fomModules.push_back(L"MAK-VRFExt-12_evolved.xml");
        fomModules.push_back(L"MAK-DIGuy-7_evolved.xml");
        fomModules.push_back(L"MAK-LgrControl-2_evolved.xml");
        fomModules.push_back(L"MAK-VRFAggregate-7_evolved.xml");
        fomModules.push_back(L"MAK-DynamicTerrain-2_evolved.xml");
        fomModules.push_back(L"MAK-VRLExt-3_evolved.xml");
        fomModules.push_back(L"MAK-DER-1_evolved.xml");
        fomModules.push_back(L"RPR-Enumerations_Experimental_IFF.xml");
        fomModules.push_back(L"RPR-MAK_Experimental_IFF-4.xml");
        fomModules.push_back(L"SBE-Ext-FOM_v1.4.xml");

        rtiAmb.createFederationExecution(fedName, fomModules);
    }
    catch (FederationExecutionAlreadyExists& ex)
    {
        wcout << L"FederationExecutionAlreadyExists: " << ex.what() << endl
            << L"\tCould not create federation execution!" << endl;
    }
    catch (Exception& ex)
    {
        wcout << L"RTI Exception: " << ex.what() << endl
            << L"\tCould not create federation execution!" << endl;
        exit(0);
    }

    rtiAmb.evokeMultipleCallbacks(0.1, 0.2);

    wcout << L"Federation Created." << endl;
}

void joinFedEx(RTIambassador& rtiAmb, std::wstring const& federateType, std::wstring const& federationName)
{
    bool joined = false;
    const int maxTry = 10;
    int numTries = 0;
    wcout << L"joinFederationExecution " << federateType << L" " << federationName << endl;

    while (!joined && numTries++ < maxTry)
    {
        try
        {
            theFederateHandle = rtiAmb.joinFederationExecution(federateType, federationName);
            joined = true;
        }
        catch (FederationExecutionDoesNotExist)
        {
            wcout << L"FederationExecutionDoesNotExist, try " << numTries << L" out of " << maxTry << endl;
            continue;
        }
        catch (Exception& ex)
        {
            wcout << L"RTI Exception: " << ex.what() << endl;
            return;
        }
        rtiAmb.evokeMultipleCallbacks(0.1, 0.2);
    }

    if (joined)
    {
        wcout << L"Joined Federation." << endl;
    }
    else
    {
        wcout << L"Giving up." << endl;
        rtiAmb.destroyFederationExecution(federationName);
        exit(0);
    }
}

void resignAndDestroy(RTIambassador& rtiAmb, std::wstring const& federationName)
{
    wcout << L"Resign and Destroy Federation" << endl;
    rtiAmb.resignFederationExecution(DELETE_OBJECTS);
    rtiAmb.destroyFederationExecution(federationName);
    rtiAmb.disconnect();
}

bool publishAndSubscribeInteraction_DSTRecv(RTIambassador& rtiAmb)
{
    InitInterClass();

    const vector<wstring> recv_TargetInfoReply = { L"requireID", L"targetNumber", L"targetName" };
    const vector<wstring> send_LogInInfo = { L"publisherType", L"deviceID", L"serviceNumber", L"logInName",
                                             L"affiliation" };

    if (!mapInteractionParameters(rtiAmb, InterClass_TargetInfoReply, InterClassHandle_TargetInfoReply,
        ParamNameHandleMap_TargetInfoReply, recv_TargetInfoReply))
        return false;
    if (!mapInteractionParameters(rtiAmb, InterClass_LogInInfo, InterClassHandle_LogInInfo,
        ParamNameHandleMap_LogInInfo, send_LogInInfo))
        return false;

    if (!subscribeInteraction(rtiAmb, InterClass_TargetInfoReply, InterClassHandle_TargetInfoReply))
        return false;

    if (!publishInteraction(rtiAmb, InterClass_LogInInfo, InterClassHandle_LogInInfo))
        return false;
    if (!subscribeInteraction(rtiAmb, InterClass_LogInInfo, InterClassHandle_LogInInfo))
        return false;

    return true;
}

void StartRTI()
{
    InitInterClass();
    try
    {
        std::wstring federationName(L"MAK-ONE-2025");
        std::wstring federationFile(L"RPR_FOM_v2.0_1516-2010.xml");
        std::wstring federateType(L"rtisimple1516e");
        std::wstring localSettingsDesignator;

        wcout << L"Using " << rtiName() << L" " << rtiVersion() << L"\n";

        theAmbData.clear();

        RTIambassadorFactory* rtiAmbFactory = new RTIambassadorFactory();
        std::auto_ptr<RTIambassador> rtiAmbAP = rtiAmbFactory->createRTIambassador();
        rtiAmb = rtiAmbAP.release();
        MyFederateAmbassador fedAmb(theAmbData);

        rtiAmb->evokeCallback(0.0);

        AttributeHandleValueMap attrValues;
        ParameterHandleValueMap paramValues;

        long count = 0;
        bool doConnect = true;
        bool connected = false;
        while (1)
        {
            if (doConnect)
            {   
                doConnect = false;

                if (!connect(*rtiAmb, fedAmb, localSettingsDesignator))
                {
                    wcout << L"Connect to RTI failed!" << endl;
                    continue;
                }

                listFederations(*rtiAmb);

                createFedEx(*rtiAmb, federationName, federationFile);

                joinFedEx(*rtiAmb, federateType, federationName);

                if (!publishAndSubscribeInteraction_DSTRecv(*rtiAmb))
                {
                    resignAndDestroy(*rtiAmb, federationName);
                    if (rtiAmb != 0)
                    {
                        delete rtiAmb;
                        rtiAmb = 0;
                    }
                }


                paramValues_LogInInfo.clear();

                connected = true;


            }
            else if (connected)
            {
                std::stringstream ss;
                ss << "1516-" << count++;
                std::string tag(ss.str());
                (void)tag;
                (void)paramValues;
                (void)attrValues;

                rtiAmb->evokeMultipleCallbacks(0.1, 0.5);
            }

#ifdef WIN32
            Sleep(2000);
#else
            sleep(2);
#endif
        }

        if (connected)
        {
            resignAndDestroy(*rtiAmb, federationName);
        }
    }
    catch (Exception& ex)
    {
        wcout << L"RTI Exception (main loop): " << ex.what() << endl;
    }
    if (rtiAmb != 0)
    {
        delete rtiAmb;
        rtiAmb = 0;
    }
}


namespace nesslab_backend_plugins
{
    // Sender   : YSU → Sub VRIS
    //                     ↓
    // Receiver :         VRIS

    /*
    namespace
    {
        // 행동(Master) 장비 <----> Sub_VRIS <--TCP-- VRIS, Port = 4010(임의)
        nesslabTcpServer* handposeTCPServer = nullptr;
        int                     handposePort = -1;

        vector<BYTE> handposeRecvBuffer;
        PacketHandlers packetHandlers;
    }

    enum LANG { KOR, ENG };
    string GetHandposeLableName(int idx, LANG language)
    {
        if (language == LANG::KOR)
        {
            switch (idx)
            {
            case 1:
                return "산개";
            case 2:
                return "집합(Assemble)";
            case 3:
                return "집합(Join me)";
            case 4:
                return "증속";
            case 5:
                return "Wedge Formation";
            case 6:
                return "가스";
            case 7:
                return "착검";
            case 8:
                return "적 발견";
            case 9:
                return "신속하게";
            case 10:
                return "정지";
            case 11:
                return "무릎 앉아";
            case 12:
                return "포복으로 이동";
            case 13:
                return "Pace Count";
            case 14:
                return "통신병 앞으로";
            case 15:
                return "Head count";
            case 16:
                return "위험지역 도착";
            case 17:
                return "움직이지마!";
            case 18:
                return "정지! 사주경계";
            case 19:
                return "수신완료";
            case 20:
                return "사격개시/(기관총)사격속도 조절";
            case 21:
                return "방향 또는 고각 변경";
            case 22:
                return "사격 위치 이동/화력 조정";
            case 23:
                return "사격중지";
            case 252:
                return "End Gesture";
            case 253:
                return "테스트 제스처(2-3-5-1)";
            case 254:
                return "Hold";
            case 255:
                return "Custom 제스처";
            default:
                return "Unknown";
            }
        }
        else
        {
            switch (idx)
            {
            case 1:
                return "dst_hand_freeze";
            case 2:
                return "dst_hand_enemy_in_sight";
            case 3: //2
                return "dst_hand_";
            case 4:
                return "dst_hand_change_direction";
            case 5: //4
                return "dst_hand_";
            case 6: //5
                return "dst_hand_";
            case 7:
                return "dst_hand_head_count";
            case 8:
                return "dst_hand_enemy_in_sight";
            case 9:
                return "dst_hand_message_acknowledged";
            case 10:
                return "dst_hand_radio_operator_forward";
            case 11:
                return "dst_hand_wedge_formation";
            case 12:    //21
                return "dst_hand_";
                //for test
            case 252:
                return "End Gesture";
            case 253:
                return "test_diguy_hand";
            case 254:
                return "dst_hand_hold";
            case 255:
                return "custom_hand_wedge_formation";
            default:
                return "default";
            }
        }
    };


    makVrf::DtArticulatedPartStateRepository* myRightWrist = nullptr;
    ostringstream handRight;
    ostringstream point("point");
    ostringstream testGesture("test_diguy_hand_l");

    //kcl 251229
    ostringstream holdGesture_L("dst_hand_hold_l");
    ostringstream holdGesture_R("dst_hand_hold_r");
    ostringstream pointingGesture("diguy_signal_join_me");

    DtDiGuyActionEvent myRightHandAction;
    DtDiGuyActionEvent myPointAction;
    DtDiGuyActionEvent myTestGestureAction;

    //kcl 251229
    DtDiGuyActionEvent myHoldGestureAction_L;
    DtDiGuyActionEvent myHoldGestureAction_R;
    DtDiGuyActionEvent myPointingGestureAction;

    ostringstream latestGesture;
    DtDiGuyActionEvent cleanGestureAction;
    */

    //RTINetworkPlugin* myOwner;

    //Constructor => Enage Human 스켈레톤 캐릭터 생성하면 실행
    RTINetworkPlugin::RTINetworkPlugin(const DtString& name,
        DtLocalObject* owner,
        DtSimulationServices* simManager,
        DtComponentDescriptor* desc,
        DtReaderWriterRegistry* parentRegistry)
        : DtActuatorComponent(name, owner, simManager, desc, parentRegistry)
        , myPlatformLocalObjectFacade(owner)
        , myLeftShoulder(0)
        , myRightShoulder(0)
        , myLeftWrist(0)
        , myAnimationInterval(4)
        , myTimeInInterval(0)
    {
        cout << "[RTINetworkPlugin][Trace] Constructor " << endl;
    }

    // destructor => Human 아닌 오브젝트를 생성하거나 다른 인간 캐릭터 생성하면 실행
    RTINetworkPlugin::~RTINetworkPlugin()
    {
        cout << "[RTINetworkPlugin][Trace] Destructor " << entity()->uuid().string() << endl;
    }

    bool RTINetworkPlugin::init()
    {
        cout << "[RTINetworkPlugin][Trace] Initialize " << endl;
        if (!DtActuatorComponent::init())
        {
            return false;
        }

        myLocalState = localStateRepository()->as<DtVrfMovingObjectStateRepository>();

        //InitPlugin();
        StartRTI();

        return true;
    }

    const char* RTINetworkPlugin::type() const
    {
        return RTI_NETWORK_PLUGIN_TYPE;
    }

    /*
    void RTINetworkPlugin::InitPlugin()
    {

    }
    */

    DtSimComponent* RTINetworkPlugin::creator(const DtString& name,
        DtLocalObject* owner,
        DtSimulationServices* simManager,
        DtComponentDescriptor* desc,
        DtReaderWriterRegistry* parentRegistry)
    {
        return new RTINetworkPlugin(name, owner, simManager, desc, parentRegistry);
    }

    double myPartAngle = -1.57;
    double myPartAngleRate = 0;

    void RTINetworkPlugin::tick()
    {
        if (!simulationServices())
        {
            objectConsoleError()("Uninitialized simulation services detected by component: %s\n",
                name().c_str());
            return;
        }

        return;
    }

    void RTINetworkPlugin::SetFOMData()
    {
        if (!rtiAmb)
            return;

        unsigned short pub = htons(3);
        unsigned short dev = htons(2);
        std::vector<unsigned char> serv = { 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A };
        std::vector<unsigned char> name = { 0x62, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A };
        std::vector<unsigned char> affi = { 0x63, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A };

        paramValues_LogInInfo[ParamNameHandleMap_LogInInfo[L"publisherType"]].setData(&pub, sizeof(unsigned short));
        paramValues_LogInInfo[ParamNameHandleMap_LogInInfo[L"deviceID"]].setData(&dev, sizeof(unsigned short));
        paramValues_LogInInfo[ParamNameHandleMap_LogInInfo[L"serviceNumber"]].setData(serv.data(), 10);
        paramValues_LogInInfo[ParamNameHandleMap_LogInInfo[L"logInName"]].setData(name.data(), 10);
        paramValues_LogInInfo[ParamNameHandleMap_LogInInfo[L"affiliation"]].setData(affi.data(), 10);
        //rtiAmb->sendInteraction(InterClassHandle_LogInInfo, paramValues_LogInInfo, makeTag("LogInInfo"));

        std::stringstream ss;
        ss << "1516-" << "LogInInfo";
        std::string tag(ss.str());

        VariableLengthData myTag(tag.c_str(), tag.size() + 1);
        rtiAmb->sendInteraction(InterClassHandle_LogInInfo, paramValues_LogInInfo, myTag);
    }



    void RTINetworkPlugin::SetLogInInfoData(std::vector<unsigned char> data)
    {
        unsigned short pubType = htons(1);
        unsigned short devID = htons(*reinterpret_cast<const uint16_t*>(&data[6]));

        paramValues_LogInInfo[ParamNameHandleMap_LogInInfo[L"publisherType"]].setData(&pubType, sizeof(unsigned short));
        paramValues_LogInInfo[ParamNameHandleMap_LogInInfo[L"deviceID"]].setData(&devID, sizeof(unsigned short));
        paramValues_LogInInfo[ParamNameHandleMap_LogInInfo[L"serviceNumber"]].setData(&data[8], 10);
        paramValues_LogInInfo[ParamNameHandleMap_LogInInfo[L"logInName"]].setData(&data[18], 10);
        paramValues_LogInInfo[ParamNameHandleMap_LogInInfo[L"affiliation"]].setData(&data[28], 10);
        //rtiAmb->sendInteraction(InterClassHandle_LogInInfo, paramValues_LogInInfo, makeTag("LogInInfo"));

        std::stringstream ss;
        ss << "1516-" << "LogInInfo";
        std::string tag(ss.str());

        VariableLengthData myTag(tag.c_str(), tag.size() + 1);
        rtiAmb->sendInteraction(InterClassHandle_LogInInfo, paramValues_LogInInfo, myTag);
    }

    static VariableLengthData makeTag(const char* suffix)
    {
        std::stringstream ss;
        ss << "1516-" << suffix;
        std::string tag(ss.str());

        return VariableLengthData(tag.c_str(), tag.size() + 1);
    }
}

void InitInterClass()
{
    vecInterClassName.clear();
    vecInterClassName.push_back(InterClass_TargetInfoReply);
    vecInterClassName.push_back(InterClass_LogInInfo);
}

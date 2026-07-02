/*******************************************************************************
** Copyright (c) 2018 MAK Technologies, Inc.
** All rights reserved.
*******************************************************************************/
//#define KCLTEST 1
//#if DtHLA

#include "traineeFom.h"

#include <vl/stateEncoderFactory.h>
#include <vl/stateDecoderFactory.h>
#include <vl/fom.h>
#include <vl/fomMapper.h>
#include <vrfExtObjects/extEntityStateRepository.h>
#include <vrfExtObjects/extendedAttributesInitializer.h>
#include <vrfExtObjects/vrfEntityPublisher.h>
#include <vrfExtObjects/reflectedExtEntityList.h>

#include <vl/exerciseConn.h>
#include <vl/exerciseConnInitializer.h>
#include <vl/reflectedEntityList.h>
#include <vl/entityStateRepository.h>
#include <vl/reflectedEntity.h>
#include <vl/fireInteraction.h>
#include <vl/topoView.h>
#include <vl/fomMapper.h>
#include <vl/environmentProcessRepository.h>
#include <vl/reflectedEnvironmentProcess.h>
#include <vl/reflectedEnvironmentProcessList.h>
#include <vl/signalInteraction.h>
#include <vl/setDataInteractionHLA.h>

#include <vlutil/vlProcessControl.h>
#include <vlutil/vlMiniDumper.h>
#include <vlutil/vlStringUtil.h>
#include <vlutil/vlSimpleKeyboard.h>
#include <vl/setDataInteractionHLA.h>

//[ Fom Interaction Class]
//>Send
#include "extTraineeSensor.h"
#include "extTraineeSensorEncoder.h"
#include "extTraineeSensorDecoder.h"
//>Receive
#include "extTrainingCtrl.h"
#include "extTrainingCtrlEncoder.h"
#include "extTrainingCtrlDecoder.h"

//Nesslab
#include "Nesslab/nesslabTcpServer.h"
#include "Nesslab/nesslabTcpClient.h"
#include "Nesslab/nesslabCommon.h"

#include <mutex>
#include <condition_variable>

using namespace nesslab_common;

// VRIS --Tcp--> TITS
namespace vris_to_tits_packets
{
#pragma pack(push, 1)
    struct TraineeInfoPacket
    {
        PacketHeader header;

        //Payload
        std::uint8_t serviceNumber[16] = {};
        //BYTE* serviceNumber = new BYTE[16];
        std::uint8_t trainState = 0;	// 0: Keep Alive, 1: Start, 2: Stop

        PacketTrailer trailer;

        TraineeInfoPacket()
        {
            //deviceID => 0x71 : VRIS

            header.deviceID = 0x71;     // TITS
            header.msgID = 0x78;        // Service Number Msg
                                        //msgType  => 0x00 : Reserved
            header.length = sizeof(TraineeInfoPacket);//0x18 (24)
        }
    };

#pragma pack(pop)
}

// TITS --Tcp--> VRIS
namespace tits_to_vris_packets
{
#pragma pack(push, 1)
    struct TraineeStatePacket
    {
        PacketHeader header;

        //Payload
        //BYTE* serviceNumber = new BYTE[16];
        std::uint8_t serviceNumber[16] = {};
        std::uint8_t heartbeat = 0;
        std::uint8_t concentration = 0;
        std::uint8_t stress = 0;
        std::uint8_t fatigue = 0;

        PacketTrailer trailer;//tail

        TraineeStatePacket()
        {

            header.deviceID = 0x73;// TITS
            //msgID    => 0x7a : 훈련자 생체 정보(4종)
            header.length = sizeof(TraineeStatePacket);//0x1b (27)
        }
    };

#pragma pack(pop)

#pragma pack(push, 1)
    struct PatchStatePacket
    {
        PacketHeader header;

        //Payload
        std::uint8_t helmetPatch = 0;
        std::uint8_t bodyPatch = 0;
        std::uint8_t handPatch = 0;

        PacketTrailer trailer;//tail

        PatchStatePacket()
        {

            header.deviceID = 0x73;// TITS
            header.msgID = 0x79;  // 패치 상태
            header.length = sizeof(PatchStatePacket);//0x0A (10)
        }
    };

#pragma pack(pop)

}

//VRIS --Tcp-> DSMSs
namespace vris_to_dsms_packets
{
#pragma pack(push, 1)
    struct TraineeStatePacket
    {
        PacketHeader header;

        //Payload
        std::uint8_t serviceNumber[16] = {};
        std::uint8_t heartbeat = 0;
        std::uint8_t concentration = 0;
        std::uint8_t stress = 0;
        std::uint8_t fatigue = 0;

        PacketTrailer trailer;//tail

        TraineeStatePacket()
        {
            header.deviceID = 0x71;	// VRIS
            header.msgID = 0x71;	// 훈련자 생체 데이터(4종)
            header.msgType = 0x00;
            header.length = sizeof(TraineeStatePacket);//0x1b (27)
        }
    };
#pragma pack(pop)

}

namespace TITS
{
    bool isTraineeIdReceived = false;

    thread netThread;   //Connection Control

    thread reconnectThread_tits;
    thread reconnectThread_dsms;

    uint16_t mySeatID = 0x00;
    std::uint8_t myServiceNumber[10] = {};

    namespace {
        //TITS -> VRIS, Port = 7272
        nesslabTcpClient* titsTcpClient = nullptr;
        std::string			titsIP = "127.0.0.1";
        int					titsPort = -1;
        std::vector<BYTE>	titsRecvBuffer;
        vris_to_tits_packets::TraineeInfoPacket titsTraineeInfoPkt;		//Send
        tits_to_vris_packets::TraineeStatePacket  titsTraineeStatePkt;	//Receive
        tits_to_vris_packets::PatchStatePacket  titsPatchStatePkt;	//Receive

        //VRIS ->DSMS , Port = 3100
        nesslabTcpClient* dsmsTcpClient = nullptr;
        std::string			dsmsIP = "127.0.0.1";
        int					dsmsPort = -1;
        vris_to_dsms_packets::TraineeStatePacket dsmsTraineeStatePkt;

        std::atomic_bool pluginStopping{ false };
        std::mutex dsmsMtx;
        std::condition_variable dsmsCv;

        std::mutex titsMtx;
        std::condition_variable titsCv;

        PacketHandlers packetHandlers;
    }


    DtExerciseConn* m_pExCon;
    const DtString theFomClass = "AlternateEntity";

    void TraineeFom::PrintTraineeSensor(extTraineeSensor inter)
    {
        SetConsoleOutputCP(949);
        std::cout << "\n[KCL] Device State Data" << std::endl;
        std::cout << "[KCL] Publish Type        " << inter.PublisherType() << std::endl;
        std::cout << "[KCL] Device ID           " << inter.deviceID() << std::endl;
        std::cout << "[KCL] Heartbeat           " << inter.Heartbeat() << std::endl;
        std::cout << "[KCL] Concentration       " << inter.Concentrativeness() << std::endl;
        std::cout << "[KCL] Stress              " << inter.Stress() << std::endl;
        std::cout << "[KCL] Fatigue             " << inter.Fatigue() << std::endl;
    }

    void TraineeFom::traineeFom(DtExerciseConn* exConn, bool runExtPropInit)
    {
        //DtFom* fom = exConn->fom();
        //DtFomMapper* mapper = exConn->fomMapper();
        DtInfo << "[KCL] Start Trainee State Fom\n";
        m_pExCon = exConn;

        DtExerciseConn::InitializationStatus status = DtExerciseConn::DtINIT_SUCCESS;
        DtClock* clock = exConn->clock();

        DtInfo << "[KCL] [" << clock->simTime() << ",  " << " 0] Initializing\n";
        exConn->setExecutionStatus(vrlstatus_initializing);
        exConn->drainInput(0.1);

        DtInfo << "[KCL] Exercise connection : " << runExtPropInit << "\n";

        if (status != 0)
            DtInfo << "[KCL] Error creating exercise connection.\n";

        //extTrainingCtrl::addCallback(exConn, OnTrainingCtrlCB, NULL);
        extTrainingCtrl::addCallback(exConn, TITS::OnTrainingCtrlCB, this);

        DtInfo << "[KCL] Add Training Control Callback\n";

        std::cout << "[" << clock->simTime() << ", " << " 0] Executing " << std::endl;
        exConn->setExecutionStatus(vrlstatus_executing);
        exConn->drainInput(0.1);

        DtExtendedAttributesInitializerPtr init(new DtEntityExtendedAttributesInitializer);
        init->addSupportedClass(theFomClass);

        DtVrfEntityPublisher::setInitializer(init);
        DtReflectedExtEntityList::setInitializer(init);

        if (runExtPropInit)
        {
            init->initialize(exConn);
        }

        initPlugin();
    }

    void TraineeFom::untraineeFom(DtExerciseConn* exConn)
    {
        pluginStopping.store(true, std::memory_order_release);
        titsCv.notify_all();
        dsmsCv.notify_all();

        if (netThread.joinable())
            netThread.join();

        if (reconnectThread_tits.joinable())
            reconnectThread_tits.join();

        if (reconnectThread_dsms.joinable())
            reconnectThread_dsms.join();


        if (titsTcpClient != nullptr)
        {
            std::lock_guard<std::mutex> titsLock(titsMtx);

            titsTcpClient->Disconnect();
            delete titsTcpClient;
            titsTcpClient = nullptr;
        }

        if (dsmsTcpClient != nullptr)
        {
            std::lock_guard<std::mutex> dsmsLock(dsmsMtx);

            dsmsTcpClient->Disconnect();
            delete dsmsTcpClient;
            dsmsTcpClient = nullptr;
        }

        /*
        DtFom* fom = exConn->fom();
        DtFomMapper* mapper = exConn->fomMapper();

        DtDebug << "[KCL] End Trinee State Fom\n";

        DtVrfEntityPublisher::setInitializer(DtExtendedAttributesInitializerPtr());
        DtReflectedExtEntityList::setInitializer(DtExtendedAttributesInitializerPtr());
        */
    }


    void TraineeFom::initPlugin()
    {

        pluginStopping.store(false, std::memory_order_release);
        titsRecvBuffer.clear();
        packetHandlers.clear();

        //Get ConfigFile Data
        {
            char titsBuf[256];
            char dsmsBuf[256];

            //TITS
            GetPrivateProfileString("TITS", "titsIP", INI_STRING_NOT_FOUND_DEFAULT.c_str(), titsBuf, sizeof(titsBuf), CONFIG_FILE_PATH.c_str());
            titsIP = titsBuf;
            titsPort = GetPrivateProfileInt("TITS", "titsPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());

            //DSMS
            GetPrivateProfileString("DSMS", "dsmsIP", INI_STRING_NOT_FOUND_DEFAULT.c_str(), dsmsBuf, sizeof(dsmsBuf), CONFIG_FILE_PATH.c_str());
            dsmsIP = dsmsBuf;
            dsmsPort = GetPrivateProfileInt("TITS", "dsmsPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());

            //TraineeData
            char aryServiceNumber[256];
            GetPrivateProfileString("TraineeData", "traineeID", INI_STRING_NOT_FOUND_DEFAULT.c_str(), aryServiceNumber, sizeof(aryServiceNumber), CONFIG_FILE_PATH.c_str());
            std::memcpy(&myServiceNumber, &aryServiceNumber, sizeof(myServiceNumber));
            mySeatID = GetPrivateProfileInt("TraineeData", "seatID", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());

            if (titsPort == INI_INT_NOT_FOUND_DEFAULT || dsmsPort == INI_INT_NOT_FOUND_DEFAULT || dsmsIP == INI_STRING_NOT_FOUND_DEFAULT)
                cout << (LOG_WARN_CHANNEL_NAME) << "[TitsPlugin][Error] Failed to read data from INI file. Using default value." << endl;

            cout << "[TitsPlugin][Debug] titsPort: " << titsPort << ", dsmsIP: " << dsmsIP << ", dsmsPort: " << dsmsPort << endl;
        }

        //Store packet data(데이터 받는 패킷만 처리 => 파싱, 콜백 처리용)
        {
            //TITS → VRIS : Trainee state info 
            RegisterPacketHandler<tits_to_vris_packets::TraineeStatePacket>(packetHandlers, (int)DeviceID::Tits, (uint8_t)VrisMsgID::TraineeStateMsgID, titsTraineeStatePkt,
                [this](const tits_to_vris_packets::TraineeStatePacket& packet) 
                {
                    OnTraineeStatePacket(packet);
                });

            //TITS → VRIS : Patch Device state info 
            RegisterPacketHandler<tits_to_vris_packets::PatchStatePacket>(packetHandlers, (int)DeviceID::Tits, (uint8_t)VrisMsgID::PatchStateMsgID, titsPatchStatePkt,
                [this](const tits_to_vris_packets::PatchStatePacket& packet)
                {
                    OnPatchStatePacket(packet);
                });
        }

        //Create and start TCP/UDP
        {

            netThread = std::thread([this]() {

                //TITS → VRIS
                titsTcpClient = new nesslabTcpClient(titsPort, titsIP);

                titsTcpClient->SetOnConnectionChanged(
                    [this](nesslabTcpClient::ConnectionState connectionState)
                    {
                        if (connectionState == nesslabTcpClient::ConnectionState::Connected)
                        {
                            OnTitsConnected();
                        }
                        else if (connectionState == nesslabTcpClient::ConnectionState::Disconnected)
                        {
                            OnTitsDisconnected();
                        }
                    }
                );

                titsTcpClient->SetOnDataReceived(
                    [this](const uint8_t* data, int len)
                    {
                        OnDataReceived(data, len);
                    });
                titsTcpClient->Connect();


                //VRIS → DSMS
                dsmsTcpClient = new nesslabTcpClient(dsmsPort, dsmsIP);
                dsmsTcpClient->SetOnConnectionChanged(
                    [this](nesslabTcpClient::ConnectionState connectionState)
                    {
                        if (connectionState == nesslabTcpClient::ConnectionState::Connected)
                        {
                            OnDsmsConnected();
                        }
                        else if (connectionState == nesslabTcpClient::ConnectionState::Disconnected)
                        {
                            OnDsmsDisconnected();
                        }
                    }
                );
                dsmsTcpClient->Connect();

                });
        }

        /*
                {

            netThread = std::thread([]() {

                //TITS → VRIS
                titsTcpClient = new nesslabTcpClient(titsPort, titsIP);

                titsTcpClient->SetOnConnectionChanged(
                    [](nesslabTcpClient::ConnectionState connectionState)
                    {
                        if (connectionState == nesslabTcpClient::ConnectionState::Connected)
                        {
                            OnTitsConnected();
                        }
                        else if (connectionState == nesslabTcpClient::ConnectionState::Disconnected)
                        {
                            OnTitsDisconnected();
                        }
                    }
                );

                titsTcpClient->SetOnDataReceived(
                    [](const uint8_t* data, int len)
                    {
                        OnDataReceived(data, len);
                    });
                titsTcpClient->Connect();


                //VRIS → DSMS
                dsmsTcpClient = new nesslabTcpClient(dsmsPort, dsmsIP);
                dsmsTcpClient->SetOnConnectionChanged(
                    [](nesslabTcpClient::ConnectionState connectionState)
                    {
                        if (connectionState == nesslabTcpClient::ConnectionState::Connected)
                        {
                            OnDsmsConnected();
                        }
                        else if (connectionState == nesslabTcpClient::ConnectionState::Disconnected)
                        {
                            OnDsmsDisconnected();
                        }
                    }
                );
                dsmsTcpClient->Connect();

                });
        }
        */
    }

    void OnTrainingCtrlCB(extTrainingCtrl* inter, void* usr)
    {
        SetConsoleOutputCP(949);
        std::uint8_t trainState = inter->getControlMessage();
        std::cout << "\n[KCL] Trainee State Data" << std::endl;
        std::cout << "[KCL] RealWorld Time      " << inter->getRealWorldTime() << std::endl;
        std::cout << "[KCL] Simulation Time     " << inter->getSimulationTime() << std::endl;
        std::cout << "[KCL] Control Message     " << trainState << std::endl;

        //SendDataToTITS(trainState);
        if (usr) {
            static_cast<TraineeFom*>(usr)->SendDataToTITS(trainState);
        }
    }

    //VRIS → TITS
    void TraineeFom::SendDataToTITS(BYTE trainState)
    {
        if (titsTcpClient == nullptr) return;

        std::memcpy(&titsTraineeInfoPkt.serviceNumber, myServiceNumber, sizeof(myServiceNumber));
        titsTraineeInfoPkt.trainState = trainState;

        const size_t size = sizeof(titsTraineeInfoPkt);
        unsigned char sendData[size];
        std::memcpy(sendData, &titsTraineeInfoPkt, size);

        bool result = titsTcpClient->SendData(sendData, size);
        if (result)
        {
            std::cout << "[TitsPlugin][Debug] Send data to TITS : ";
            std::cout << std::hex << std::uppercase;
            for (int i = 0; i < size; ++i)
                std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(sendData[i]) << " ";
            std::cout << std::dec << std::endl;

        }
        else
            cout << "[TitsPlugin][Error] Failed to send data to the TITS ";

    }

    // TITS → VRIS : 훈련자 생체 정보
    void TraineeFom::OnTraineeStatePacket(const tits_to_vris_packets::TraineeStatePacket& packet)
    {
        std::cout << "[TitsPlugin][Trace] onTraineeStatePacket (tits > vris) \n";

        //CP949 String
        std::string serviceNumberStr(reinterpret_cast<const char*>(packet.serviceNumber), sizeof(packet.serviceNumber));
        //Unicode16 String
        std::wcout.imbue(std::locale("kor"));
        std::wstring serviceNumberUniStr = Cp949ToUnicode(serviceNumberStr);
        std::wcout << L"serviceNumberStr: " << serviceNumberUniStr << std::endl;

        isTraineeIdReceived = true;

        SendDataToVRF();
        SendDataToDsms();
    }

    void TraineeFom::OnPatchStatePacket(const tits_to_vris_packets::PatchStatePacket& packet)
    {
        std::cout << "[TitsPlugin][Trace] onPatchStatePacket (tits > vris) \n";

        //TODO : Patch Attribute Setting
        //       어떻게 titsPlugin.cpp로 전달할 것인가?

        setPatchState(packet.helmetPatch, packet.bodyPatch, packet.handPatch);
    }

    //VRIS → VRF
    void TraineeFom::SendDataToVRF()
    {
        //DtNetLogInInfo10 tmpServiceNumber{ DtText = {} }
        DtNetLogInInfo10 tmpServiceNumber{};
        std::memcpy(tmpServiceNumber.DtText, titsTraineeStatePkt.serviceNumber, sizeof(titsTraineeStatePkt.serviceNumber));

        SendTraineeSensor(1, mySeatID, titsTraineeStatePkt.heartbeat, titsTraineeStatePkt.concentration, titsTraineeStatePkt.stress, titsTraineeStatePkt.fatigue);
    }
    

    //VRIS → DSMS
    void TraineeFom::SendDataToDsms()
    {
        if (dsmsTcpClient == nullptr) return;

        std::memcpy(&dsmsTraineeStatePkt.serviceNumber, myServiceNumber, sizeof(myServiceNumber));

        const size_t size = sizeof(dsmsTraineeStatePkt);
        unsigned char sendData[size];
        std::memcpy(sendData, &dsmsTraineeStatePkt, size);


        bool result = dsmsTcpClient->SendData(sendData, size);
        if (result)
        {

            std::cout << "[TitsPlugin][Debug] Send data to Dsms : ";
            std::cout << std::hex << std::uppercase;
            for (int i = 0; i < size; ++i)
                std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(sendData[i]) << " ";
            std::cout << std::dec << std::endl;

        }
        else
            cout << "[TitsPlugin][Error] Failed to send data to the Dsms ";
    }

    void TraineeFom::OnTitsConnected()
    {
        std::cout << "[TitsPlugin][Debug] TITS Connected " << std::endl;
        //SendDataToTits();
    }

    void TraineeFom::OnTitsDisconnected()
    {
        std::cout << "[TitsPlugin][Debug] TITS Disconnected " << std::endl;
        if (pluginStopping.load(std::memory_order_acquire)) return;


        if (reconnectThread_tits.joinable())
        {
            if (reconnectThread_tits.get_id() == std::this_thread::get_id())
                reconnectThread_tits.detach();
            else
                reconnectThread_tits.join();
        }

        reconnectThread_tits = std::thread([this]() {
            HandleTitsReconnectThr();
            });

        //reconnectThread_tits = thread(HandleTitsReconnectThr);

    }

    void TraineeFom::OnDsmsConnected()
    {
        //SendDataToDsms();
        std::cout << "[TitsPlugin][Debug] DSMS Connected " << std::endl;
    }

    void TraineeFom::OnDsmsDisconnected()
    {
        std::cout << "[TitsPlugin][Debug] DSMS Disconnected " << std::endl;
        if (pluginStopping.load(std::memory_order_acquire)) return;


        if (reconnectThread_dsms.joinable())
        {
            if (reconnectThread_dsms.get_id() == std::this_thread::get_id())
                reconnectThread_dsms.detach();
            else
                reconnectThread_dsms.join();
        }

        reconnectThread_dsms = std::thread([this]() {
            HandleDsmsReconnectThr();
            });

        //reconnectThread_dsms = thread(HandleDsmsReconnectThr);
    }

    void TraineeFom::HandleTitsReconnectThr()
    {
        if (pluginStopping.load(std::memory_order_acquire)) return;

        std::unique_lock<std::mutex> lock(titsMtx);

        bool status = titsCv.wait_for(lock, std::chrono::seconds(RECONNECT_DELAY_SEC), []() { return pluginStopping.load(std::memory_order_acquire); });
        if (status)
            return;

        if (titsTcpClient != nullptr)
            titsTcpClient->Connect();
    }

    void TraineeFom::HandleDsmsReconnectThr()
    {
        if (pluginStopping.load(std::memory_order_acquire)) return;

        std::unique_lock<std::mutex> lock(dsmsMtx);

        bool status = dsmsCv.wait_for(lock, std::chrono::seconds(RECONNECT_DELAY_SEC), []() { return pluginStopping.load(std::memory_order_acquire); });
        if (status)
            return;

        if (dsmsTcpClient != nullptr)
            dsmsTcpClient->Connect();
    }

    void TraineeFom::OnDataReceived(const uint8_t* data, int len)
    {

        std::cout << "[TitsPlugin][Debug] Received Data from TITS, Data Length : " << len << ", Data :";
        std::cout << std::hex << std::uppercase;
        for (int i = 0; i < len; ++i)
            std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
        std::cout << std::dec << std::endl;


        titsRecvBuffer.insert(titsRecvBuffer.end(), data, data + len);

        ParseReceiveBuffer(packetHandlers, titsRecvBuffer);
    }

    //CP949 -> UTF-16
    std::wstring TraineeFom::Cp949ToUnicode(const std::string& cp949Str) {

        if (cp949Str.empty())
            return {};


        int requiredSize = MultiByteToWideChar(CP_ACP, 0, cp949Str.c_str(), -1, NULL, 0);

        if (requiredSize <= 0) return {};

        std::wstring unicodeStr(requiredSize - 1, L'\0'); // 널 제외
        MultiByteToWideChar(CP_ACP, 0, cp949Str.c_str(), -1, const_cast<wchar_t*>(unicodeStr.c_str()), requiredSize);


        return unicodeStr;
    }

    void TraineeFom::setPatchState(byte helmet, byte body, byte hand)
    {
        if (myPlugin == nullptr)
            return;
        myPlugin->setHelmetPatchState(helmet);
        myPlugin->setBodyPatchState(body);
        myPlugin->setHandPatchState(hand);
        cout << "[FOM Mappter] Set Patch State : Helemt[ " << static_cast<int>(helmet) << " ] Body[ " << static_cast<int>(body) << " ] Hand[ " << static_cast<int>(hand) << " ]" << endl;
    }
    
    /*
    void TraineeFom::setHelmetPatchState(byte helmet)
    {
        if (myPlugin == nullptr)
            return;
        myPlugin->setHelmetPatchState(helmet);
        cout << "[FOM Mappter] Set Helmet Patch State : " << helmet << endl;
    }

    void TraineeFom::setBodyPatchState(byte body)
    {
        if (myPlugin == nullptr)
            return;
        myPlugin->setBodyPatchState(body);
        cout << "[FOM Mappter] Set Body Patch State  : " << body << endl;
    }

    void TraineeFom::setHandPatchState(byte hand)
    {
        if (myPlugin == nullptr)
            return;
        myPlugin->setHandPatchState(hand);
        cout << "[FOM Mappter] Set Hand Patch State  : " << hand << endl;
    }
    */

    void TraineeFom::setPluginClass(void* usr)
    {
        if (usr) {
            myPlugin = static_cast<nesslab_frontend_plugins::TITSPlugin*>(usr);
        }
    }


    void TraineeFom::SendTraineeSensor(unsigned short usPublisherType, unsigned short usDeviceID, std::uint8_t heartbeat, std::uint8_t concentration, std::uint8_t stress, std::uint8_t fatigue)
    {
        extTraineeSensor inter;
        inter.setPublisherType(DtNetU16(usPublisherType));
        inter.setDeviceID(DtNetU16(usDeviceID));
        inter.setHeartbeat(DtNetU32(heartbeat));
        inter.setConcentrativeness(DtNetU32(concentration));
        inter.setStress(DtNetU32(stress));
        inter.setFatigue(DtNetU32(fatigue));

        PrintTraineeSensor(inter);

        m_pExCon->sendStamped(inter);
    }


    //#endif
}


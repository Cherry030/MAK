/*******************************************************************************
** Copyright (c) 2018 MAK Technologies, Inc.
** All rights reserved.
*******************************************************************************/
//#define KCLTEST 1
//#if DtHLA

#include "deviceStateFom.h"

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
#include "extDeviceState.h"
#include "extDeviceStateEncoder.h"
#include "extDeviceStateDecoder.h"

//Nesslab
#include "Nesslab/nesslabTcpServer.h"
#include "Nesslab/nesslabTcpClient.h"
#include "Nesslab/nesslabCommon.h"

#include <mutex>
#include <condition_variable>
#include <random>
#include <cstdint>

using namespace nesslab_common;

//VRIS -> DSMS
namespace sub_to_vris_packets
{
    //인프라
#pragma pack(push, 1)
    struct InfraPostureStatePacket
    {
        PacketHeader header;
        uint16_t	seatID = 0x02;//사용X
        uint8_t     devType;
        std::uint8_t devState[3] = {};
        PacketTrailer trailer;

        InfraPostureStatePacket()
        {
            header.deviceID = (int)DeviceID::Subvris;
            header.msgID = (uint8_t)VrisMsgID::InfraPostureStateMsgID;
            header.length = sizeof(InfraPostureStatePacket);
        }
    };
#pragma pack(pop)

    //세미인프라
#pragma pack(push, 1)
    struct SemiInfraPostureStatePacket
    {
        PacketHeader header;
        uint16_t	seatID = 0x02;//사용X
        uint8_t     devType;
        std::uint8_t devState[3] = {};
        PacketTrailer trailer;

        SemiInfraPostureStatePacket()
        {
            header.deviceID = (int)DeviceID::Subvris;
            header.msgID = (uint8_t)VrisMsgID::SemiInfraPostureStateMsgID;
            header.length = sizeof(SemiInfraPostureStatePacket);
        }
    };
#pragma pack(pop)

    //모의총기
#pragma pack(push, 1)
    struct GunStatePacket
    {
        PacketHeader header;
        uint16_t	seatID = 0x02;//사용X
        std::uint8_t devState;
        PacketTrailer trailer;

        GunStatePacket()
        {
            header.deviceID = (int)DeviceID::Subvris;
            header.msgID = (uint8_t)VrisMsgID::GunStateMsgID;
            header.length = sizeof(GunStatePacket);
        }
    };
#pragma pack(pop)

    //햅틱슈트
#pragma pack(push, 1)
    struct HapticStatePacket
    {
        PacketHeader header;
        uint16_t	seatID = 0x02;//사용X
        std::uint8_t devState;
        PacketTrailer trailer;

        HapticStatePacket()
        {
            header.deviceID = (int)DeviceID::Subvris;
            header.msgID = (uint8_t)VrisMsgID::HapticStateMsgID;
            header.length = sizeof(HapticStatePacket);
        }
    };
#pragma pack(pop)

    //기동보조
#pragma pack(push, 1)
    struct ManeuverStatePacket
    {
        PacketHeader header;
        uint16_t	seatID = 0x02;//사용X
        std::uint8_t devState;
        PacketTrailer trailer;

        ManeuverStatePacket()
        {
            header.deviceID = (int)DeviceID::Subvris;
            header.msgID = (uint8_t)VrisMsgID::ManeuverStateMsgID;
            header.length = sizeof(ManeuverStatePacket);
        }
    };
#pragma pack(pop)

    //패치디바이스
#pragma pack(push, 1)
    struct PatchStatePacket
    {
        PacketHeader header;
        uint16_t	seatID = 0x02;//사용X
        std::uint8_t devState[3] = { };
        PacketTrailer trailer;

        PatchStatePacket()
        {
            header.deviceID = (int)DeviceID::Subvris;
            header.msgID = (uint8_t)VrisMsgID::PatchStateMsgID;
            header.length = sizeof(PatchStatePacket);
        }
    };
#pragma pack(pop)
}

//VRIS -> DSMS
namespace vris_to_dsms_packets
{
#pragma pack(push, 1)
    struct DeviceStatePacket
    {
        PacketHeader header;
        uint16_t	seatID = 0x02;//사용X
        /*
        uint8_t     treadmillType=0x11; //임의로 STMS : 추후에 트레드밀 플러그인에서 받아오ㅏ야함
        uint8_t     treadmill;
        uint8_t     manipulatorType = 0x31; //트레드밀과 마찬가지 (1: 받아오기 2: 트레드밀 종류에 따라서)
        uint8_t     manipulator;
        */
        uint8_t     postureType;
        uint8_t     posture;
        uint8_t     gun;
        uint8_t     haptic;
        uint8_t     patch[3];
        uint8_t     manuever;
        PacketTrailer trailer;

        DeviceStatePacket()
        {
            header.deviceID = (int)DeviceID::Vris;
            header.msgID = (uint8_t)VrisMsgID::DeviceStateMsgID;
            header.length = sizeof(DeviceStatePacket);
        }

        /*
        uint8_t treadmillVRF()
        {
            if (treadmill == 0x01 || treadmill == 0x02)   return 0x01;
            else if (treadmill == 0x03) return 0x02;
            else return 0x00;
        }

        uint8_t manipulatorVRF()
        {
            if (manipulator == 0x01 || manipulator == 0x02)   return 0x01;
            else if (manipulator == 0x03) return 0x02;
            else return 0x00;
        }

                uint8_t postureVRF()
        {
            return 0x00;
            //if()
        }

        uint8_t gunVRF()
        {
            return gun;
        }

        uint8_t hapticVRF()
        {
            return haptic;
        }

        uint8_t manueverVRF()
        {
            return manuever;
        }

        uint8_t patchVRF()
        {
            //모두 연결
            if (patch[0] == 0x01 && patch[1] == 0x01 && patch[2] == 0x01)
                return 0x01;
            //하나라도 에러면
            else if (patch[0] == 0x02 || patch[1] == 0x02 || patch[2] == 0x02)
                return 0x02;
            //하나라도 연결끊김이면
            else
                return 0x00;
        }
        */


    };
#pragma pack(pop)
}

namespace DEV
{
    bool isTraineeIdReceived = false;
    thread netThread;
    thread reconnectThread;
    thread sendStateThread;

    namespace {

        //Sub VRIS -> VRIS, Port = 미정
        nesslabTcpServer* subTcpServer = nullptr;
        int					subPort = -1;
        std::vector<BYTE>	subRecvBuffer{};
        sub_to_vris_packets::InfraPostureStatePacket	    infraPostureStatePkt;
        sub_to_vris_packets::SemiInfraPostureStatePacket	semiinfraPostureStatePkt;
        sub_to_vris_packets::GunStatePacket	                gunStatePkt;
        sub_to_vris_packets::HapticStatePacket	            hapticStatePkt;
        sub_to_vris_packets::ManeuverStatePacket	        maneuverStatePkt;
        sub_to_vris_packets::PatchStatePacket	            patchStatePkt;

        //VRIS -> DSMS, Port = 3006
        nesslabTcpClient* dsmsTcpClient = nullptr;
        std::string			dsmsIP = "127.0.0.1";
        int					dsmsPort = -1;
        vris_to_dsms_packets::DeviceStatePacket dsmsDeviceStatePkt;


        std::atomic_bool pluginStopping{ false };
        std::mutex dsmsMtx;
        std::condition_variable dsmsCv;

        std::mutex vrfMtx;
        std::condition_variable vrfCv;

        PacketHandlers packetHandlers;

        DevStateFom::DeviceStateInfo* prevDevStateInfo;
        DevStateFom::DeviceStateInfo* currentDevStateInfo;
        
        uint8_t latestSendPosture = 0x99;
        uint8_t latestSendPostures[3] = { 0x99, 0x99, 0x99 };
    }


    DtExerciseConn* m_pExCon;
    const DtString theFomClass = "AlternateEntity";

    void DevStateFom::PrintDeviceState(extDeviceState inter)
    {
        SetConsoleOutputCP(949);
        std::cout << "\n[KCL] Send Device State Data" << std::endl;

        std::cout << "[KCL] Publisher Type      " << inter.publisherType() << std::endl;
        std::cout << "[KCL] Device ID           " << inter.deviceID() << std::endl;
        std::cout << "[KCL] Treadmill           " << inter.getTreadmillStatus() << std::endl;
        std::cout << "[KCL] Manipulator         " << inter.getMainpulatorStatus() << std::endl;
        std::cout << "[KCL] MotionTracking      " << inter.getMotionTrackingStatus() << std::endl;
        std::cout << "[KCL] MockGun             " << inter.getMockupGunStatus() << std::endl;
        std::cout << "[KCL] HapticSuite         " << inter.getHapticSuiteStatus() << std::endl;
        std::cout << "[KCL] WearablePatch       " << inter.getWearblePatchStatus() << std::endl;
        std::cout << "[KCL] MoneuverAssistant   " << inter.getMoneuverAssistantStatus() << std::endl;
    }

    void DevStateFom::deviceStateFom(DtExerciseConn* exConn, bool runExtPropInit)
    {
        DtFom* fom = exConn->fom();
        DtFomMapper* mapper = exConn->fomMapper();
        DtInfo << "[KCL] Start Login Fom Mapper\n";
        m_pExCon = exConn;

        DtExerciseConn::InitializationStatus status = DtExerciseConn::DtINIT_SUCCESS;
        DtClock* clock = exConn->clock();

        DtInfo << "[KCL] [" << clock->simTime() << ",  " << " 0] Initializing\n";
        exConn->setExecutionStatus(vrlstatus_initializing);
        exConn->drainInput(0.1);

        DtInfo << "[KCL] Exercise connection : " << runExtPropInit << "\n";

        if (status != 0)
            DtInfo << "[KCL] Error creating exercise connection.\n";

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

    void DevStateFom::undeviceStateFom(DtExerciseConn* exConn)
    {
        pluginStopping.store(true, std::memory_order_release);
        dsmsCv.notify_all();

        if (netThread.joinable())
            netThread.join();

        if (reconnectThread.joinable())
            reconnectThread.join();

        if (sendStateThread.joinable())
            sendStateThread.join();

        if (subTcpServer != nullptr)
        {
            subTcpServer->ServerStop();
            delete subTcpServer;
            subTcpServer = nullptr;
        }

        if (dsmsTcpClient != nullptr)
        {
            std::lock_guard<std::mutex> dsmsLock(dsmsMtx);

            dsmsTcpClient->Disconnect();
            delete dsmsTcpClient;
            dsmsTcpClient = nullptr;
        }

        DtFom* fom = exConn->fom();
        DtFomMapper* mapper = exConn->fomMapper();

        DtDebug << "[KCL] End Device State Fom\n";

        DtVrfEntityPublisher::setInitializer(DtExtendedAttributesInitializerPtr());
        DtReflectedExtEntityList::setInitializer(DtExtendedAttributesInitializerPtr());
    }

    void DevStateFom::initPlugin()
    {
        pluginStopping.store(false, std::memory_order_release);
        subRecvBuffer.clear();
        packetHandlers.clear();

        //Get ConfigFile Data
        {
            char databuf[256];

            //Device State : Sub VRIS
            subPort = GetPrivateProfileInt("SUB_VRIS", "subPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());

            //DSMS
            GetPrivateProfileString("DSMS", "dsmsIP", INI_STRING_NOT_FOUND_DEFAULT.c_str(), databuf, sizeof(databuf), CONFIG_FILE_PATH.c_str());
            dsmsIP = databuf;
            dsmsPort = GetPrivateProfileInt("SUB_VRIS", "dsmsPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());

            //TraineeData
            dsmsDeviceStatePkt.seatID = GetPrivateProfileInt("TraineeData", "seatID", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());

            if (subPort == INI_INT_NOT_FOUND_DEFAULT || dsmsPort == INI_INT_NOT_FOUND_DEFAULT || dsmsDeviceStatePkt.seatID == INI_INT_NOT_FOUND_DEFAULT || dsmsIP == INI_STRING_NOT_FOUND_DEFAULT)
                cout << (LOG_WARN_CHANNEL_NAME) << "[DeviceStatePlugin][Error] Failed to read data from INI file. Using default value." << endl;

            cout << "[DeviceStatePlugin][Debug] subPort: " << subPort << ", dsmsIP: " << dsmsIP << ", dsmsPort: " << dsmsPort << ", seatID: " << dsmsDeviceStatePkt.seatID << endl;
        }

        //Store packet data(데이터 받는 패킷만 처리 => 파싱, 콜백 처리용)
        {
            //Sub VRIS -> VRIS
            RegisterPacketHandler<sub_to_vris_packets::InfraPostureStatePacket>(packetHandlers, infraPostureStatePkt.header.deviceID, infraPostureStatePkt.header.msgID, infraPostureStatePkt,
                [this](const sub_to_vris_packets::InfraPostureStatePacket& packet)
                {
                    OnDeviceStatePacket(packet);
                }
            );

            RegisterPacketHandler<sub_to_vris_packets::SemiInfraPostureStatePacket>(packetHandlers, semiinfraPostureStatePkt.header.deviceID, semiinfraPostureStatePkt.header.msgID, semiinfraPostureStatePkt,
                [this](const sub_to_vris_packets::SemiInfraPostureStatePacket& packet)
                {
                    OnDeviceStatePacket(packet);
                }
            );

            RegisterPacketHandler<sub_to_vris_packets::GunStatePacket>(packetHandlers, gunStatePkt.header.deviceID, gunStatePkt.header.msgID, gunStatePkt,
                [this](const sub_to_vris_packets::GunStatePacket& packet)
                {
                    OnDeviceStatePacket(packet);
                }
            );

            RegisterPacketHandler<sub_to_vris_packets::HapticStatePacket>(packetHandlers, hapticStatePkt.header.deviceID, hapticStatePkt.header.msgID, hapticStatePkt,
                [this](const sub_to_vris_packets::HapticStatePacket& packet)
                {
                    OnDeviceStatePacket(packet);
                }
            );

            RegisterPacketHandler<sub_to_vris_packets::ManeuverStatePacket>(packetHandlers, maneuverStatePkt.header.deviceID, maneuverStatePkt.header.msgID, maneuverStatePkt,
                [this](const sub_to_vris_packets::ManeuverStatePacket& packet)
                {
                    OnDeviceStatePacket(packet);
                }
            );

            RegisterPacketHandler<sub_to_vris_packets::PatchStatePacket>(packetHandlers, patchStatePkt.header.deviceID, patchStatePkt.header.msgID, patchStatePkt,
                [this](const sub_to_vris_packets::PatchStatePacket& packet)
                {
                    OnDeviceStatePacket(packet);
                }
            );
        }

        //Create and start TCP/UDP
        {
            netThread = std::thread([this]() {

                //RFID -> VRIS  
                subTcpServer = new nesslabTcpServer(subPort);
                subTcpServer->SetOnDataReceived(
                    [this](const std::string& clientAddr, const uint8_t* data, int len)
                    {
                        OnDataReceived(clientAddr, data, len);
                    });
                subTcpServer->ServerStart();


                //VRIS -> DSMS
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

        //Send Device State To VRF
        if (sendStateThread.joinable())
        {
            if (sendStateThread.get_id() == std::this_thread::get_id())
                sendStateThread.detach();
            else
                sendStateThread.join();
        }
        //sendStateThread = thread(HandleSendDeviceStateThr);

        sendStateThread = std::thread([this]() {
            HandleSendDeviceStateThr();
            });

        prevDevStateInfo = new DEV::DevStateFom::DeviceStateInfo(latestTreadmillType, latestTreadmillState, latestManipulatorState, latestPatchState);
        currentDevStateInfo = new DEV::DevStateFom::DeviceStateInfo(latestTreadmillType, latestTreadmillState, latestManipulatorState, latestPatchState);
    }

    //VRIS → VRF
    void DevStateFom::SendDataToVRF()
    {

        SendDeviceState(1, dsmsDeviceStatePkt.seatID, currentDevStateInfo->treadmillState, currentDevStateInfo->manipulatorState, 
                                                    postureVRF(dsmsDeviceStatePkt.posture),
                                                    gunVRF(dsmsDeviceStatePkt.gun), 
                                                    hapticVRF(dsmsDeviceStatePkt.haptic), 
                                                    patchVRF(dsmsDeviceStatePkt.patch), 
                                                    manueverVRF(dsmsDeviceStatePkt.manuever));

        
        cout << "[DeviceStatePlugin][KCL] SendData "
            << static_cast<int>(currentDevStateInfo->treadmillState) << " "
            << static_cast<int>(currentDevStateInfo->manipulatorState) << " "
            << static_cast<int>(postureVRF(dsmsDeviceStatePkt.posture)) << " "
            << static_cast<int>(gunVRF(dsmsDeviceStatePkt.gun)) << " "
            << static_cast<int>(hapticVRF(dsmsDeviceStatePkt.haptic)) << " "
            << static_cast<int>(patchVRF(dsmsDeviceStatePkt.patch)) << " "
            << static_cast<int>(manueverVRF(dsmsDeviceStatePkt.manuever)) << endl;
            
    }

    //VRIS → DSMS
    void DevStateFom::SendDataToDsms()
    {
        //if (!isTraineeIdReceived || dsmsTcpClient == nullptr) return;
        if (dsmsTcpClient == nullptr) return;

        /*
        dsmsDeviceStatePkt.postureType = currentDevStateInfo->postureType;
        std::memcpy(dsmsDeviceStatePkt.posture, &currentDevStateInfo->postureState, sizeof(currentDevStateInfo->postureState));
        dsmsDeviceStatePkt.gun = currentDevStateInfo->gunState;
        dsmsDeviceStatePkt.haptic = currentDevStateInfo->hapticState;
        std::memcpy(dsmsDeviceStatePkt.patch, &currentDevStateInfo->patchState, sizeof(currentDevStateInfo->patchState));
        dsmsDeviceStatePkt.manuever = currentDevStateInfo->maneuverState;
        */
            
        const size_t size = sizeof(dsmsDeviceStatePkt);
        unsigned char sendData[size];
        std::memcpy(sendData, &dsmsDeviceStatePkt, size);

        /*
        if (std::find(IsPostureMasterState.begin(), IsPostureMasterState.end(), dsmsDeviceStatePkt.posture) != IsPostureMasterState.end())
            latestSendPosture[0] = dsmsDeviceStatePkt.posture;
        else if (std::find(IsPostureClient1State.begin(), IsPostureClient1State.end(), dsmsDeviceStatePkt.posture) != IsPostureClient1State.end())
            latestSendPosture[1] = dsmsDeviceStatePkt.posture;
        else if (std::find(IsPostureClient2State.begin(), IsPostureClient2State.end(), dsmsDeviceStatePkt.posture) != IsPostureClient2State.end())
            latestSendPosture[2] = dsmsDeviceStatePkt.posture;
        */

        bool result = dsmsTcpClient->SendData(sendData, size);
        if (result)
        {
            std::cout << "[DeviceStatePlugin][Debug] Send data to Dsms : ";
            std::cout << std::hex << std::uppercase;
            for (int i = 0; i < size; ++i)
                std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(sendData[i]) << " ";
            std::cout << std::dec << std::endl;
        }
        else
            cout << "[DeviceStatePlugin][Error] Failed to send data to the Dsms ";

        if (std::find(IsPostureMasterState.begin(), IsPostureMasterState.end(), dsmsDeviceStatePkt.posture) != IsPostureMasterState.end())
        {
            //std::cout << "[KCL][M] Latest Send Posture Changed  : " << static_cast <int>(latestSendPostures[0]);
            latestSendPostures[0] = dsmsDeviceStatePkt.posture;
            latestSendPosture = latestSendPostures[0];
            //std::cout << " > " << static_cast <int>(latestSendPostures[0]) << std::endl;
        }
        else if (std::find(IsPostureClient1State.begin(), IsPostureClient1State.end(), dsmsDeviceStatePkt.posture) != IsPostureClient1State.end())
        {
            //std::cout << "[KCL][C1] Latest Send Posture Changed  : " << static_cast <int>(latestSendPostures[1]);
            latestSendPostures[1] = dsmsDeviceStatePkt.posture;
            latestSendPosture = latestSendPostures[1];
            //std::cout << " > " << static_cast <int>(latestSendPostures[1]) << std::endl;
        }
        else if (std::find(IsPostureClient2State.begin(), IsPostureClient2State.end(), dsmsDeviceStatePkt.posture) != IsPostureClient2State.end())
        {
            //std::cout << "[KCL][C2] Latest Send Posture Changed  : " << static_cast <int>(latestSendPostures[2]);
            latestSendPostures[2] = dsmsDeviceStatePkt.posture;
            latestSendPosture = latestSendPostures[2];
            //std::cout << " > " << static_cast <int>(latestSendPostures[2]) << std::endl;
        }
    }

    void DevStateFom::OnDsmsConnected()
    {
        std::cout << "[DeviceStatePlugin][Debug] DSMS Connected " << std::endl;
    }

    void DevStateFom::OnDsmsDisconnected()
    {
        std::cout << "[DeviceStatePlugin][Debug] DSMS Disconnected " << std::endl;
        if (pluginStopping.load(std::memory_order_acquire)) return;


        if (reconnectThread.joinable())
        {
            if (reconnectThread.get_id() == std::this_thread::get_id())
                reconnectThread.detach();
            else
                reconnectThread.join();
        }

        //reconnectThread = thread(HandleDsmsReconnectThr, this);

        reconnectThread = std::thread([this]() {
            HandleDsmsReconnectThr();
            });

    }

    void DevStateFom::OnDeviceStatePacket(const sub_to_vris_packets::InfraPostureStatePacket& packet)
    {
       // cout << "[DeviceStatePlugin][Trace] Infra [" << packet.devType << "] State : " << static_cast<int> (packet.devState[0]) <<  ", " <<
       //     static_cast<int> (packet.devState[1]) << ", " << static_cast<int> (packet.devState[2]) << endl;
         
        std::vector<BYTE> tmpState = { packet.devState[0], packet.devState[1], packet.devState[2] };
        SetDeviceStateInfo(packet.devType, tmpState);

        dsmsDeviceStatePkt.postureType = (BYTE)DeviceID::Iars;
        //std::memcpy(&dsmsDeviceStatePkt.posture, &tmpState, sizeof(tmpState));
    }

    void DevStateFom::OnDeviceStatePacket(const sub_to_vris_packets::SemiInfraPostureStatePacket& packet)
    {
        cout << "[DeviceStatePlugin][Trace] Semi Infra [" << packet.devType << "] State : " << packet.devState[0] << ", " <<
            packet.devState[1] << ", " << packet.devState[2] << endl;

        std::vector<BYTE> tmpState = { packet.devState[0], packet.devState[1], packet.devState[2] };
        SetDeviceStateInfo(packet.devType, tmpState);

        dsmsDeviceStatePkt.postureType = (BYTE)DeviceID::Sars;
        //std::memcpy(&dsmsDeviceStatePkt.posture, &tmpState, sizeof(tmpState));
    }

    void DevStateFom::OnDeviceStatePacket(const sub_to_vris_packets::GunStatePacket& packet)
    {
        cout << "[DeviceStatePlugin][Trace] Gun State : " << packet.devState << endl;

        std::vector<BYTE> tmpState = { packet.devState };
        SetDeviceStateInfo((BYTE)DeviceID::Hagh, tmpState);

        //dsmsDeviceStatePkt.gun = packet.devState;
    }

    void DevStateFom::OnDeviceStatePacket(const sub_to_vris_packets::HapticStatePacket& packet)
    {
        cout << "[DeviceStatePlugin][Trace] Haptic State : " << packet.devState << endl;

        std::vector<BYTE> tmpState = { packet.devState };
        SetDeviceStateInfo((BYTE)DeviceID::Hsth, tmpState);

        //dsmsDeviceStatePkt.haptic = packet.devState;
    }

    void DevStateFom::OnDeviceStatePacket(const sub_to_vris_packets::ManeuverStatePacket& packet)
    {
        cout << "[DeviceStatePlugin][Trace] Maneuver State : " << packet.devState << endl;

        std::vector<BYTE> tmpState = { packet.devState };
        SetDeviceStateInfo((BYTE)DeviceID::Madh, tmpState);

        //dsmsDeviceStatePkt.manuever = packet.devState;
    }

    void DevStateFom::OnDeviceStatePacket(const sub_to_vris_packets::PatchStatePacket& packet)
    {
        cout << "[DeviceStatePlugin][Trace] Patch Device State : " << 
            static_cast<unsigned char>(packet.devState[0]) << ", " <<
            static_cast<unsigned char>(packet.devState[1]) << ", " << 
            static_cast<unsigned char>(packet.devState[2]) << endl;

        std::vector<BYTE> tmpState = { packet.devState[0], packet.devState[1], packet.devState[2] };
        SetDeviceStateInfo((BYTE)DeviceID::Tits, tmpState);

        //std::memcpy(&dsmsDeviceStatePkt.patch, &tmpState, sizeof(tmpState));
    }

    uint8_t DevStateFom::ResolveValue(uint8_t prev, uint8_t current)
    {
        // 둘 다 Not Connected
        if (prev == 0x99 && current == 0x99)
            return 0x99; 

        // 현재 끊겼지만 이전에 값이 있으면 
        else if (current == 0x99 && prev != 0x99) 
            return prev; 

        // 나머지
        return current;
    }
    void DevStateFom::SetSendDataPacket()
    {
        // 현재와 비교 
        // 트레드밀 & 매니퓰레이터 : dsms에서 직접 받을 것
        dsmsDeviceStatePkt.postureType = currentDevStateInfo->postureType;
        cout << "[DeviceStatePlugin][KCL] Posture " << static_cast<int>(prevDevStateInfo->postureState) << " > " << static_cast<int>(currentDevStateInfo->postureState) << endl;
        //안들어옴
        if (currentDevStateInfo->postureType == 0x00)
            dsmsDeviceStatePkt.posture = 0x99;
        else
            dsmsDeviceStatePkt.posture = ResolveValue(prevDevStateInfo->postureState, currentDevStateInfo->postureState);
        dsmsDeviceStatePkt.gun      = ResolveValue(prevDevStateInfo->gunState, currentDevStateInfo->gunState);
        dsmsDeviceStatePkt.haptic = ResolveValue(prevDevStateInfo->hapticState, currentDevStateInfo->hapticState);
        dsmsDeviceStatePkt.patch[0] = ResolveValue(prevDevStateInfo->patchState[0], currentDevStateInfo->patchState[0]);
        dsmsDeviceStatePkt.patch[1] = ResolveValue(prevDevStateInfo->patchState[1], currentDevStateInfo->patchState[1]);
        dsmsDeviceStatePkt.patch[2] = ResolveValue(prevDevStateInfo->patchState[2], currentDevStateInfo->patchState[2]);
        dsmsDeviceStatePkt.manuever = ResolveValue(prevDevStateInfo->maneuverState, currentDevStateInfo->maneuverState);
    }

    void DevStateFom::HandleSendDeviceStateThr()
    {
        while (!pluginStopping) 
        {
            SetSendDataPacket();
            SendDataToVRF();

            if (dsmsTcpClient != nullptr)
                SendDataToDsms();

            /*
            // 전송 후에는 init
            cout << "[DeviceStatePlugin][KCL] Previous "
                << static_cast<int>(prevDevStateInfo->treadmillState) << " "
                << static_cast<int>(prevDevStateInfo->manipulatorState) << " "
                << static_cast<int>(prevDevStateInfo->postureState) << " "
                << static_cast<int>(prevDevStateInfo->gunState) << " "
                << static_cast<int>(prevDevStateInfo->hapticState) << " "
                << static_cast<int>(prevDevStateInfo->patchState[0]) << " "
                << static_cast<int>(prevDevStateInfo->patchState[1]) << " "
                << static_cast<int>(prevDevStateInfo->patchState[2]) << " "
                << static_cast<int>(prevDevStateInfo->maneuverState) << endl;

            cout << "[DeviceStatePlugin][KCL] Current  "
                << static_cast<int>(currentDevStateInfo->treadmillState) << " "
                << static_cast<int>(currentDevStateInfo->manipulatorState) << " "
                << static_cast<int>(currentDevStateInfo->postureState) << " "
                << static_cast<int>(currentDevStateInfo->gunState) << " "
                << static_cast<int>(currentDevStateInfo->hapticState) << " "
                << static_cast<int>(currentDevStateInfo->patchState[0]) << " "
                << static_cast<int>(currentDevStateInfo->patchState[1]) << " "
                << static_cast<int>(currentDevStateInfo->patchState[2]) << " "
                << static_cast<int>(currentDevStateInfo->maneuverState) << endl;

            cout << endl;
            */
            delete prevDevStateInfo; 
            prevDevStateInfo = currentDevStateInfo;
            currentDevStateInfo = new DEV::DevStateFom::DeviceStateInfo(latestTreadmillType, latestTreadmillState, latestManipulatorState, latestPatchState);

            //std::copy(std::begin(currentDevStateInfo->patchState), std::end(currentDevStateInfo->patchState), latestPatchState);

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void DevStateFom::HandleDsmsReconnectThr()
    {
        if (pluginStopping.load(std::memory_order_acquire)) return;

        std::unique_lock<std::mutex> lock(dsmsMtx);

        bool status = dsmsCv.wait_for(lock, std::chrono::seconds(RECONNECT_DELAY_SEC), []() { return pluginStopping.load(std::memory_order_acquire); });
        if (status)
            return;

        if (dsmsTcpClient != nullptr)
            dsmsTcpClient->Connect();
    }



    void DevStateFom::SetDeviceStateInfo(BYTE type, std::vector<BYTE> state)
    {

        switch (type)
        {
            //VRE Message로 올것 
            //트레드밀 & 매니퓰레이터
        case (BYTE)DeviceID::Stms:
        case (BYTE)DeviceID::Mtms:
            currentDevStateInfo->treadmillType = type;

            currentDevStateInfo->treadmillState = treadmillVRF(state[0]);
            currentDevStateInfo->manipulatorState = manipulatorVRF(state[1]);
            break;
            //자세행동인식
        case (BYTE)DeviceID::Iars:
        case (BYTE)DeviceID::Sars:
            currentDevStateInfo->postureType = type;
            // Master 값 변경
            if (latestSendPostures[0] != state[0])
            {
                currentDevStateInfo->postureState = state[0];
                std::cout << "[POSTURE][KCL] Master : " << static_cast<int>(currentDevStateInfo->postureState) << std::endl;
            }

            // C1 값 변경
            else if (latestSendPostures[1] != state[1])
            {
                currentDevStateInfo->postureState = state[1];
                std::cout << "[POSTURE][KCL] Client 1 : " << static_cast<int>(currentDevStateInfo->postureState) << std::endl;
            }

            // C2 값 변경
            else if (latestSendPostures[2] != state[2])
            {
                currentDevStateInfo->postureState = state[2];
                std::cout << "[POSTURE][KCL] Client 2 : " << static_cast<int>(currentDevStateInfo->postureState) << std::endl;
            }
            else
            {
                currentDevStateInfo->postureState = latestSendPosture;
            }
            break;
            
            //모의총기
        case (BYTE)DeviceID::Hagh:
            currentDevStateInfo->gunState = state[0];
            break;
            //햅틱슈트
        case (BYTE)DeviceID::Hsth:
            currentDevStateInfo->hapticState = state[0];
            break;
            //패치디바이스
        case (BYTE)DeviceID::Tits:
            std::copy(state.begin(), state.end(), currentDevStateInfo->patchState);
            break;
            //기동보조
        case (BYTE)DeviceID::Madh:
            currentDevStateInfo->maneuverState = state[0];
            break;
        }
    }


    void DevStateFom::OnDataReceived(const std::string& clientAddr, const uint8_t* data, int len)
    {
        /*
        std::cout << "[DeviceStatePlugin][Debug] Received Data from Sub VRIS, Data Length : " << len << ", Data :";
        std::cout << std::hex << std::uppercase;
        for (int i = 0; i < len; ++i)
            std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
        std::cout << std::dec << std::endl;
        */

        subRecvBuffer.insert(subRecvBuffer.end(), data, data + len);

        ParseReceiveBuffer(packetHandlers, subRecvBuffer);
    }

    //CP949 -> UTF-16
    std::wstring DevStateFom::Cp949ToUnicode(const std::string& cp949Str) {

        if (cp949Str.empty())
            return {};


        int requiredSize = MultiByteToWideChar(CP_ACP, 0, cp949Str.c_str(), -1, NULL, 0);

        if (requiredSize <= 0) return {};

        std::wstring unicodeStr(requiredSize - 1, L'\0'); // 널 제외
        MultiByteToWideChar(CP_ACP, 0, cp949Str.c_str(), -1, const_cast<wchar_t*>(unicodeStr.c_str()), requiredSize);


        return unicodeStr;
    }

    void DevStateFom::SendDeviceState(unsigned short usPublisherType, unsigned short usDeviceID, byte bTreadmill, byte bmanipulator, byte bPosture, byte bGun, byte bHaptic, byte bPatch, byte bManeuver)
    {
        extDeviceState inter;

        inter.setPublisherType(DtNetU16(usPublisherType));
        inter.setDeviceID(DtNetU16(usDeviceID));

        inter.setTreadmill(bTreadmill);
        inter.setMainpulator(bmanipulator);
        inter.setMotionTracking(bPosture);
        inter.setMockupGun(bGun);
        inter.setHapticSuite(bHaptic);
        inter.setWearblePatch(bPatch);
        inter.setMoneuverAssistant(bManeuver);

        /*
        PrintDeviceState(inter);
        
        cout << "[DeviceStatePlugin][KCL] To VRF " <<
            "Treadmill      " << static_cast<int>(bTreadmill) << "\n" <<
            "Manipulator    " << static_cast<int>(bmanipulator) << "\n" <<
            "Posture        " << static_cast<int>(bPosture) << "\n" <<
            "Gun            " << static_cast<int>(bGun) << "\n" <<
            "Haptic         " << static_cast<int>(bHaptic) << "\n" <<
            "Patch          " << static_cast<int>(bPatch) << "\n" <<
            "Maneuver       " << static_cast<int>(bManeuver) << endl;
        */
        //return;
        m_pExCon->sendStamped(inter);
    }

    void DevStateFom::setTreadmillType(byte val)
    {
        cout << "[FOM Mapper] Set Treadmill Type : " << static_cast<int>(val) << endl;
        latestTreadmillType = val;
        //SetDeviceStateInfo((BYTE)DeviceID::Stms, { val });
    }

    void DevStateFom::setTreadmillState(byte val)
    {
        cout << "[FOM Mapper] Set Treadmill State : " << static_cast<int>(val) << endl;
        latestTreadmillState = val;

        if (latestTreadmillType == (BYTE)DeviceID::Stms)
            SetDeviceStateInfo((BYTE)DeviceID::Stms, { val });
        else if (latestTreadmillType == (BYTE)DeviceID::Mtms)
            SetDeviceStateInfo((BYTE)DeviceID::Mtms, { val });
    }

    void DevStateFom::setManipulatorState(byte val)
    {
        cout << "[FOM Mapper] Set Manipulator State : " << static_cast<int>(val) << endl;
        latestManipulatorState = val;
        if (latestTreadmillType == (BYTE)DeviceID::Stms)
            SetDeviceStateInfo((BYTE)DeviceID::Pmms, { val });
        else if (latestTreadmillType == (BYTE)DeviceID::Mtms)
            SetDeviceStateInfo((BYTE)DeviceID::Amms, { val });
    }

    void DevStateFom::setHelmetPatchState(byte helmet)
    {
        cout << "[FOM Mapper] Set Helmet Patch State : " << static_cast<int>(helmet) << endl;
        latestPatchState[0] = helmet;
        std::vector<uint8_t> patchVector(latestPatchState, latestPatchState + 3);
        SetDeviceStateInfo((BYTE)DeviceID::Tits, patchVector);
    }

    void DevStateFom::setBodyPatchState(byte body)
    {
        cout << "[FOM Mapper] Set Body Patch State  : " << static_cast<int>(body) << endl;
        latestPatchState[1] = body;
        std::vector<uint8_t> patchVector(latestPatchState, latestPatchState + 3);
        SetDeviceStateInfo((BYTE)DeviceID::Tits, patchVector);
    }

    void DevStateFom::setHandPatchState(byte hand)
    {
        cout << "[FOM Mapper] Set Hand Patch State  : " << static_cast<int>(hand) << endl;
        latestPatchState[2] = hand;
        std::vector<uint8_t> patchVector(latestPatchState, latestPatchState + 3);
        SetDeviceStateInfo((BYTE)DeviceID::Tits, patchVector);
    }

    //#endif
}


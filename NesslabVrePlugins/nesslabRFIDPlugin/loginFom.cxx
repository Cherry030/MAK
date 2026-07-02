/*******************************************************************************
** Copyright (c) 2018 MAK Technologies, Inc.
** All rights reserved.
*******************************************************************************/
//#define KCLTEST 1
//#if DtHLA

#include "loginFom.h"

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
#include "extLogInInfo.h"
#include "extLogInInfoEncoder.h"
#include "extLogInInfoDecoder.h"

//Nesslab
#include "Nesslab/nesslabTcpServer.h"
#include "Nesslab/nesslabTcpClient.h"
#include "Nesslab/nesslabCommon.h"

#include <mutex>
#include <condition_variable>

using namespace nesslab_common;

//RFID -> VRIS
namespace rfid_to_vris_packets
{
#pragma pack(push, 1)
    struct TraineeInfoPacket
    {
        PacketHeader header;
        //std::uint8_t traineeID[16] = {};
        std::uint16_t seatID = 0;//셋트 아이디
        std::uint8_t serviceNumber[10] = {};
        std::uint8_t loginName[10] = {};
        std::uint8_t affiliation[10] = {};
        PacketTrailer trailer;

        TraineeInfoPacket()
        {
            header.deviceID = (int)DeviceID::Rfid;//0xca(IDD에 따로없어서 정함)
            header.msgID = 0xc1;//(IDD에 따로없어서 정함)
            header.length = sizeof(TraineeInfoPacket);
        }
    };
#pragma pack(pop)
}

//VRIS -> DSMS
namespace vris_to_dsms_packets
{
#pragma pack(push, 1)
    struct DsmsTraineeIdPacket
    {
        PacketHeader header;
        uint16_t	seatID = 0x02;//사용X
        BYTE		serviceNumber[16] = {};
        PacketTrailer trailer;

        DsmsTraineeIdPacket()
        {
            header.deviceID = (int)DeviceID::Vris;
            header.msgID = (uint8_t)VrisMsgID::TraineeIdMsgID;
            header.length = sizeof(DsmsTraineeIdPacket);
        }
    };
#pragma pack(pop)
}

namespace RFID
{
    bool isTraineeIdReceived = false;
    thread netThread;
    thread reconnectThread;

    namespace {

        //RFID -> VRIS, Port = 4000
        nesslabTcpServer* rfidTcpServer = nullptr;
        int					rfidPort = -1;
        std::vector<BYTE>	rfidRecvBuffer{};
        rfid_to_vris_packets::TraineeInfoPacket	rfidTraineePkt;


        //VRIS -> DSMS, Port = 3006
        nesslabTcpClient* dsmsTcpClient = nullptr;
        std::string			dsmsIP = "127.0.0.1";
        int					dsmsPort = -1;
        vris_to_dsms_packets::DsmsTraineeIdPacket dsmsTraineePkt;


        std::atomic_bool pluginStopping{ false };
        std::mutex dsmsMtx;
        std::condition_variable dsmsCv;

        PacketHandlers packetHandlers;
    }


    DtExerciseConn* m_pExCon;
    const DtString theFomClass = "AlternateEntity";

    void PrintLoginInfo(extLogInInfo inter)
    {
        SetConsoleOutputCP(949);
        std::cout << "\n[KCL] Send Login Info" << std::endl;
        std::cout << "[KCL] Publisher Type      " << inter.publisherType() << std::endl;
        std::cout << "[KCL] Device ID           " << inter.deviceID() << std::endl;
        std::cout << "[KCL] Service Number      " << GetString_DtNetLogInInfo10(inter.serviceNumber()) << std::endl;
        std::cout << "[KCL] Login Name          " << GetString_DtNetLogInInfo10(inter.logInName()) << std::endl;
        std::cout << "[KCL] Affiliation         " << GetString_DtNetLogInInfo10(inter.affiliation()) << std::endl;
    }

    std::string  GetString_DtNetLogInInfo10(DtNetLogInInfo10 info)
    {
        size_t len = strnlen(reinterpret_cast<const char*>(info.DtText), 10);
        std::string str(reinterpret_cast<const char*>(info.DtText), len);

        return str;
    }

    DtNetLogInInfo10  GetDtNetLogInInfo10_String(const std::string& str)
    {
        DtNetLogInInfo10 info{};

        size_t len = str.size();
        if (len > 10) len = 10;

        memcpy(info.DtText, str.data(), len);

        return info;
    }

    void loginFom(DtExerciseConn* exConn, bool runExtPropInit)
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
        /*
        rfidTcpClass = new TCPClass;
        rfidTcpClass->tcpPort = 4000;
        cout << "[INFO] RFID TCP 접속 시도중..." << rfidTcpClass->tcpPort << endl;
        rfidTcpClass->Start();
        if (!pDataProcessingThread)
        {
            pDataProcessingThread = new std::thread(&ProcessIncomingData);
        }
        */
    }

    void unloginFom(DtExerciseConn* exConn)
    {
        pluginStopping.store(true, std::memory_order_release);
        dsmsCv.notify_all();

        if (netThread.joinable())
            netThread.join();

        if (reconnectThread.joinable())
            reconnectThread.join();


        if (rfidTcpServer != nullptr)
        {
            rfidTcpServer->ServerStop();
            delete rfidTcpServer;
            rfidTcpServer = nullptr;
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

        DtDebug << "[KCL] End Login Fom\n";

        DtVrfEntityPublisher::setInitializer(DtExtendedAttributesInitializerPtr());
        DtReflectedExtEntityList::setInitializer(DtExtendedAttributesInitializerPtr());
    }

    void initPlugin()
    {

        pluginStopping.store(false, std::memory_order_release);
        rfidRecvBuffer.clear();
        packetHandlers.clear();

        //Get ConfigFile Data
        {

            char databuf[256];

            //RFID
            rfidPort = GetPrivateProfileInt("RFID", "rfidPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());

            //DSMS
            GetPrivateProfileString("DSMS", "dsmsIP", INI_STRING_NOT_FOUND_DEFAULT.c_str(), databuf, sizeof(databuf), CONFIG_FILE_PATH.c_str());
            dsmsIP = databuf;
            dsmsPort = GetPrivateProfileInt("RFID", "dsmsPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());

            //TraineeData
            dsmsTraineePkt.seatID = GetPrivateProfileInt("TraineeData", "seatID", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());

            if (rfidPort == INI_INT_NOT_FOUND_DEFAULT || dsmsPort == INI_INT_NOT_FOUND_DEFAULT || dsmsTraineePkt.seatID == INI_INT_NOT_FOUND_DEFAULT || dsmsIP == INI_STRING_NOT_FOUND_DEFAULT)
                cout << (LOG_WARN_CHANNEL_NAME) << "[RfidPlugin][Error] Failed to read data from INI file. Using default value." << endl;

            cout << "[RfidPlugin][Debug] rfidPort: " << rfidPort << ", dsmsIP: " << dsmsIP << ", dsmsPort: " << dsmsPort << ", seatID: " << dsmsTraineePkt.seatID << endl;
        }

        //Store packet data(데이터 받는 패킷만 처리 => 파싱, 콜백 처리용)
        {
            //RFID -> VRIS
            //Trainee ID Packet
            RegisterPacketHandler<rfid_to_vris_packets::TraineeInfoPacket>(packetHandlers, rfidTraineePkt.header.deviceID, rfidTraineePkt.header.msgID, rfidTraineePkt,
                [](const rfid_to_vris_packets::TraineeInfoPacket& packet)
                {
                    OnTraineeInfoPacket(packet);
                }
            );
        }



        //Create and start TCP/UDP
        {

            netThread = std::thread([]() {

                //RFID -> VRIS  
                rfidTcpServer = new nesslabTcpServer(rfidPort);
                rfidTcpServer->SetOnDataReceived(
                    [](const std::string& clientAddr, const uint8_t* data, int len)
                    {
                        OnDataReceived(clientAddr, data, len);
                    });
                rfidTcpServer->ServerStart();


                //VRIS -> DSMS
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
    }

    void OnTraineeInfoPacket(const rfid_to_vris_packets::TraineeInfoPacket& packet)
    {
        std::cout << "[RfidPlugin][Trace] onVrisTrainee() \n";

        //CP949 String
        std::string serviceNumberStr(reinterpret_cast<const char*>(packet.serviceNumber), sizeof(packet.serviceNumber));
        std::string loginNameStr(reinterpret_cast<const char*>(packet.loginName), sizeof(packet.loginName));
        std::string affiliationStr(reinterpret_cast<const char*>(packet.affiliation), sizeof(packet.affiliation));
        //std::cout << "traineeIDStr: " << traineeIDStr << "\n";(multibyte)


        //Unicode16 String
        //wide 출력 스트림(wcout)에 한국어 locale 적용
        std::wcout.imbue(std::locale("kor"));
        std::wstring serviceNumberUniStr = Cp949ToUnicode(serviceNumberStr);
        std::wcout << L"traineeIDStr: " << serviceNumberUniStr << std::endl;

        std::wcout.imbue(std::locale("kor"));
        std::wstring loginNameUniStr = Cp949ToUnicode(loginNameStr);
        std::wcout << L"loginNameStr: " << loginNameUniStr << std::endl;

        std::wcout.imbue(std::locale("kor"));
        std::wstring affiliationUniStr = Cp949ToUnicode(affiliationStr);
        std::wcout << L"affiliationStr: " << affiliationUniStr << std::endl;

        //Set ConfigFile Data
        {
            const std::wstring configFilePath = Cp949ToUnicode(CONFIG_FILE_PATH);
            BOOL result = WritePrivateProfileStringW(L"TraineeData", L"traineeID", serviceNumberUniStr.c_str(), configFilePath.c_str());

            if (!result)
                cout << "[RfidPlugin][Error] Failed to read data from INI file. Using default value." << endl;
        }

        isTraineeIdReceived = true;

        SendDataToVRF();
        SendDataToDsms();
    }

    //VRIS → VRF
    void SendDataToVRF()
    {
        //DtNetLogInInfo10 tmpServiceNumber{ DtText = {} }
        DtNetLogInInfo10 tmpServiceNumber{};
        std::memcpy(tmpServiceNumber.DtText, rfidTraineePkt.serviceNumber, sizeof(rfidTraineePkt.serviceNumber));
        DtNetLogInInfo10 tmpLoginName{};
        std::memcpy(tmpLoginName.DtText, rfidTraineePkt.loginName, sizeof(rfidTraineePkt.loginName));
        DtNetLogInInfo10 tmpAffiliation{};
        std::memcpy(tmpAffiliation.DtText, rfidTraineePkt.affiliation, sizeof(rfidTraineePkt.affiliation));

        SendLoginInfo(1, rfidTraineePkt.seatID, tmpServiceNumber, tmpLoginName, tmpAffiliation);
    }

    //VRIS → DSMS
    void SendDataToDsms()
    {
        if (!isTraineeIdReceived || dsmsTcpClient == nullptr) return;


        std::memcpy(&dsmsTraineePkt.serviceNumber, rfidTraineePkt.serviceNumber, sizeof(rfidTraineePkt.serviceNumber));


        const size_t size = sizeof(dsmsTraineePkt);
        unsigned char sendData[size];
        std::memcpy(sendData, &dsmsTraineePkt, size);


        bool result = dsmsTcpClient->SendData(sendData, size);
        if (result)
        {

            std::cout << "[RfidPlugin][Debug] Send data to Dsms : ";
            std::cout << std::hex << std::uppercase;
            for (int i = 0; i < size; ++i)
                std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(sendData[i]) << " ";
            std::cout << std::dec << std::endl;

        }
        else
            cout << "[RfidPlugin][Error] Failed to send data to the Dsms ";


    }

    void OnDsmsConnected()
    {
        std::cout << "[RFIDPlugin][Debug] DSMS Connected " << std::endl;
        SendDataToDsms();
    }

    void OnDsmsDisconnected()
    {
        std::cout << "[RFIDPlugin][Debug] DSMS Disconnected " << std::endl;
        if (pluginStopping.load(std::memory_order_acquire)) return;


        if (reconnectThread.joinable())
        {
            if (reconnectThread.get_id() == std::this_thread::get_id())
                reconnectThread.detach();
            else
                reconnectThread.join();
        }

        reconnectThread = thread(HandleDsmsReconnectThr);

    }

    void HandleDsmsReconnectThr()
    {
        if (pluginStopping.load(std::memory_order_acquire)) return;

        std::unique_lock<std::mutex> lock(dsmsMtx);

        bool status = dsmsCv.wait_for(lock, std::chrono::seconds(RECONNECT_DELAY_SEC), []() { return pluginStopping.load(std::memory_order_acquire); });
        if (status)
            return;

        if (dsmsTcpClient != nullptr)
            dsmsTcpClient->Connect();
    }

    void OnDataReceived(const std::string& clientAddr, const uint8_t* data, int len)
    {

        std::cout << "[RfidPlugin][Debug] Received Data from RFID, Data Length : " << len << ", Data :";
        std::cout << std::hex << std::uppercase;
        for (int i = 0; i < len; ++i)
            std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
        std::cout << std::dec << std::endl;


        rfidRecvBuffer.insert(rfidRecvBuffer.end(), data, data + len);

        ParseReceiveBuffer(packetHandlers, rfidRecvBuffer);
    }

    //CP949 -> UTF-16
    std::wstring Cp949ToUnicode(const std::string& cp949Str) {

        if (cp949Str.empty())
            return {};


        int requiredSize = MultiByteToWideChar(CP_ACP, 0, cp949Str.c_str(), -1, NULL, 0);

        if (requiredSize <= 0) return {};

        std::wstring unicodeStr(requiredSize - 1, L'\0'); // 널 제외
        MultiByteToWideChar(CP_ACP, 0, cp949Str.c_str(), -1, const_cast<wchar_t*>(unicodeStr.c_str()), requiredSize);


        return unicodeStr;
    }


    void SendLoginInfo(unsigned short usPublisherType, unsigned short usDeviceID, DtNetLogInInfo10 aryServiceNumber, DtNetLogInInfo10 aryLoginName, DtNetLogInInfo10 aryAffiliation)
    {
        extLogInInfo inter;

        inter.setPublisherType(DtNetU16(usPublisherType));
        inter.setDeviceID(DtNetU16(usDeviceID));

        /*
        DtNetLogInInfo10 serviceNumber;
        memcpy(serviceNumber.DtText, strServiceNumber.c_str(), 10);

        inter.setServiceNumber(GetDtNetLogInInfo10_String(strServiceNumber));
        inter.setLogInName(GetDtNetLogInInfo10_String(strLoginName));
        inter.setAffiliation(GetDtNetLogInInfo10_String(strAffiliation));
        */
        inter.setServiceNumber(aryServiceNumber);
        inter.setLogInName(aryLoginName);
        inter.setAffiliation(aryAffiliation);

        PrintLoginInfo(inter);

        m_pExCon->sendStamped(inter);
    }

    //#endif
}


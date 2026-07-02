#include "RFIDPlugin.h"


#include <mutex>
#include <condition_variable>

#include "Nesslab/nesslabTcpServer.h"
#include "Nesslab/nesslabTcpClient.h"
#include "Nesslab/nesslabCommon.h"
#include <iomanip>

using namespace makVre;
using namespace nesslab_common;


//RFID -> VRIS
namespace rfid_to_vris_packets
{

#pragma pack(push, 1)
	struct TraineeIdPacket
	{
		PacketHeader header;

		//Payload
		std::uint8_t traineeID[16] = {};

		PacketTrailer trailer;

		TraineeIdPacket()
		{
			header.deviceID = (int)DeviceID::Rfid;//0xca(IDD에 따로없어서 정함)
			header.msgID = 0xc1;//(IDD에 따로없어서 정함)
			header.length = sizeof(TraineeIdPacket);
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

		//Payload		
		uint16_t	seatID = 0x02;//사용X
		BYTE		traineeID[16] = {};

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



namespace nesslab_frontend_plugins
{

	namespace {

		//RFID -> VRIS, Port = 4000
		nesslabTcpServer* rfidTcpServer = nullptr;
		int					rfidPort = -1;
		std::vector<BYTE>	rfidRecvBuffer{};
		rfid_to_vris_packets::TraineeIdPacket	rfidTraineePkt;


		//VRIS -> DSMS, Port = 3006
		nesslabTcpClient* dsmsTcpClient = nullptr;
		std::string			dsmsIP = "127.0.0.1";
		int					dsmsPort = -1;
		vris_to_dsms_packets::DsmsTraineeIdPacket dsmsTraineePkt;


		std::atomic_bool pluginStopping{ false };
		std::mutex dsmsMtx;
		std::condition_variable rfidCv;

		PacketHandlers packetHandlers;
	}

	//엔진시작하면 실행
	RFIDPlugin::RFIDPlugin()
	{
		std::cout << "[RfidPlugin][Trace] Constructor " << std::endl;
	}



	RFIDPlugin::~RFIDPlugin()
	{
		std::cout << "[RfidPlugin][Trace] Destructor " << std::endl;
	}



	bool RFIDPlugin::init(makVre::DtVreInputManager& mgr) {

		std::cout << "[RfidPlugin][Trace] init() " << std::endl;


		InitPlugin();


		return true;
	}


	void RFIDPlugin::shutdown() {
		std::cout << "[RfidPlugin][Trace] shutdown " << std::endl;


		pluginStopping.store(true, std::memory_order_release);
		rfidCv.notify_all();

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

	}


	void RFIDPlugin::tick(double dt) {

	}




	void RFIDPlugin::InitPlugin()
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
				LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[RfidPlugin][Error] Failed to read data from INI file. Using default value." << std::endl;

			std::cout << "[RfidPlugin][Debug] rfidPort: " << rfidPort << ", dsmsIP: " << dsmsIP
				<< ", dsmsPort: " << dsmsPort << ", seatID: " << dsmsTraineePkt.seatID << std::endl;

		}



		//Store packet data(데이터 받는 패킷만 처리 => 파싱, 콜백 처리용)
		{
			//RFID -> VRIS
			//Trainee ID Packet
			RegisterPacketHandler<rfid_to_vris_packets::TraineeIdPacket>(packetHandlers, rfidTraineePkt.header.deviceID, rfidTraineePkt.header.msgID, rfidTraineePkt,
				[this](const rfid_to_vris_packets::TraineeIdPacket& packet)
				{
					OnTraineeIdPacket(packet);
				}
			);
		}



		//Create and start TCP/UDP
		{

			netThread = std::thread([this]() {

				//RFID -> VRIS  
				rfidTcpServer = new nesslabTcpServer(rfidPort);
				rfidTcpServer->SetOnDataReceived(
					[this](const std::string& clientAddr, const uint8_t* data, int len)
					{
						OnDataReceived(clientAddr, data, len);
					});
				rfidTcpServer->ServerStart();


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
	}






	//VRIS -> DSMS
	void RFIDPlugin::SendDataToDsms()
	{
		if (!isTraineeIdReceived || dsmsTcpClient == nullptr) return;


		std::memcpy(&dsmsTraineePkt.traineeID, rfidTraineePkt.traineeID, sizeof(rfidTraineePkt.traineeID));


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
			LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[RfidPlugin][Error] Failed to send data to the Dsms ";


	}


	// Tcp Server Connected callback
	void RFIDPlugin::OnDsmsConnected()
	{
		//Send TraineeID Packet
		SendDataToDsms();
	}





	void RFIDPlugin::HandleDsmsReconnectThr()
	{
		if (pluginStopping.load(std::memory_order_acquire)) return;

		std::unique_lock<std::mutex> lock(dsmsMtx);

		bool status = rfidCv.wait_for(lock, std::chrono::seconds(RECONNECT_DELAY_SEC), [this]() { return pluginStopping.load(std::memory_order_acquire); });
		if (status)
			return;

		if (dsmsTcpClient != nullptr)
			dsmsTcpClient->Connect();
	}



	//Dsms Disconnected Callback
	void RFIDPlugin::OnDsmsDisconnected()
	{
		if (pluginStopping.load(std::memory_order_acquire)) return;


		if (reconnectThread.joinable())
		{
			if (reconnectThread.get_id() == std::this_thread::get_id())
				reconnectThread.detach();
			else
				reconnectThread.join();
		}

		reconnectThread = std::thread(&RFIDPlugin::HandleDsmsReconnectThr, this);

	}



	void RFIDPlugin::OnDataReceived(const std::string& clientAddr, const uint8_t* data, int len)
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


	// Callback when a trainee ID packet is received from RFID.
	void RFIDPlugin::OnTraineeIdPacket(const rfid_to_vris_packets::TraineeIdPacket& packet)
	{
		std::cout << "[RfidPlugin][Trace] onVrisTrainee() \n";

		//CP949 String
		std::string traineeIDStr(reinterpret_cast<const char*>(packet.traineeID), sizeof(packet.traineeID));
		//std::cout << "traineeIDStr: " << traineeIDStr << "\n";(multibyte)


		//Unicode16 String
		//wide 출력 스트림(wcout)에 한국어 locale 적용
		std::wcout.imbue(std::locale("kor"));
		std::wstring traineeIDUniStr = Cp949ToUnicode(traineeIDStr);
		std::wcout << L"traineeIDStr: " << traineeIDUniStr << std::endl;


		//Set ConfigFile Data
		{
			const std::wstring configFilePath = Cp949ToUnicode(CONFIG_FILE_PATH);
			BOOL result = WritePrivateProfileStringW(L"TraineeData", L"traineeID", traineeIDUniStr.c_str(), configFilePath.c_str());

			if (!result)
				LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[RfidPlugin][Error] Failed to read data from INI file. Using default value.\n";
		}

		isTraineeIdReceived = true;

		SendDataToDsms();

	}

}
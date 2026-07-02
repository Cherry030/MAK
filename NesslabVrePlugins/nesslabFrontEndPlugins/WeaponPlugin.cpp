#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>

#include "utilities/vreMessageManager/vreMessageManager.h"
#include "framework/vreMessages/vrfSetResource.h"
#include "framework/vreMessages/vrfTaskEntityScript.h"
#include "vrePlayerStation/playerStation.h"

#include "WeaponPlugin.h"

using namespace makVre;
using namespace makVrv;
using namespace nesslab_common;



std::vector<WeaponInfo> gMyWeapons{};

//CustomHumanControlLogic.h
extern bool gStopRequested;


//Weapon --BLE--> SubVris --TCP--> Vrus
namespace weapon_to_vris_packets
{

#pragma pack(push,1)

	//격발신호(격발정보) 데이터
	struct FireInfoDataPacket
	{
		PacketHeader header;

		//Payload
		uint8_t ammoInMag	= 0;//잔탄량 => 0 ~ 100
		uint8_t fireCounter = 0;//격발량 => 0 ~ 100
		uint8_t magStatus	= 0;//탄창 상태 => 0=정상, 1=JAM(기능고장), 2=잔탄량 0, 3=재장전(임시)
		uint8_t battery		= 0;//배터리 => 0 ~ 100(5단위)

		PacketTrailer trailer;


		FireInfoDataPacket()
		{
			header.deviceID = (int)DeviceID::Hagh;//전자탄창, 제어기(TODO: VRIS로 수정)
			header.msgID	= (uint8_t)HaghMsgID::WeaponFireMsgID;//격발 정보 메시지(격발신호)
			header.msgType	= 0;//Reserved
			header.length	= sizeof(FireInfoDataPacket);
		}

	};

#pragma pack(pop)


}


//VRIS --Tcp-> DSMS
namespace vris_to_dsms_packets
{

#pragma pack(push, 1)
	struct weaponInfoDataPacket
	{
		PacketHeader header;


		//Payload
		uint8_t		weaponType	= 0;// 무기타입 => 0x00 소총, 0x01 권총, 0x02 투척무기, 0x03 기타
		uint8_t		isFire		= 0;// 발사여부 => 0x00 False, 0x01 True
		uint16_t	fireCounter = 0;// 격발 수
		int			totalAmmo	= 0;// 전체 남은 탄수
		uint8_t		ammoInMag	= 0;// 탄창에 남은 탄수
		uint8_t		magCount	= 0;// 남은 탄창 수
		uint8_t		hasMagSwap	= 0;// 탄창 교체 여부(0x00 미교체, 0x01 교체)

		PacketTrailer trailer;

		weaponInfoDataPacket()
		{
			header.deviceID = (int)DeviceID::Vris;
			header.msgID	= (uint8_t)HaghMsgID::WeaponFireMsgID;//총기정보 메시지
			header.msgType	= 0x00;//Reserved
			header.length	= sizeof(weaponInfoDataPacket);// 0x12(18)
		}
	};
#pragma pack(pop)
}



namespace nesslab_frontend_plugins {


	namespace
	{

		//Weapon --(BLE)-->SubVRIS--(TCP)-->VRIS, Port = 5015
		nesslabTcpServer*	weaponTcpServer	 = nullptr;
		int					weaponServerPort = -1;
		weapon_to_vris_packets::FireInfoDataPacket	fireInfoPkt;


		//VRIS --TCP--> DSMS, Port = 3103
		//nesslabTcpClient*	dsmsTcpClient = nullptr;
		nesslabTcpClient*	dsmsWeaponTcpClient = nullptr;
		std::string			dsmsWeaponIP = "127.0.0.1";
		int					dsmsWeaponPort = -1;


		vris_to_dsms_packets::weaponInfoDataPacket weaponInfoPkt;


		std::atomic_bool pluginStopping = false;
		std::mutex dsmsMtx;
		std::condition_variable dsmsCv;
		std::thread dsmsReconnectThr;


		std::vector<BYTE> weaponRecvBuffer{};

		PacketHandlers packetHandlers{};
	}



	//CTOR
	WeaponPlugin::WeaponPlugin()
		:DtPlayerComponent()
	{
		std::cout << "[WeaponPlugin][Trace] Constructor " << std::endl;
	}


	//DTOR
	WeaponPlugin::~WeaponPlugin()
	{
		std::cout << "[WeaponPlugin][Debug] Destructor " << std::endl;
	}



	bool WeaponPlugin::initialize(DtPlayerStation* player, DtInitTable& config)
	{

		//do base class init.  Shouldn't fail
		if (!DtPlayerComponent::initialize(player, config))
		{
			return false;
		}
		std::cout << "[WeaponPlugin][Debug] Initialize() " << std::endl;
	

		return true;
	}



	void WeaponPlugin::shutdown()
	{
		std::cout << "[WeaponPlugin][Trace] Entering function: shutdown() " << std::endl;

		//base class shutdown
		DtPlayerComponent::shutdown();

		//Remove Callback
		//myAttributeCallbacks -> 등록된 모든 콜백을 추적, disconnectAll -> 일괄정리
		myAttributeCallbacks.disconnectAll();


		pluginStopping.store(true, std::memory_order_relaxed);
		dsmsCv.notify_all();

		if (netThread.joinable())
			netThread.join();

		if (dsmsReconnectThr.joinable())
			dsmsReconnectThr.join();


		if (weaponTcpServer != nullptr)
		{
			weaponTcpServer->ServerStop();
			delete weaponTcpServer;
			weaponTcpServer = nullptr;
		}


		if (dsmsWeaponTcpClient != nullptr)
		{
			std::lock_guard<std::mutex> lock(dsmsMtx);
			
			dsmsWeaponTcpClient->Disconnect();
			delete dsmsWeaponTcpClient;
			dsmsWeaponTcpClient = nullptr;
		}

	}

	bool WeaponPlugin::postInitialize()
	{
		//기본 클래스의 postInitialize()를 먼저 호출 -> 컴포넌트 계층 구조 전체에서 초기화 순서가 올바르게 유지
		DtPlayerComponent::postInitialize();

		humanControLogic = myPlayer->findComponent<DtHumanControlLogic>();
		if (humanControLogic == nullptr)
		{
			LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[WeaponPlugin][Error] Could not find DtHumanControlLogic component." << std::endl;
			return false;
		}


		return true;
	}


	const char* WeaponPlugin::type() const
	{
		return weaponPluginType;
	}

	

	bool WeaponPlugin::InitPlugin()
	{
		std::cout << "[WeaponPlugin][Trace] Entering function: initPlugin() " << std::endl;


		pluginStopping.store(false, std::memory_order_release);

		weaponRecvBuffer.clear();
		packetHandlers.clear();
		gMyWeapons.clear();
				


		//WeaponInfo, weaponCallback
		{
			//WeaponInfo
			bool result = TryRegisterWeaponInfo();
			if (!result)
				return false;



			// Register callback for attribute
			result = TryRegisterWeaponCallbacks();
			if (!result)
			{
				//RemoveAllWeaponCallbacks();
				myAttributeCallbacks.disconnectAll();
				return false;
			}
		}



		//Get ConfigFile Data
		{
			char databuf[256] = {};


			//Weapon
			weaponServerPort = GetPrivateProfileInt("Weapon", "weaponPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());


			//DSMS
			GetPrivateProfileString("DSMS", "dsmsIP", INI_STRING_NOT_FOUND_DEFAULT.c_str(), databuf, sizeof(databuf), CONFIG_FILE_PATH.c_str());
			dsmsWeaponIP = databuf;
			dsmsWeaponPort = GetPrivateProfileInt("Weapon", "dsmsPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());


			if (weaponServerPort == INI_INT_NOT_FOUND_DEFAULT || dsmsWeaponPort == INI_INT_NOT_FOUND_DEFAULT || dsmsWeaponIP == INI_STRING_NOT_FOUND_DEFAULT)
				LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[WeaponPlugin][Error] Failed to read data from INI file. Using default value.\n";


			std::cout << "[WeaponPlugin][Debug] weaponPort: " << weaponServerPort << ", dsmsIP: " << dsmsWeaponIP << ", dsmsPort: " << dsmsWeaponPort << std::endl;
		}


		
		//Store packet data(데이터 받는 패킷만 처리 => 파싱, 콜백 처리용)
		{
			//Weapon -> VRIS
			//Fire Info  Packet
			RegisterPacketHandler<weapon_to_vris_packets::FireInfoDataPacket>(packetHandlers, fireInfoPkt.header.deviceID, fireInfoPkt.header.msgID, fireInfoPkt,
					[this](const weapon_to_vris_packets::FireInfoDataPacket& packet)
					{
						OnWeaponFire(packet);
					}
			);

		}


		//Create and start TCP/UDP
		{

			netThread = std::thread([this]() {

				//Weapon --Tcp--> VRIS
				weaponTcpServer = new nesslabTcpServer(weaponServerPort);
				weaponTcpServer->SetOnDataReceived(
					[this](const std::string& clientAddr, const uint8_t* data, int len)
					{
						OnDataReceived(clientAddr, data, len);
					}
				);
				weaponTcpServer->ServerStart();


				dsmsWeaponTcpClient = new nesslabTcpClient(dsmsWeaponPort, dsmsWeaponIP);
				dsmsWeaponTcpClient->SetOnConnectionChanged(
					[this](nesslabTcpClient::ConnectionState state)
					{
						if (state == nesslabTcpClient::ConnectionState::Disconnected)
							OnDsmsDisconnected();
					}
				);
				dsmsWeaponTcpClient->Connect();

			});




			/*
			//Weapon --Tcp--> VRIS
			weaponTcpServer = new nesslabTcpServer(weaponServerPort);
			weaponTcpServer->SetOnDataReceived(
				[this](const std::string& clientAddr, const uint8_t* data, int len)
				{
					OnDataReceived(clientAddr, data,len);
				}
			);
			weaponTcpServer->ServerStart();

			std::cout << "[WeaponPlugin][Debug] dsmsIP: " << dsmsWeaponIP << ", dsmsPort: " << dsmsWeaponPort << std::endl;

			dsmsWeaponTcpClient = new nesslabTcpClient(dsmsWeaponPort, dsmsWeaponIP);
			dsmsWeaponTcpClient->SetOnConnectionChanged(
				[this](nesslabTcpClient::ConnectionState state)
				{
					if (state == nesslabTcpClient::ConnectionState::Disconnected)
						OnDsmsDisconnected();
				}
			);
			dsmsWeaponTcpClient->Connect();
			*/


		}


		/*
		//currentWeaponIndex
		{
			
				//처음 시작 무기 인덱스(gCurrentWeaponIndex 변수)를 저장하기 위해서 OnWeaponIndexChanged 콜백 함수에서 Engage하면 인덱스값을 저장하는데
				//같은 캐릭터를 다시 Engage하면 OnWeaponIndexChanged() 콜백함수를 호출x
				//=> Console창에 WARN[HumanLogic] 02:04:40.379 makVre::DtHumanControlLogic::onWeaponIndexChanged: Weapon index is set out of range. 에러 출력
				//=> 해결방법: getAttribute<int>("weaponIndex")을 했을때 처음 Engage했을떄만 -1출력되니까 -1아닐때만 gCurrentWeaponIndex값 변경
			
			int currentWeaponIndex = playerAttributeStore()->getAttribute<int>("weaponIndex");
			if (currentWeaponIndex != -1)//처음 Engage X	
			{
				if (currentWeaponIndex >= 0 && currentWeaponIndex < gMyWeapons.size())
					gCurrentWeaponIndex = currentWeaponIndex;
			}
		}
		*/



		return true;
	}

	
	void WeaponPlugin::tick(double dt)
	{


		//tick에서 처리하는 이유 => initialize()에서 무기정보를 못가져옴(0출력)
		if (!pluginInitialized)
		{
			//플러그인 초기화 및 콜백 함수 추가
			pluginInitialized = InitPlugin();
		}




		/* Attributes 이름 출력
		std::cout << "\n";
		if (playerAttributeStore().assigned())
		{
			playerAttributeStore()->forEach([](makVre::DtAttributeHandle child) {
				LOG_INFO("Weapon") << child->name() << std::endl;
				// LOG_INFO("Weapon") << child->path() << std::endl;
				});
		}
		std::cout << "\n";
		*/


	}


	//Weapon fire Callback
	void WeaponPlugin::OnWeaponFire(const weapon_to_vris_packets::FireInfoDataPacket& packet)
	{
		std::cout << "[모의총기 수신] 격발신호 데이터 => 잔탄량 : " << (int)fireInfoPkt.ammoInMag << ", 격발량 : " << (int)fireInfoPkt.fireCounter << ", 탄창 상태 : " << (int)fireInfoPkt.magStatus
			<< "배터리 : " << (int)fireInfoPkt.battery << std::endl;

		
		//※ 탄창 상태가 정상(0)이 아니면 리턴
		if (fireInfoPkt.magStatus != 0)
			return;


		HandleWeaponFire();
	}



	//※격발 성공하면 총알 변경 콜백함수에서 dsms로 데이터 전송(격발 성공여부는 전역변수로 저장)
	void WeaponPlugin::HandleWeaponFire()
	{
		std::cout << "[WeaponPlugin][Trace] handleWeaponFire() \n";


		//트레드밀에서 정지신호받았으면 리턴처리
		if (gStopRequested)
			return;



		weaponFired = false;
		
		int weaponState = player().playerAttributeStore()->getAttribute<int>("weaponState");

		//탄창에 남은 총알개수
		int ammoInMag = 0;

		//전체 남은 총알 개수
		int totalRemainingAmmo = GetTotalRemainingAmmo(currentWeaponIndex);


		//총 집어넣은상태 => weaponStow(True)
		if (weaponState != 3)
		{
			humanControLogic->weaponStow(false);
			humanControLogic->weaponUp();
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}



		WeaponType weaponType = gMyWeapons[currentWeaponIndex].weaponType;

		if (weaponType == WeaponType::RIFLE || weaponType == WeaponType::PISTOL)
		{

			ammoInMag = GetCurrentAmmoInMag(currentWeaponIndex);

			if (ammoInMag > 0)
			{
				//Fire
				humanControLogic->fire(1);
				weaponFired = true;
			}
			else
			{
				std::cout << ((totalRemainingAmmo <= 0) ? "[WeaponPlugin][Debug] No bullets..." : "[WeaponPlugin][Debug] Reload the gun...") << std::endl;
				weaponFired = false;
			}

		}
		else if (weaponType == WeaponType::GRENADE || weaponType == WeaponType::LAUNCHER)
		{

			ammoInMag = totalRemainingAmmo;


			if (ammoInMag > 0)
			{
				//Fire
				humanControLogic->fire(1);
				weaponFired = true;
			}
			else
			{
				std::cout << "[WeaponPlugin][Debug] No bullets..." << std::endl;
				weaponFired = false;

			}

		}

		/*
		//총알없을때도 모니터링쪽으로 데이터를 보내야하면 추가
		if (!isWeaponFire)
		{
			std::cout << "[WeaponPlugin][Error] Bullet firing failed " << std::endl;
			BYTE remainingMag = (totalAmmoRemaining - currentBullets) / maxAmmoCapacity;//남은 탄창 수
			sendDataToDsms();
		}
		*/

	}






	// Weapon Index change callback
	void WeaponPlugin::OnWeaponIndexChanged(const int& index)
	{
		//같은 캐릭터 Engage 다시했을 경우 tick보다 빨리 처리되서 size = 0 처리되서 에러남
		if (!pluginInitialized) return;

		std::cout << "[WeaponPlugin][Trace] Entering function: OnWeaponIndexChanged() index: "<< index << std::endl;

		WeaponInfo weapon = gMyWeapons[index];

		currentWeaponIndex = index;

		//발사 여부
		weaponFired = false;


		//무기 변경시 현재 무기의 탄창수 저장(재장전 성공 여부 확인용)
		if (weapon.weaponType == WeaponType::RIFLE || weapon.weaponType == WeaponType::PISTOL)
		{

			int ammoInMag = GetCurrentAmmoInMag(index);
			int maxAmmoInMag = GetMaxAmmoInMag(index);
			int totalRemainingAmmo = GetTotalRemainingAmmo(index);

			//남은 탄창 수
			currentMagCount = (totalRemainingAmmo - ammoInMag) / maxAmmoInMag;


			//sendDataToDsms함수에서 처리하면 발사 트리거 발생하면 딜레이있음
			fireInfoPkt.ammoInMag = GetCurrentAmmoInMag(index);
			fireInfoPkt.fireCounter = maxAmmoInMag - ammoInMag;


		}
		else if (weapon.weaponType == WeaponType::GRENADE || weapon.weaponType == WeaponType::LAUNCHER)
		{
			currentMagCount = 0;

			weaponInfoPkt.ammoInMag = weaponInfoPkt.totalAmmo;

			int ammoInMag = GetTotalRemainingAmmo(index);
			//int totalRemainingAmmo = getTotalRemainingAmmo(index);

			fireInfoPkt.ammoInMag = ammoInMag;


		}

		SendDataToDsms();
	}


	
	//onTotalRemainingAmmo() 실행되고 OnBulletCountChanged()실행
	//※※onBulletCountChanged에서 데이터 처리하면 전체 총알수가 한프레임 늦게 처리되서 적용이안됨
	void WeaponPlugin::OnTotalRemainingAmmo(const double& totalAmmo)
	{
		std::cout << "[WeaponPlugin][Trace] Entering function: onTotalRemainingAmmo() totalAmmo:" << totalAmmo << std::endl;

		SendDataToDsms();
	}


	//탄창에 남은 총알 개수
	int WeaponPlugin::GetCurrentAmmoInMag(int weaponIndex)
	{
		WeaponInfo weapon = gMyWeapons[weaponIndex];
		if (weapon.weaponType == WeaponType::GRENADE || weapon.weaponType == WeaponType::LAUNCHER)
			return 0;

		//ex) clip:weapon, clip:weapon2 ...
		std::pair<int, int> bullets = playerAttributeStore()->getAttribute< std::pair<int, int>>(gMyWeapons[weaponIndex].attributeName);

		return bullets.first;
	}



	//탄창에 보관할 수 있는 최대 총알 수
	int WeaponPlugin::GetMaxAmmoInMag(int weaponIndex)
	{
		//투척류는 탄창개념이아니라 에러남
		WeaponInfo weapon = gMyWeapons[weaponIndex];		
		if (weapon.weaponType == WeaponType::GRENADE || weapon.weaponType == WeaponType::LAUNCHER)
			return 0;



		//ex) clip:weapon, clip:weapon2 ...
		std::pair<int, int> bullets = playerAttributeStore()->getAttribute< std::pair<int, int>>(weapon.attributeName);
		return bullets.second;

	}


	//전체 남은 총알 개수
	int WeaponPlugin::GetTotalRemainingAmmo(int weaponIndex)
	{

		std::string attributeName = gMyWeapons[weaponIndex].munitionType;
		makVre::DtAttributeHandle grenadeCountHandle = playerAttributeStore()["resources"][attributeName]["currentAmount"];
		if (grenadeCountHandle.assigned())
		{
			const double& value = grenadeCountHandle->get<double>();
			return value;
		}

		return 0;
	}



	void WeaponPlugin::SendDataToDsms()
	{
		if (dsmsWeaponTcpClient == nullptr)
			return;


		int weaponType = (int)gMyWeapons[currentWeaponIndex].weaponType;


		weaponInfoPkt.weaponType = weaponType;
		weaponInfoPkt.isFire = weaponFired;
		weaponInfoPkt.totalAmmo = GetTotalRemainingAmmo(currentWeaponIndex);


		//남은 탄창 수 => (전체 남은 탄수 - 탄창에 남은 탄수)/탄창에 보관가능한 최대 총알 개수
		if (weaponType == (int)WeaponType::GRENADE || weaponType == (int)WeaponType::LAUNCHER)
		{
			weaponInfoPkt.ammoInMag = weaponInfoPkt.totalAmmo;
			weaponInfoPkt.magCount = 0;

			//코드리뷰중 1로 하기로 정함
			weaponInfoPkt.fireCounter = 1;

		}
		else
		{
			//weaponInfoPkt.ammoInMag = getCurrentAmmoInMag(weaponIndex);//VRE 캐릭터 탄창에 남은 탄수 => 딜레이확인필요
			weaponInfoPkt.ammoInMag = fireInfoPkt.ammoInMag;//전자탄창에 남은 탄수
			weaponInfoPkt.magCount = (weaponInfoPkt.totalAmmo - weaponInfoPkt.ammoInMag) / GetMaxAmmoInMag(currentWeaponIndex);

			weaponInfoPkt.fireCounter = fireInfoPkt.fireCounter;//격발수

		}


		//탄창 교체 여부 => 기존 탄창 개수랑 다르면 재장전 성공했다고 처리
		if (currentMagCount != weaponInfoPkt.magCount)
		{
			weaponInfoPkt.hasMagSwap = true;
			currentMagCount = weaponInfoPkt.magCount;

		}
		else
			weaponInfoPkt.hasMagSwap = false;


		weaponFired = false;


		//Struct -> byte*
		const int size = sizeof(weaponInfoPkt);
		unsigned char byteArray[size];
		std::memcpy(&byteArray, &weaponInfoPkt, size);



		bool result = dsmsWeaponTcpClient->SendData(byteArray, size);

		if (result)
		{
			std::cout << "[WeaponPlugin][Info] Send data to DSMS : ";
			std::cout << std::hex << std::uppercase;
			for (int i = 0; i < size; ++i)
				std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(byteArray[i]) << " ";
			std::cout << std::dec << std::endl;

		}
		else
			LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[WeaponPlugin][Error] Failed to send data to the DSMS " << std::endl;

	}



	void WeaponPlugin::OnDataReceived(const std::string& clientAddr, const uint8_t* data, int len)
	{

		//std::cout << "[WeaponPlugin][DEBUG] Received Data from Weapon";
		//std::cout << "Data Length : " << len << ", Data : ";
		//std::cout << std::hex << std::uppercase;
		//for (int i = 0; i < len; ++i)
		//	std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
		//std::cout << std::dec << std::endl;

		weaponRecvBuffer.insert(weaponRecvBuffer.end(), data, data + len);
		ParseReceiveBuffer(packetHandlers, weaponRecvBuffer);

	}



	void WeaponPlugin::HandleDsmsDisconnected()
	{
		if (pluginStopping.load(std::memory_order_acquire)) return;

		std::unique_lock<std::mutex> lock(dsmsMtx);

		bool status = dsmsCv.wait_for(lock, std::chrono::seconds(RECONNECT_DELAY_SEC), [&]() { return pluginStopping.load(std::memory_order_acquire); });

		//!status => timeout
		if (status)
			return;

		if (dsmsWeaponTcpClient != nullptr)
			dsmsWeaponTcpClient->Connect();

	}


	

	//VRIS<->DSMS TCP Client Disconnected Callback
	void WeaponPlugin::OnDsmsDisconnected()
	{
		if (pluginStopping.load(std::memory_order_acquire)) return;

		if (dsmsReconnectThr.joinable())
		{
			if (dsmsReconnectThr.get_id() == std::this_thread::get_id())
				dsmsReconnectThr.detach();
			else
				dsmsReconnectThr.join();
		}

		dsmsReconnectThr = std::thread(&WeaponPlugin::HandleDsmsDisconnected, this);
	}




	bool WeaponPlugin::TryRegisterWeaponInfo()
	{
		std::cout << "[WeaponPlugin][Trace] TryRegisterWeaponInfo() \n";


		//훈련자 무기 이름 및 탄 타입
		std::vector<std::string> munitionTypeList = playerAttributeStore()->getAttribute<std::vector<std::string>>("weaponList");//ex) 2 8 120 2 3 0 1, 2 8 225 2 3 0 0, 2 8 225 2 1 1 0
		std::vector<std::string> weaponNameList = playerAttributeStore()->getAttribute<std::vector<std::string>>("weapons");   //ex) M4,M9,FragGrenade,SmokeGrenade, FlashBang
		if (weaponNameList.size() == 0 || munitionTypeList.size() == 0 || weaponNameList.size() != munitionTypeList.size())
		{
			return false;
		}

		int weaponIndex = 0;
		for (int i = 0; i < weaponNameList.size(); ++i)
		{
			WeaponInfo weapon;
			weapon.weaponIndex = weaponIndex;
			weapon.weaponName = weaponNameList[i];
			weapon.munitionType = munitionTypeList[i];


			//※ObjectType -> Kind:Domain:Country:Category:Subcategory:Specific:Extra (VR-Forces 5.2d Simulation Object Editor -> Munitions에서 확인가능)
			//※Subcategory 85- Grenade, Hand  -> 수류탄
			std::stringstream entityTypeStream(weapon.munitionType);
			std::string subCategoryStr;
			const int entitySubCategoryIndex = 4;
			for (int i = 0; i <= entitySubCategoryIndex; ++i)
			{
				std::getline(entityTypeStream, subCategoryStr, ':');
			}


			int subCategory = 0;
			try
			{
				subCategory = std::stoi(subCategoryStr);
			}
			catch (const std::invalid_argument& e)
			{
				std::cout << "[WeaponPlugin][Erorr] invalid_argument: " << e.what() << std::endl;
				return false;
			}

			//weaponType
			{
				//Subcategory 85- Grenade, Hand -> 나머지는 총기류 처리
				weapon.weaponType = (subCategory == 85) ? WeaponType::GRENADE : WeaponType::RIFLE;

				//권총, 소총 구분은 API로는 불가능(VRE2.1.1b기준), 이름으로 구분
				if (weapon.weaponType == WeaponType::RIFLE)
				{
					if (weapon.weaponName.compare("M9") == 0 || weapon.weaponName.compare("K5") == 0)
						weapon.weaponType = WeaponType::PISTOL;

				}
			}


			//attributeName
			{
				//Rfile, PISTOL
				if (weapon.weaponType == WeaponType::RIFLE || weapon.weaponType == WeaponType::PISTOL)
				{
					//ex) clip:2:8:225:2:1:1:0
					weapon.attributeName = "clip:" + weapon.munitionType;
				}
				//GRENADE, LAUNCHER
				else if (weapon.weaponType == WeaponType::GRENADE || weapon.weaponType == WeaponType::LAUNCHER)
				{
					//ex) resource:other-4|M18-green
					weapon.attributeName = weapon.munitionType;
				}
			}

			weaponIndex++;


			gMyWeapons.push_back(weapon);
		}

		
		for (WeaponInfo& weapon : gMyWeapons)
		{
			std::cout << "[WeaponPlugin][Info] WeaponIndex: " << weapon.weaponIndex << ", Name: " << weapon.weaponName
				<< ", WeaponAttributeName: " << weapon.attributeName << ",amMunitionType: " << weapon.munitionType
				<< ", myWeaponType: " << (int)weapon.weaponType
				<< std::endl;
		}


		return true;
	}



	bool WeaponPlugin::TryRegisterWeaponCallbacks()
	{
		std::cout << "[WeaponPlugin][Trace] TryRegisterWeaponCallbacks() \n";


		//Weapon Index Change
		{
			// Register callback for weaponIndex attribute changes
			if (playerAttributeStore()->hasAttribute("weaponIndex"))
			{
				makVre::DtAttributeHandle weaponIndexAttributeHandle = playerAttributeStore()["weaponIndex"];
				myAttributeCallbacks.connect(weaponIndexAttributeHandle, this, &WeaponPlugin::OnWeaponIndexChanged);
			}
			else
			{
				//Failed to find the state attribute, log a warning
				LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[WeaponPlugin][Error] Failed to find state attribute  => weaponIndex" << std::endl;
				return false;
			}
		}


		
		//Add callback for weapon ammo count change
		{
			
			//※에러나면 VRE 인게이지에서 shift + F4 -> clip:, resource: 확인
			for (const WeaponInfo& weapon : gMyWeapons)
			{
				std::cout << "[WeaponPlugin][Debug] weapons Name => " << weapon.weaponName << std::endl;


				//전체 남은 탄수(총기+수류탄)
				std::string attributeName = weapon.munitionType;//ex)2:8:225:2:1:1:2(각 의미는 Simulation Object Editor에서확인가능)
				makVre::DtAttributeHandle handle = playerAttributeStore()["resources"][attributeName]["currentAmount"];
				if (handle.assigned())
				{
					myAttributeCallbacks.connect(handle, this, &WeaponPlugin::OnTotalRemainingAmmo);
				}
				else
				{
					LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[WeaponPlugin][Error]  Failed to find state attributeName => " << weapon.attributeName << std::endl;
					return false;
				}

			}
		}


		return true;
	}



	//Check SkeletonCharacter
	#include "vlpi/entityType.h"
	bool IsSkeletalCharacter(DtPlayerStation* myPlayer)
	{

		DtEntityType skeletonEntityType("3:1:225:1:41:1:3");
		bool isSkeletonCh = myPlayer->entityType().matchPattern(skeletonEntityType) ? true : false;

		return isSkeletonCh;
	}



}
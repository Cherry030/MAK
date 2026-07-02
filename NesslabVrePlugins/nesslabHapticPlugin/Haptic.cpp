#include <vreUtil/assert.h>
#include <vreUtil/logger.h>

#include <vrfobjcore/detonationContext.h>
#include <vrfobjcore/humanStateRepository.h>
#include <vrfobjcore/attachedTerrain.h>
#include <vrfutil/kinematicTools.h>

#include "Haptic.h"
#include "Nesslab/nesslabTcpServer.h"
#include "Nesslab//nesslabTcpClient.h"
#include "Nesslab/nesslabCommon.h"
#include "Nesslab/StopAllPluginsMessage.h"



//※※Mak 피격 부위(6부위) => 상체 앞, 상체 왼쪽, 상체 오른쪽, 상체 뒤, 머리, 다리 ※※

using namespace nesslab_common;
using namespace makVrf;
using namespace makVre;



namespace nesslab_backend_plugins {

//VRIS --TCP--> Haptic(HSTH)
#pragma pack(push, 1)
	struct VrisToHapticPacket
	{
		PacketHeader header;

		//Payload
		BYTE	hitBodyZone = 0;//피격 부위 => 0x01 상체 앞, 0x02 상체 왼쪽, 0x03 상체 오른쪽, 0x04 상체 뒤, 0x05 머리, 0x06 다리, 0x07 전체피해
		BYTE	damageType  = 0;//피해 타입 => 0x00 = 물리 타입, 0x01 = 총상, 0x02 = 자상(칼,도검류), 0x03 = 화학피해(화상,케미컬)
		BYTE	damage      = 0;//피격 데미지 0~100
		BYTE	damageState = 0;//현재 캐릭터 Health상태 값 => 0x00 = 정상, 0x01 = 경상, 0x02 = 중상, 0x03 = 사망
		BYTE	reserved    = 0;

		PacketTrailer trailer;

		VrisToHapticPacket()
		{		
			header.deviceID = (int)DeviceID::Vris;//발신 디바이스 ID
			header.msgID	= (uint8_t)VrisMsgID::DamageEffectControlMsgID;
			header.msgType	= 0;//Reserved
			header.length	= sizeof(VrisToHapticPacket);//0x0c(12)
		}

	};
#pragma pack(pop)



	//VRIS --TCP--> DSMS
#pragma pack(push, 1)
	struct VrisToDsmsPacket
	{
		PacketHeader header;

		//Payload
		BYTE	hitBodyZone = 0;//피격 부위, 0x00 피해없음, 0x01 상체 앞, 0x02 상체 왼쪽, 0x03 상체 오른쪽, 0x04 상체 뒤, 0x05 머리, 0x06 다리, 0x07 전체피해
		BYTE	attakerID[16]{};	//공격한 훈련자 ID
		BYTE	damage = 0;//데미지 1-100
		int		health = 0;//1-100

		PacketTrailer trailer;

		VrisToDsmsPacket()
		{
			header.deviceID = (int)DeviceID::Vris;
			header.msgID	= (uint8_t)VrisMsgID::HitInfoMsgID;
			header.msgType	= 0;   ///Reserved
			header.length	= sizeof(VrisToDsmsPacket);//0x0c(12)
		}
	};
#pragma pack(pop)


	namespace {

		//Haptic <--BLE--> Sub_VRIS <--TCP-- VRIS, Port = 8300
		nesslabTcpServer*	hapticTcpServer = nullptr;
		int					hapticPort = -1;
		VrisToHapticPacket	vrisToHapticPkt;


		//Mak --TCP--> DSMS, Port = 3004
		nesslabTcpClient*	dsmsTcpClient = nullptr;
		std::string			dsmsIP = "127.0.0.1";
		int					dsmsPort = -1;
		VrisToDsmsPacket	vrisToDsmsPkt;


		std::mutex dsmsMtx;
		std::condition_variable dsmsCV;
		std::thread dsmsReconnectThr;
		std::atomic_bool pluginStopping = false;

	}


	//Constructor => Enage Human 캐릭터 생성하면 실행
	HapticPlugin::HapticPlugin(const DtString& name,
		DtLocalObject* owner,
		DtSimulationServices* simManager,
		DtComponentDescriptor* desc,
		DtReaderWriterRegistry* parentRegistry)
		: DtVreHumanDamageActuator(name, owner, simManager, desc, parentRegistry)
	{
		std::cout << "[HapticPlugin][Trace] Constructor \n";

		InitPlugin();


		DtVreMessageManager::instance().addHandler(StopAllPluginsMessage::theType(),
			DtVreMessageDelegate(this, &HapticPlugin::HandleStopPluginMessage));


	}



	// destructor => Human 아닌 오브젝트를 생성하거나 다른 인간 캐릭터 생성하면 실행
	HapticPlugin::~HapticPlugin()
	{
		std::cout << "[HapticPlugin][Trace] Destructor " << entity()->uuid().string() << std::endl;


		pluginStopping.store(true, std::memory_order_release);
		dsmsCV.notify_all();

		if (netThread.joinable())
			netThread.join();

		if (dsmsReconnectThr.joinable())
			dsmsReconnectThr.join();

		if (hapticTcpServer != nullptr)
		{
			hapticTcpServer->ServerStop();
			delete hapticTcpServer;
			hapticTcpServer = nullptr;
		}


		if (dsmsTcpClient != nullptr)
		{
			std::lock_guard<std::mutex> lock(dsmsMtx);

			//dsmsTcpClient->ClientStop();
			dsmsTcpClient->Disconnect();
			delete dsmsTcpClient;
			dsmsTcpClient = nullptr;
		}



		DtVreMessageManager::instance().removeHandler(StopAllPluginsMessage::theType(),
			DtVreMessageDelegate(this, &HapticPlugin::HandleStopPluginMessage));


	}



	// This returns a string from compTypes.h, and identifies the type of compone
	const char* HapticPlugin::type() const
	{

		return makVre::DtVreHumanDamageActuatorType;
	}


	void HapticPlugin::InitPlugin()
	{
		pluginStopping.store(false, std::memory_order_release);


		//Get ConfigFile Data
		{
			char databuf[256] = {};

			//haptic
			hapticPort = GetPrivateProfileInt("Haptic", "hapticPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());

			//Dsms
			GetPrivateProfileString("DSMS", "dsmsIP", INI_STRING_NOT_FOUND_DEFAULT.c_str(), databuf, sizeof(databuf), CONFIG_FILE_PATH.c_str());
			dsmsIP = databuf;
			dsmsPort = GetPrivateProfileInt("Haptic", "dsmsPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());


			if (hapticPort == INI_INT_NOT_FOUND_DEFAULT || dsmsPort == INI_INT_NOT_FOUND_DEFAULT || dsmsIP == INI_STRING_NOT_FOUND_DEFAULT)
				LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[HapticPlugin][Error] Failed to read data from INI file. Using default value.\n";


			std::cout << "[HapticPlugin][Debug] hapticPort: " << hapticPort << ", dsmsIP: " << dsmsIP << ", dsmsPort: " << dsmsPort << std::endl;
		}



		//Create and start TCP/UDP
		{

			netThread = std::thread([this]() {

				//Haptic <--BLE--> Sub_VRIS <--TCP-- VRIS
				hapticTcpServer = new nesslabTcpServer(hapticPort);
				hapticTcpServer->ServerStart();

				//Mak --TCP--> DSMS
				dsmsTcpClient = new nesslabTcpClient(dsmsPort, dsmsIP);
				dsmsTcpClient->SetOnConnectionChanged(
					[this](nesslabTcpClient::ConnectionState connectionState)
					{
						if (connectionState == nesslabTcpClient::ConnectionState::Disconnected)
							OnDsmsDisconnected();
					}
				);
				dsmsTcpClient->Connect();

			});
		}

	}

	
	//PS-9652
	bool HapticPlugin::adjudicateCollateralDamage(const DtDetonationContext& detonationContext)
	{
		//플러그인 자체가 Enage Human 캐릭터 생성하면 실행
		//if (!isPlayerControlled("Human")) return false;
	

		DtRwInt healthBeforeHit = entity()->stateProperties().findProperty<DtRwInt>("OverallHealth")->value();
		//std::cout << "[HapticPlugin][Debug] BeforeHit  health: " << healthBeforeHit << std::endl;

		//실제 데미지 처리 함수
		bool retVal = DtVreHumanDamageActuator::adjudicateCollateralDamage(detonationContext);

		DtRwInt healthAfterHit = entity()->nextFrameStateProperties().findProperty<DtRwInt>("OverallHealth")->value();
		//std::cout << "[HapticPlugin][Debug] AfterHit health: " << healthAfterHit << std::endl;

		int hitDamage = healthBeforeHit - healthAfterHit;
		if (hitDamage == 0)
			return retVal;

		//피해타입
		BYTE damageType = 0x00;

		//피해부위
		std::string makBodyZoneName = "";

		//피해 상태
		BYTE damageState = 0x00;



		// Get the surface and impact angle of the hit on the entity's bounding box.
		// Surface name is one of the following:  "front", "left-side", "right-side", "top", "bottom", "rear"
		// Create a working copy of the incoming detonation context, because it will modified
		// and in this call.
		DtDetonationContext workingDetonationContext = detonationContext;
		getDirectFireSurfaceAndAngleOfIncidence(workingDetonationContext);



		if (detonationContext.detonationInter().munitionType().domain() == 8)
		{			
			//무기타입 SISO => domain 8 대인(Anti-Personnel) //firearm types


			//1. 직접 피해 + 무기타입 domain 8(Anti-Personnel)
			DtHumanStateRepository* myHumanStateRepository = dynamic_cast<DtHumanStateRepository*>(entity()->internalState());
			const Coordinate_System* coordinateSystemPtr = terrainAttachedTo()->coordinateSystem();
			const DtVector& entityLocalLocation = entity()->localPosition();
			DtVector detonationEntityLocation = detonationContext.detonationInter().entityLocation();


			DtDamageState explosiveDamageState = myHumanLocalObjectFacade.nextFrameDamageState();

			// Get the damage data for the body zone that was hit.  
			DtDamageZoneInfo damageZoneInfo;
			if (getClosestDamageZone(workingDetonationContext, damageZoneInfo))
			{
				std::cout << "[HapticPlugin][Debug] Damage Type: Gun \n";

				//피격 부위
				makBodyZoneName = damageZoneInfo.bodyZoneName;

			}

		}
		else 
		{
			//Explosive weapon type (e.g., Frag Grenade, RPG, TANK (M829A1-AP-120mm), etc.)

			std::cout << "[HapticPlugin][Debug] Damage Type: Explosive \n";

			DtDamageState explosiveDamageState = myHumanLocalObjectFacade.nextFrameDamageState();

			//피격 부위(전체)
			makBodyZoneName = "All";

		}


		////공격한 객체 이름 ex) R1, R2, R3, ....
		//string attackerName = workingDetonationContext.detonationInter().attackerId().markingText().string();
		//cout << "AttackerName : " << attackerName << endl;


		//임시 공격자 ID
		BYTE attackerID[16];
		memset(attackerID, 1, sizeof(attackerID));


		if (healthAfterHit <= 0)
		{
			std::cout << "[HapticPlugin][Debug] Character has died." << std::endl;
			
			//TODO: 사망 이벤트 추가 => ex)햅틱 전체 진동 및 진동세기 강하게

		}


		//피격 부위 
		BYTE hapticHitZone = FormatHitZone(makBodyZoneName);
		if (hapticHitZone == 0)//피해없음
			return retVal;
			

		//포맷형식에 맞춰서 임시값 0x01(총상 + 폭발)
		damageType = 0x01;

		
		//피해 상태 값	
		damageState = FormatDamageState(healthAfterHit);


		//VRIS -> Haptic
		SendDataToHaptic(hapticHitZone, damageType, hitDamage, damageState);


		//VRIS -> DSMS
		SendDataToDsms(hapticHitZone, attackerID, hitDamage, healthAfterHit);


		

		std::cout << "[HapticPlugin][Debug] DamageType : " << (int)damageType << ", BodyZoneName : " << makBodyZoneName
			<< ", DamageState : " << (int)damageState << ", Damage : " << hitDamage << ", healthAfterHit : " << healthAfterHit << std::endl;

		

		return retVal;


	}


	/*
	
	직사(Direct Fire)로 피격됐을 때, 그 피격/폭발 컨텍스트를 기준으로 가장 가까운(가장 영향이 큰) Damage Zone을 찾아서 정보를 저장
	detonationContext : 피격/탄착/폭발 관련 정보를 담은 컨텍스트
	damageZone : 함수가 계산한 “가장 가까운 피해 존”의 정보를 여기에 채워 넣음
	반환값 bool : true면 유효한 damage zone을 찾았고 damageZone이 채워짐, false면 해당 컨텍스트에서 적용할 damage zone을 못 찾음
	
	*/
	bool HapticPlugin::getClosestDamageZone(const DtDetonationContext& detonationContext,
		DtDamageZoneInfo& damageZone)
	{
		DtHumanStateRepository* myHumanStateRepository = dynamic_cast<DtHumanStateRepository*>(entity()->internalState());

		// Location of hit in body coordinates of the entity.
		DtVector detonationEntityLocation = detonationContext.detonationInter().entityLocation();

		// Name of the surface on the entity's bounding volume that was hit.
		// One of the following:  "front", "left-side", "right-side", "top", "bottom", "rear".
		std::string surfaceName = std::string(detonationContext.surface().c_str());

		// Get the posture
		DtLifeformState posture = myHumanStateRepository->assignedPosture();
		

		// Look up the damage map for this posture
		DtDamageMap damageMap = myPostureDamageMap[posture];

		// The distance to current closest damage zone to the detonation.
		// The value is distance squared.
		// Initialize to max to indicate it has not been assigned yet.
		double currentClosestDistanceSquared = std::numeric_limits<double>::max();

		DtDamageZoneInfo* currentClosestDamageZoneInfo = nullptr;

		// Iterate through the different damage zones to find which zone was hit.
		for (auto& damageMapIter : damageMap)
		{
			DtDamageZoneInfo damageZoneInfo = damageMapIter.second;

			// determine the vector from the detonation location to the body zone center.
			DtVector distanceToZoneCenter = (damageZoneInfo.bodyZoneCenter - detonationEntityLocation);

			// For distance calculations, use the magnitude squared of the vector.
			double distZoneCenterSquared = distanceToZoneCenter.magnitudeSquared();

			if (distZoneCenterSquared <= currentClosestDistanceSquared)
			{
				// If this zone is independent of surface data, or it does use surface data and the surface is a match... 
				if (!damageZoneInfo.useSurfaceData || (damageZoneInfo.useSurfaceData && surfaceName == damageZoneInfo.surfaceName))
				{
					// Set this damage zone data to be the new closest one
					currentClosestDistanceSquared = distZoneCenterSquared;
					currentClosestDamageZoneInfo = &(damageMapIter.second);
				}
			}
		}


		if (currentClosestDamageZoneInfo)
		{
			damageZone = *currentClosestDamageZoneInfo;


			return true;
		}

		return false;
	}



	DtSimComponent* HapticPlugin::creator(const DtString& name,
		DtLocalObject* owner,
		DtSimulationServices* simManager,
		DtComponentDescriptor* desc,
		DtReaderWriterRegistry* parentRegistry)
	{
		std::cout << "[HapticPlugin][Trace] creator() \n";

		//return MarkThreadSafe(new HapticPlugin(name, owner, simManager, desc, parentRegistry));
		return new HapticPlugin(name, owner, simManager, desc, parentRegistry);


	}




	//VRIS -> DSMS
	void HapticPlugin::SendDataToDsms(const BYTE hitBodyZone, const BYTE attackerID[16], const BYTE damage, const int health)
	{
		if (stopRequested)
			return;

		if (dsmsTcpClient == nullptr) return;

		std::cout << "[HapticPlugin][Trace] sendDataToDsms \n";


		vrisToDsmsPkt.hitBodyZone = hitBodyZone;
		memcpy(vrisToDsmsPkt.attakerID, attackerID, sizeof(attackerID));
		vrisToDsmsPkt.damage = damage;
		vrisToDsmsPkt.health = health;


		const size_t size = sizeof(VrisToDsmsPacket);
		unsigned char byteArray[size];
		std::memcpy(&byteArray, &vrisToDsmsPkt, size);


		bool result = dsmsTcpClient->SendData(byteArray, size);

		if (result)
		{
			//std::cout << "[HapticPlugin][Debug] Send data to MonitoringPC : ";
			//std::cout << std::hex << std::uppercase;
			//for (int i = 0; i < size; ++i)
			//	std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(byteArray[i]) << " ";
			//std::cout << std::dec << std::endl;
		}
		else
			LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[HapticPlugin][Error] Failed to send data to the monitoring PC \n";

	}




	//VRIS -> Haptic
	void HapticPlugin::SendDataToHaptic(const BYTE hitBodyZone, const BYTE damageType, const BYTE	damage, const BYTE damageState)
	{
		if (stopRequested)
			return;


		if (hapticTcpServer == nullptr)
			return;

		//Set Data
		vrisToHapticPkt.hitBodyZone = hitBodyZone;
		vrisToHapticPkt.damageType = damageType;
		vrisToHapticPkt.damage = damage;
		vrisToHapticPkt.damageState = damageState;


		//Struct -> byte*
		const size_t size = sizeof(VrisToHapticPacket);
		unsigned char byteArray[size];
		std::memcpy(byteArray, &vrisToHapticPkt, size);


		bool result = hapticTcpServer->SendData(byteArray, size);
		if (result)
		{
			std::cout << "[HapticPlugin][Debug] Send data to Haptic : ";
			std::cout << std::hex << std::uppercase;
			for (int i = 0; i < size; ++i)
				std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(byteArray[i]) << " ";
			std::cout << std::dec << std::endl;
		}
		else
			LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[HapticPlugin][Error] Failed to send data to the Haptic ";


	}



	void HapticPlugin::HandleDsmsDisconnectedThr()
	{

		if (pluginStopping.load(std::memory_order_acquire)) return;

		std::cout << "[HapticPlugin][Trace] Entering function: HandleDsmsDisconnectedThr() " << std::endl;


		std::unique_lock<std::mutex> lock(dsmsMtx);

		bool status = dsmsCV.wait_for(lock, std::chrono::seconds(RECONNECT_DELAY_SEC), [&]() { return pluginStopping.load(std::memory_order_acquire); });
		if (status)
			return;


		if (dsmsTcpClient != nullptr)
			dsmsTcpClient->Connect();

	}




	void HapticPlugin::OnDsmsDisconnected()
	{
		if (pluginStopping.load(std::memory_order_acquire)) return;

		std::cout << "[HapticPlugin][Trace] Entering function: OnDsmsDisconnected() " << std::endl;

		if (dsmsReconnectThr.joinable())
		{
			if (dsmsReconnectThr.get_id() == std::this_thread::get_id())
				dsmsReconnectThr.detach();
			else
				dsmsReconnectThr.join();
		}


		dsmsReconnectThr = std::thread(&HapticPlugin::HandleDsmsDisconnectedThr, this);
	}








	//Mak 피격 부위랑 햅틱&모니터링 피격 부위 포맷형식이랑 매칭
	std::uint8_t HapticPlugin::FormatHitZone(const std::string hitBodyZoneName)
	{
		//std::cout << "[HapticPlugin][Debug] bodyZoneName: " << hitBodyZoneName << std::endl;

		uint8_t result = 0;

		if (hitBodyZoneName.compare("torso-front") == 0)//상체 앞
		{
			result = (uint8_t)HapticHitZone::TorsoFront;
		}
		else if (hitBodyZoneName.compare("torso-left") == 0)
		{
			result = (uint8_t)HapticHitZone::TorsoLeft;
		}
		else if (hitBodyZoneName.compare("torso-right") == 0)
		{
			result = (uint8_t)HapticHitZone::TorsoRight;
		}
		else if (hitBodyZoneName.compare("torso-rear") == 0)
		{
			result = (uint8_t)HapticHitZone::TorsoRear;
		}
		else if (hitBodyZoneName.compare("head") == 0)
		{
			result = (uint8_t)HapticHitZone::Head;
		}
		else if (hitBodyZoneName.compare("legs") == 0)
		{
			result = (uint8_t)HapticHitZone::Legs;
		}
		else if (hitBodyZoneName.compare("All") == 0)//전체 피해(ex) 투척물)
		{
			result = (uint8_t)HapticHitZone::All;
		}


		return result;
	}

	//햅틱&모니터링 피해 상태 포맷형식이랑 매칭
	std::uint8_t HapticPlugin::FormatDamageState(const int healthAfterHit)
	{

		if (healthAfterHit == 100)
		{
			return (uint8_t)DamageState::Normal;
		}
		else if (healthAfterHit >= 50)
		{
			//경상(50~99)
			return (uint8_t)DamageState::Light;
		}
		else if (healthAfterHit > 0)
		{
			//중상(1~49)
			return (uint8_t)DamageState::Serious;
		}
		else//사망(0)
			return (uint8_t)DamageState::Dead;

	}

	makVre::DtVreMessageResult HapticPlugin::HandleStopPluginMessage(makVre::DtVreMessage* msg)
	{

		//ASSERT_TYPE(msg, CustomMessage, rMsg);
		std::cout << "[HapticPlugin][Trace] recv HandleCustomMessage \n";
		
		
		makVre::StopAllPluginsMessage* stopPluginMsg = dynamic_cast<makVre::StopAllPluginsMessage*>(msg);

		if (!stopPluginMsg)
		{
			return makVre::IGNORED;
		}

		// Make sure that this message is meant for our engaged entity.
		if (stopPluginMsg->getSender() != entity()->entityId())
		{
			return makVre::IGNORED;
		}


		std::cout << "[HapticPlugin][Info] StopAllPluginsMessage->getStopRequested(): <<" << std::boolalpha << stopPluginMsg->getStopRequested() << std::endl;

		stopRequested = stopPluginMsg->getStopRequested();


		return makVre::HANDLED;

	}

}


/*

	void HapticPlugin::HandleHapticDisconnectedThr()
	{
		if (pluginStopping.load(std::memory_order_acquire)) return;

		//std::cout << "[HapticPlugin][Trace] Entering function: HandleHapticDisconnectedThr() " << std::endl;


		std::unique_lock<std::mutex> lock(hapticMtx);

		bool status = hapticCv.wait_for(lock, std::chrono::seconds(RECONNECT_DELAY_SEC), [&]() { return pluginStopping.load(std::memory_order_acquire); });
		if (status)
			return;

		if ( hapticTcpServer != nullptr)
			hapticTcpServer->ServerStart();


	}



	void HapticPlugin::OnHapticDisConnected()
	{
		if (pluginStopping.load(std::memory_order_acquire)) return;

		std::cout << "[HapticPlugin][Trace] Entering function: OnHapticDisConnected() " << std::endl;

		if (hapticReconnectThr.joinable())
		{
			if (hapticReconnectThr.get_id() == std::this_thread::get_id())
				hapticReconnectThr.detach();
			else
				hapticReconnectThr.join();
		}
		hapticReconnectThr = std::thread(&HapticPlugin::HandleHapticDisconnectedThr, this);
	}


*/
/*
※※ 중요 ※※
//detonationInter
cout << "[HapticPlugin][DEBUG] detonationContext.detonationInter().result() => " << detonationContext.detonationInter().result() << endl;//지워주기
cout << "[Haptic Plugin] detonationContext.detonationInter().munitionType() => " << detonationContext.detonationInter().munitionType() << endl;

//타인이 자기를 쐈을때
//detonationContext.detonationInter().result() = > 1 = > DtDetResEntityImpact
//detonationContext.detonationInter().munitionType() = > 2:8 : 225 : 2 : 1 : 1 : 0
//detonationContext.detonationInter().munitionType() = > 2:8 : 225 : 2 : 1 : 1 : 0

//자기자신을 쐈을때
//detonationContext.detonationInter().result() = > 3 = > DtDetResGroundImpact 지면 충격
//detonationContext.detonationInter().munitionType() = > 2:8 : 225 : 2 : 1 : 1 : 0 = >
//detonationContext.detonationInter().munitionType() = > 2:8 : 225 : 2 : 1 : 1 : 0


//폭탄으로 죽었을 때
//detonationContext.detonationInter().result() = > 5 = > DtDetResDetonation
//detonationContext.detonationInter().munitionType() = > 2:9 : 225 : 2 : 85 : 1 : 0
//detonationContext.detonationInter().munitionType() = > 2:9 : 225 : 2 : 85 : 1 : 0


//탱크 포격
//[Haptic Plugin] detonationContext.detonationInter().result() = > 1 => DtDetResEntityImpact
//[Haptic Plugin] detonationContext.detonationInter().munitionType() = > 2:2 : 225 : 2 : 13 : 3 : 0


//RPG
//[Haptic Plugin] detonationContext.detonationInter().result() = > 1 => DtDetResEntityImpact


//detonationContext.detonationInter().result() => 5(= DtDetResDetonation)(vlpi/disEnums.h에 포함)


M4
=> 2:8 : 225 : 2 : 1 : 1 : 0
2:8:222:2:2:3:0

M16
2:8:225:2:1:1:0


AK
2:8:222:2:2:3:0

권총
2:8:225:2:3:0:0


폭탄
2: 9 : 225 : 2 : 85 : 1 : 0

유탄발사기
2:2:222:2:8:1:0


*/
/* => 나중에 필요한 데이터 있으면 사용하기

//LOG_INFO("EXAMPLE") << std::setprecision(2) <<
std::cout << "----- Direct hit ----- \n"
<< "surface:  " << workingDetonationContext.surface().c_str() << "\t"
<< "angle of incidence:  " << DtRad2Deg(workingDetonationContext.angleOfIncidence()) << "\t"
<< "body location:  " << workingDetonationContext.detonationInter().entityLocation() << "\t"

//attacker_Id
<< "attackerId_uuidString : " << workingDetonationContext.detonationInter().attackerId().uuidString() << "\t"

//target_Id
<< "targetId_uuidString : " << workingDetonationContext.detonationInter().targetId().uuidString() << "\t"

//Damage_Body_Zone_Name => head, torso-front, torso-left, torso-right,  torso-rear,   legs
<< "body zone : " << damageZoneInfo.bodyZoneName << "\t"

//Entity_localPosition
<< "entityLocalLocation : " << entity()->localPosition() << "\n"


//Player Health
//<< "\n" << "myOverallHealth : " << myOverallHealth->value()
//<< "Health : " << entity()->nextFrameStateProperties().findProperty<DtRwInt>("OverallHealth")->value() << "\n"
<< "Health : " << currentHealth << "\n"
<< "--------------" << std::endl;



(BE)TopoPlugin   => VRE Character Position
(FE)ActionPlugin => VRE Character Heading

void SkeletonPlugin::CharacterPosition(float chaPosX, float chaPosY, float chaPosZ)
{


	//GetPlayerLocation
	localStateRepository()->localPosition();

	////SetPlayerLocation

	DtVector counter(chaPosX, chaPosY, chaPosZ);
	localStateRepository()->setLocalPosition(counter);


	cout << "CharacterLocalPosition => " << localStateRepository()->localPosition() << endl;
}
*/

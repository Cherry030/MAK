
/*									메모
1) Mapping
VR-Engage =>  soilTypeMapping, bodyOfWaterTypeMapping, culturalFeatureTypeMapping, roughnessTypeMapping
- 파일 => vortexConfig.lua
- 경로 => C:\MAK\vrengage2.1.1b\appData\scripts\vortexConfig.lua

-- DtSurface::DtSoilType(21개)
   soilTypeMapping = {
	  SoftSoil = "Mud";
	  Swamp = "Mud";
	  Mud = "Mud";
	  Forest = "Grass";
	  Grass = "Grass";
	  CultivatedFields = "Grass";
	  Orchards = "Grass";
	  Rock = "Grass";
	  Boulder = "Grass";
	  Sand = "Sand";
	  DirtRoad = "Gravel";
	  GravelRoad = "Gravel";
	  PavedRoad = "Road";
	  MuddyRoad = "Road";
	  AsphaltOrOtherHardSurface = "Road";
	  USRailRoad = "Road";
	  EuroRailRoad = "Road";
	  Flimsy = "filter";
	  BodyOfWater = "filter";
	  DryGround = "Road";
	  UndefinedSoilType = "useRoughnessTypeMapping";
   };


   -- DtSurface::DtRoughnessSoilType(8개) => 이동 저항 지형 매핑
   roughnessTypeMapping = {
	  PavedRoad = "Road";
	  HardPacked = "Road";
	  Rocks = "Road";
	  Sand = "Sand";
	  Gravel = "Gravel";
	  ShallowWater = "Mud";
	  Muck = "Mud";
	  DeepWater = "filter";
   };



   2) DtRoughnessSoilType 지형 타입에 따른 이동 저항값 설정
   => C:\MAK\vrengage2.1.1b\data\simulationModelSets\VR-Engage\vrfSim\systems\movement\human.sysdef => soil-to-max-speed-factor-map


   3) 코드(surface.h)
   C:\MAK\vrforces5.1.1b\include\geometry\surface.h

   virtual DtMedium whereSurface() const;
   virtual DtEnvState envState() const;
   virtual DtSoilType soilType() const;
   virtual DtBodyOfWaterType bodyOfWaterType() const;
   virtual DtCulturalFeatureType culturalFeatureType() const;


*/

#include "TerrainSoil.h"

#include <vrfutil/profiler.h>
#include <vrfobjcore/physicalWorld.h>
#include <tdbutil/mathUtilities.h>
#include <vreUtil/logger.h>

#include "Nesslab/nesslabTcpClient.h"
#include "Nesslab/nesslabTcpServer.h"
#include "Nesslab/nesslabCommon.h"
#include "Nesslab/StopAllPluginsMessage.h"



using namespace makVrf;
using namespace makVrv;
using namespace nesslab_common;


namespace nesslab_backend_plugins {

//VRIS --TCP--> SubVris --BLE--> 기동보조장치(MADH), Port = 5888
#pragma pack(push, 1)
	struct VrisToMadhPacket
	{

		PacketHeader header;

		//Payload
		BYTE	resistance = 0;	//이동 저항

		PacketTrailer trailer;


		VrisToMadhPacket()
		{
			header.deviceID = (int)DeviceID::Vris;
			header.msgID	= (uint8_t)VrisMsgID::MobilityResistanceMsgID;
			header.msgType	= 0;
			header.length	= sizeof(VrisToMadhPacket);
		}

	};
#pragma pack(pop)



	//VRIS --TCP--> DSMS
#pragma pack(push, 1)
	struct VrisToDsmsPacket
	{

		PacketHeader header;

		//Data(Payload)
		BYTE	soilType = 0;//지형 타입

		PacketTrailer etx;

		VrisToDsmsPacket()
		{
			header.deviceID = (int)DeviceID::Vris;
			header.msgID = (uint8_t)VrisMsgID::SoilTypeMsgID;//지형 타입 데이터
			header.msgType = 0;//Reserved
			header.length = sizeof(VrisToDsmsPacket);
		}

	};
#pragma pack(pop)



	namespace {

		//MADH(ManeuverAssistant)(기동보조장치)
		//VRIS --TCP--> SubVris --BLE--> 기동보조장치(MADH), Port = 5888 
		nesslabTcpServer*	madhTcpServer = nullptr;
		int					madhPort = -1;
		VrisToMadhPacket	vrisToMadhPacket;


		std::mutex madhMtx;
		std::condition_variable madhCv;
		std::thread madhRecoonectThr;
		std::atomic_bool pluginStopping = false;



		//MAK --TCP--> DSMS, Port = 3005
		nesslabTcpClient*	dsmsTcpClient = nullptr;
		std::string			dsmsIP = "127.0.0.1";
		int					dsmsPort = -1;
		VrisToDsmsPacket	vrisToDsmsPacket;
		std::mutex dsmsMtx;
		std::thread dsmsReconnectThr;

	}

	//Constructor => Enage Human 캐릭터 생성하면 실행
	TerrainSoilPlugin::TerrainSoilPlugin(const DtString& name,
		DtLocalObject* owner,
		DtSimulationServices* simManager,
		DtComponentDescriptor* desc,
		DtReaderWriterRegistry* parentRegistry)
		: DtActuatorComponent(name, owner, simManager, desc, parentRegistry)
	{
		std::cout << "[TerrainSoilPlugin][Trace] Constructor " << std::endl;
	}

	//Destructor => Human 아닌 오브젝트를 생성하거나 다른 인간 캐릭터 생성하면 실행
	TerrainSoilPlugin::~TerrainSoilPlugin()
	{
		std::cout << "[TerrainSoilPlugin][Trace] Destructor " << std::endl;


		makVre::DtVreMessageManager::instance().removeHandler(makVre::StopAllPluginsMessage::theType(),
			makVre::DtVreMessageDelegate(this, &TerrainSoilPlugin::HandleStopPluginMessage));


		pluginStopping.store(true, std::memory_order_release);
		madhCv.notify_all();

		if (netThread.joinable())
			netThread.join();


		if (madhRecoonectThr.joinable())
			madhRecoonectThr.join();

		if (dsmsReconnectThr.joinable())
			dsmsReconnectThr.join();

		if (dsmsTcpClient != nullptr)
		{
			std::lock_guard<std::mutex> lock(dsmsMtx);

			dsmsTcpClient->Disconnect();
			delete dsmsTcpClient;
			dsmsTcpClient = nullptr;
		}

		if (madhTcpServer != nullptr)
		{
			std::lock_guard<std::mutex> lock(madhMtx);

			madhTcpServer->ServerStop();
			delete madhTcpServer;
			madhTcpServer = nullptr;
		}


	}

	bool TerrainSoilPlugin::init()
	{
		if (!DtActuatorComponent::init())
		{
			return false;
		}

		std::cout << "[TerrainSoilPlugin][Trace] init Plugin  \n";

		InitPlugin();


		////Send Message Test
		//std::string customCommand = "CustomMessage " + entity()->uuid().uuidString();
		//std::cout << "[Message]customCommand: " << customCommand << std::endl;
		//DtConsoleCommandManager::globalConsoleCommandManager()->runCommand(customCommand);


		makVre::DtVreMessageManager::instance().addHandler(makVre::StopAllPluginsMessage::theType(),
			makVre::DtVreMessageDelegate(this, &TerrainSoilPlugin::HandleStopPluginMessage));


		return true;
	}

	

	// This returns a string from compTypes.h, and identifies the type of component
	const char* TerrainSoilPlugin::type() const
	{
		//return DtHumanInformationActuatorType;
		return TERRAIN_SOIL_PLUGIN_TYPE;
	}



	void TerrainSoilPlugin::InitPlugin()
	{
		pluginStopping.store(false, std::memory_order_release);
	

		//Get ConfigFile Data
		{

			char databuf[256] = {};

			//MADH
			madhPort = GetPrivateProfileInt("ManeuverAssist", "maneuverAssistPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());

			//DSMS
			GetPrivateProfileString("DSMS", "dsmsIP", INI_STRING_NOT_FOUND_DEFAULT.c_str(), databuf, sizeof(databuf), CONFIG_FILE_PATH.c_str());
			dsmsIP = databuf;
			dsmsPort = GetPrivateProfileInt("ManeuverAssist", "dsmsPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());


			if (dsmsPort == INI_INT_NOT_FOUND_DEFAULT || dsmsIP == INI_STRING_NOT_FOUND_DEFAULT)
				std::cerr << "[TerrainSoilPlugin][Error] Failed to read data from INI file. Using default value.\n";


			if (madhPort == INI_INT_NOT_FOUND_DEFAULT)
				LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[TerrainSoilPlugin][Error] Failed to read data from INI file. Using default value.\n";



			std::cout << "[TerrainSoilPlugin][Debug] madhPort: " << madhPort << std::endl;
		}


		
		//Create and start TCP/UDP
		{
			netThread = std::thread([this]() {

				//VRIS --TCP--> SubVris --BLE--> 기동보조장치(MADH)
				madhTcpServer = new nesslabTcpServer(madhPort);
				madhTcpServer->ServerStart();


				//MAK --TCP--> DSMS
				dsmsTcpClient = new nesslabTcpClient(dsmsPort, dsmsIP);
				dsmsTcpClient->SetOnConnectionChanged(
					[this](nesslabTcpClient::ConnectionState state)
					{
						if (state == nesslabTcpClient::ConnectionState::Disconnected)
							OnDsmsDisconnected();
					}
				);
				dsmsTcpClient->Connect();

			});

		}

	}




	void TerrainSoilPlugin::tick()
	{		
		if (!IsPlayerControlled("Human")) return;

		if (DtIsNotZero(dT()) == false) return;
		DtPROFILEzone;


		//std::cout << "[TerrainSoilPlugin] Tick \n";

		//Height And Surface(or DtVrfChordIntersectionRecord)
		double heightAtLocation;
		DtVrfChordIntersectionRecord record;

		//vre2.2 교차점에 대한 데이터가 더 있어야함
		DtTerrainIntersectStatus interStatus;
		DtTerrainIntersectOptions interOption;
		physicalWorld()->terrainHeightAndSurface(entity()->nextFrameLocalPosition(), heightAtLocation, record, interStatus, interOption);

		//vre2.1.1b
		//physicalWorld()->terrainHeightAndSurface(entity()->nextFrameLocalPosition(), heightAtLocation, record);



		//int currentSoilType = static_cast<int>(record.surface().soilType());
		currentSoilType = record.surface().soilType();
		currentBodyOfWater = record.surface().bodyOfWaterType();


		
		//Moving Resistance Information //TODO: 추후에 Building도 추가해줘야하나 고민
		if (currentSoilType != previousSoilType || currentBodyOfWater != previousBodyOfWater)
		{
			previousSoilType = currentSoilType;
			previousBodyOfWater = currentBodyOfWater;

			
			std::cout << "[TerrainSoilPlugin][Debug] SoilType  = "					<< record.surface().soilTypeString() << std::endl;
			std::cout << "[TerrainSoilPlugin][Debug] bodyOfWaterTypeString  = "		<< record.surface().bodyOfWaterTypeString() << std::endl;//ShallowPond, ShallowStream, DeepLake, DeepRiver, Ocean, ShallowLake, ShallowRiver
			std::cout << "[TerrainSoilPlugin][Debug] culturalFeatureTypeString  = " << record.surface().culturalFeatureTypeString() << std::endl;//Tree, Building
			std::cout << "[TerrainSoilPlugin][Debug] roughnessSoilType = "			<< record.surface().roughnessSoilTypeString() << std::endl;//이동저항 지형타입
			

			//std::string customCommand = "CustomMessage " + entity()->uuid().uuidString();
			//std::cout << "customCommand: " << customCommand << std::endl;
			//DtConsoleCommandManager::globalConsoleCommandManager()->runCommand(customCommand);


			//TODO: 추후에 bodyOfWaterTypeString값도 넣어줘야하면 포맷형식 새로 만들어야할 듯
			SendMoveResistanceToMadh();//VRIS->기동보조장치
			//SendSoilTypeToDsms();
		}

		

	}




	DtSimComponent* TerrainSoilPlugin::creator(const DtString& name,
		DtLocalObject* owner,
		DtSimulationServices* simManager,
		DtComponentDescriptor* desc,
		DtReaderWriterRegistry* parentRegistry)
	{

		return new TerrainSoilPlugin(name, owner, simManager, desc, parentRegistry);
	}




	std::string TerrainSoilPlugin::GetSoilTypeString(const DtSurface& surface)
	{
		if (surface.isWaterSurface()) {
			return surface.bodyOfWaterTypeString();
		}
		else if (surface.isCulturalFeature())
		{
			return surface.culturalFeatureTypeString();
		}
		else if (surface.isGround() || surface.isFlimsy())
		{
			return surface.soilTypeString();
		}
		else
		{
			return surface.roughnessSoilTypeString();
		}
	}



	bool TerrainSoilPlugin::IsPlayerControlled(const std::string& role)
	{

			if (!role.empty())
			{
				// return true if the specific role entry is not empty
				const makVrf::DtVrfStateComponent* component = entity()->getNextFrameStateComponent<makVrf::DtVrfStateComponent>();
				const DtVrfObjectStateRepository::ExtendedData& extendedDataMap = component->extendedData();
				std::string key = "role-" + role;
				DtVrfObjectStateRepository::ExtendedData::const_iterator valueIter = extendedDataMap.find(key.c_str());
				if (valueIter != extendedDataMap.end())
				{
					return true;
				}
			}
			else
			{
				// No specific role given, search map for any entries with a key starting with "role-"
				const DtVrfObjectStateRepository::ExtendedData& extendedDataMap = entity()->getNextFrameStateComponent<makVrf::DtVrfStateComponent>()->extendedData();
				DtVrfObjectStateRepository::ExtendedData::const_iterator iter = extendedDataMap.begin();
				DtVrfObjectStateRepository::ExtendedData::const_iterator end = extendedDataMap.end();
				for (; iter != end; ++iter)
				{
					// return true if we find any role entry that is not empty
					if (iter->first.findString("role-") == 0 && !iter->second.isEmpty())
					{
						return true;
					}
				}
			}

			return false;
	}



	//기동보조장치 지형별 이동 저항  포맷형식이랑 매칭	
	uint8_t TerrainSoilPlugin::FormatSoilTypeMobilityResistance()
	{

		//TODO: 지형별 이동 저항 정보 정해지면 이동 저항값 수정

		//지형에 따른 이동 저항(0~100)
		BYTE movingResistance = 0;

		switch (currentSoilType)
		{
		case DtSoilType::SoftSoil:
			movingResistance = 0;
			break;

		case DtSoilType::Swamp:
			movingResistance = 0;
			break;

		case DtSoilType::Mud:
			movingResistance = 40;
			break;

		case DtSoilType::Forest:
			movingResistance = 20;
			break;

		case DtSoilType::Grass:
			movingResistance = 20;
			break;

		case DtSoilType::CultivatedFields:
			movingResistance = 40;
			break;

		case DtSoilType::Orchards:
			movingResistance = 80;
			break;

		case DtSoilType::Rock:
			movingResistance = 90;
			break;

		case DtSoilType::Boulder:
			movingResistance = 90;
			break;

		case DtSoilType::Sand:
			movingResistance = 90;
			break;

		case DtSoilType::DirtRoad:
			movingResistance = 90;
			break;

		case DtSoilType::GravelRoad:
			movingResistance = 90;
			break;

		case DtSoilType::PavedRoad:
			movingResistance = 90;
			break;

		case DtSoilType::MuddyRoad:
			movingResistance = 90;
			break;

		case DtSoilType::AsphaltOrOtherHardSurface:
			movingResistance = 90;
			break;

		case DtSoilType::USRailRoad:
			movingResistance = 90;
			break;

		case DtSoilType::EuroRailRoad:
			movingResistance = 90;
			break;

		case DtSoilType::Flimsy:
			movingResistance = 90;
			break;

		case DtSoilType::BodyOfWater:
			movingResistance = 90;
			break;

		case DtSoilType::DryGround:
			movingResistance = 90;
			break;

		case DtSoilType::UndefinedSoilType:

		default:
			movingResistance = 90;
			break;

		}


		return movingResistance;
	}


	
	void TerrainSoilPlugin::SendMoveResistanceToMadh()
	{
		if (madhTcpServer == nullptr) return;

		using namespace std;


		//지형에 따른 이동 저항(0~100)
		BYTE movingResistance = FormatSoilTypeMobilityResistance();


		//Set Data
		vrisToMadhPacket.resistance = movingResistance;


		const size_t size = sizeof(VrisToMadhPacket);
		unsigned char byteArray[size];
		std::memcpy(byteArray, &vrisToMadhPacket, size);



		Bool result = madhTcpServer->SendData(byteArray, size);
		if (result)
		{
			//std::cout << "[TerrainSoilPlugin][Debug] Send data to MADH : ";
			//std::cout << std::hex << std::uppercase;
			//for (int i = 0; i < size; ++i)
			//	std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(byteArray[i]) << " ";
			//std::cout << std::dec << std::endl;

		}
		else
			LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[TerrainSoilPlugin][Error] Failed to send data to the MADH ";

	}

	


	void TerrainSoilPlugin::SendSoilTypeToDsms()
	{
		if (dsmsTcpClient == nullptr) return;


		//vrisToDsmsPacket.soilType = previousSoilType;
		vrisToDsmsPacket.soilType = currentSoilType;


		const unsigned int size = sizeof(VrisToDsmsPacket);
		BYTE byteArray[size];
		std::memcpy(&byteArray, &vrisToDsmsPacket, size);


		bool result = dsmsTcpClient->SendData(byteArray, size);
		if (result)
		{
			std::cout << "[TerrainSoilPlugin][Debug] Send Topo data to MonitroingPC : ";
			std::cout << std::hex << std::uppercase;
			for (int i = 0; i < size; ++i)
				std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(byteArray[i]) << " ";
			std::cout << std::dec << std::endl;
		}
		else
			std::cerr << "[TerrainSoilPlugin][Error] Failed to send data to the monitoring PC ";
	}


	void TerrainSoilPlugin::HandleDsmsDisconnectedThr()
	{
		if (pluginStopping.load(std::memory_order_acquire)) return;


		std::unique_lock<std::mutex> lock(dsmsMtx);
		bool status = madhCv.wait_for(lock, std::chrono::seconds(RECONNECT_DELAY_SEC), [&]() { return pluginStopping.load(std::memory_order_acquire); });
		if (status)
			return;

		if (dsmsTcpClient != nullptr)
			dsmsTcpClient->Connect();

	}




	void TerrainSoilPlugin::OnDsmsDisconnected()
	{

		if (pluginStopping.load(std::memory_order_acquire)) return;

		std::cout << "[TerrainSoilPlugin][Trace] Entering function: OnDsmsDisconnected() " << std::endl;



		if (dsmsReconnectThr.joinable())
		{
			if (dsmsReconnectThr.get_id() == std::this_thread::get_id())
				dsmsReconnectThr.detach();
			else
				dsmsReconnectThr.join();
		}

		dsmsReconnectThr = std::thread(&TerrainSoilPlugin::HandleDsmsDisconnectedThr, this);
	}




	makVre::DtVreMessageResult TerrainSoilPlugin::HandleStopPluginMessage(makVre::DtVreMessage* msg)
	{

		//ASSERT_TYPE(msg, CustomMessage, rMsg);
		std::cout << "[TerrainSoilPlugin][Trace] recv HandleCustomMessage \n";


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


		std::cout << "[TerrainSoilPlugin][Info] StopAllPluginsMessage->getStopRequested(): <<" << std::boolalpha << stopPluginMsg->getStopRequested() << std::endl;

		stopRequested = stopPluginMsg->getStopRequested();


		return makVre::HANDLED;

	}





}

/* VRIS -> DSMS 변경 SubVris -> DSMS

		void TerrainSoilPlugin::HandleMadhDisconnectThr()
	{
		if (pluginStopping.load(std::memory_order_acquire)) return;


		std::unique_lock<std::mutex> lock(madhMtx);
		bool status = madhCv.wait_for(lock, std::chrono::seconds(RECONNECT_DELAY_SEC), [&]() { return pluginStopping.load(std::memory_order_acquire); });

		if (status)
			return;

		if (madhTcpServer != nullptr)
			madhTcpServer->ServerStart();

	}



	void TerrainSoilPlugin::OnMadhDisconnected()
	{
		if (pluginStopping.load(std::memory_order_acquire)) return;

		std::cout << "[TerrainSoilPlugin][Trace] Entering function: OnMadhDisconnected() " << std::endl;

		if (madhRecoonectThr.joinable())
		{
			if (madhRecoonectThr.get_id() == std::this_thread::get_id())
				madhRecoonectThr.detach();
			else
				madhRecoonectThr.join();
		}


		madhRecoonectThr = std::thread(&TerrainSoilPlugin::HandleMadhDisconnectThr, this);
	}


*/
#include "CustomHumanControlLogic.h"

#include <math.h>
#include <algorithm>
#include <vector>

#include "vrePlayerStation/vrvIntersector.h"
#include "vreMessages/vrfTaskEntityScript.h"


#include "Nesslab/nesslabUdpServer.h"
#include "Nesslab/nesslabUdpClient.h"
#include "Nesslab/nesslabCommon.h"
#include "Nesslab/StopAllPluginsMessage.h"


using namespace makVre;
using namespace nesslab_common;


/*

							캐릭터 위치
DtVector	localLocation		= playerAttributeStore()->body().myPosition;//geoc
DtTaitBryan localOrientation	= playerAttributeStore()->body().myOrientation;//geoc

*/



//수류탄 투척할때 매니퓰레이터(or 연세대 스켈레톤) 회전각을 이용해서 yaw 조절(WeaponPosePlugin.h)
float gTopoHeadingRad = 0;//0~2π


//캐릭터가 멈춰야 수류탄 투척이가능해서 flag 만듬 => 이동중 수류탄 투척 기능 추가 완료
//bool  gStopMovementFlag = false;



//플러그인 정지 트리거, 트레드밀 --TCP--> BackEndPlugin --VreMessage--> CustomHumanControllLogic
bool gStopRequested = false;



//초실감 인터페이스 정의(v1.0)-20250520 문서확인
//TDM -> VRIS -> DSMS 
#pragma pack(push, 1)
struct TDMDataPacket
{
	PacketHeader header;

	//Payload
	float       velXKmh = 0; // km/h (x속도)
	float       velYKmh = 0; // km/h (y속도)
	float       oriDeg = 0; // 0~360 degree (방향)
	float       velKmh = 0; // km/h (복합속도)
	float       accMs2 = 0; // m/s2 (가속도)
	float       soundDba = 0; // dBA (장비소음)

	float		xRoll = 0;//X 경사도(ROLL)(-30 ~ 30)
	float		yPitch = 0;//Y 경사도(PITCH)(-30 ~ 30)

	uint8_t		tdmOpStat = 0x00;//트레드밀 가동상태, 0x00 미가동, 0x01 전원, 0x02 모션가동, 0x03 비상정지(모션에러)
	uint8_t		tdmRdStat = 0x00;//트레드밀 준비상태, 0x00 준비전, 0x01 준비완료
	uint8_t		manipOpStat = 0x00;//매니퓰레이터 가동상태, 0x00 미가동, 0x01 전원, 0x02 모션가동, 0x03 비상정지(모션에러)
	uint8_t		manipRdStat = 0x00;//매니퓰레이터 준비상태, 0x00 준비전, 0x01 준비완료
	int16_t		manipRotAng = 0;//매니퓰레이터 회전각, -180 ~ 180도
	uint16_t	manipReactSpd = 0;//Active 매니퓰레이터 반응속도, ms (milliseconds)
	uint16_t	manipDelayTime = 0;//Active 매니퓰레이터 지연시간, ms (milliseconds)


	PacketTrailer trailer;

	TDMDataPacket()
	{
		header.deviceID = 0x00;// 전방향 이동장치 => 0x01 소형, 0x02 중형(요청 예정)?
		header.msgID = (uint8_t)TreadmillMsgID::TreadmillMoveMsgID;// 이동 정보
		header.msgType = 0;//Reserved
		header.length = sizeof(TDMDataPacket);//0x31(49)
	}

};
#pragma pack(pop)



//장비 시뮬 모니터링 
#pragma pack(push,1)
struct TreadmillSimMonitorPacket
{
	//Treadmill
	float	velXKmh = 0; // km/h (x속도)
	float	velYKmh = 0; // km/h (y속도)
	float	velKmh = 0; // km/h (복합속도)
	int16_t	manipRotAng = 0;//매니퓰레이터 회전각, -180 ~ 180도

	//VR-Engage Character
	float vreChXkmh = 0;
	float vreChYkmh = 0;
	float vreChZkmh = 0;
	float vreChVelKmh = 0;
	float vreChRotAng = 0;
};

#pragma pack(pop)

//Test
static bool initedFactory = false;
#include "vreMessageManager/vreMessageFactory.h"


namespace nesslab_frontend_plugins {
	namespace {

		//TDM --UDP_Multicast--> VRIS, MulticastIP = 234.2.3.24, Port = 2324
		nesslabUdpServer*	tdmUdpMcastServer = nullptr;
		std::string			tdmMulticastIP = "127.0.0.1";
		int					tdmMulticastPort = -1;
		TDMDataPacket		tdmDataPkt;


		////VRIS --UDP_Unicast--> DSMS, Port = 13000
		//nesslabUdpClient* dsmsUdpUcastClient = nullptr;
		//std::string			dsmsIP = "127.0.0.1";
		//int					dsmsPort = -1;


		std::vector<BYTE>   tdmRecvBuffer{};
		PacketHandlers packetHandlers{};
	}


	//트레드밀 시뮬 모니터링(테스트용)
	namespace {
		nesslabUdpClient* simUcastClient = nullptr;
		std::string		  simIP = "127.0.0.1";
		int				  simPort = 9998;
		TreadmillSimMonitorPacket simPkt;

		//테스트 프로그램 사용할거면 true처리해주기
		bool			  enableSimMonitoring = true;


	}


	CustomHumanControlLogic::CustomHumanControlLogic()
		:DtHumanControlLogic()
	{
		std::cout << "[CustomHumanControlLogic][Trace] Entering function: Constructor()" << std::endl;

		if (initedFactory == false)
		{
			//DtVreMessageManager::instance().factory().registerMessage<CustomMessage>();
			DtVreMessageManager::instance().factory().registerMessage<StopAllPluginsMessage>();
			initedFactory = true;
						
		}

	}

	CustomHumanControlLogic::~CustomHumanControlLogic()
	{
		std::cout << "[CustomHumanControlLogic][Trace] Entering function: Destructor()" << std::endl;



	}




	bool CustomHumanControlLogic::initialize(makVre::DtPlayerStation* player, makVre::DtInitTable& config)
	{
		//do base class init.  Shouldn't fail	
		if (!DtHumanControlLogic::initialize(player, config))
		{
			LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[CustomHumanControlLogic][Error] Failed to initialize base class. " << std::endl;
			return false;
		}

		std::cout << "[CustomHumanControlLogic][Trace] Entering function: initialize()" << std::endl;


		InitPlugin();


		 // Add the message handler to receive internal messages to send out
		DtVreMessageManager::instance().addHandler(StopAllPluginsMessage::theType(),
			DtVreMessageDelegate(this, &CustomHumanControlLogic::HandleStopPluginMessage));


		return true;
	}

	void CustomHumanControlLogic::shutdown()
	{
		std::cout << "[CustomHumanControlLogic][Trace] Entering function: shutdown()" << std::endl;

		//base class shutdown
		DtHumanControlLogic::shutdown();


		if (netThread.joinable())
			netThread.join();


		if (tdmUdpMcastServer != nullptr)
		{
			tdmUdpMcastServer->ServerStop();
			delete tdmUdpMcastServer;
			tdmUdpMcastServer = nullptr;
		}


		//if (dsmsUdpUcastClient != nullptr)
		//{
		//	dsmsUdpUcastClient->Disconnect();
		//	delete dsmsUdpUcastClient;
		//	dsmsUdpUcastClient = nullptr;
		//}


		if (enableSimMonitoring && simUcastClient != nullptr)
		{
			simUcastClient->Disconnect();
			delete simUcastClient;
			simUcastClient = nullptr;
		}

		
		// Remove the event handler for internal messages.
		DtVreMessageManager::instance().removeHandler(StopAllPluginsMessage::theType(),
			DtVreMessageDelegate(this, &CustomHumanControlLogic::HandleStopPluginMessage));

	}




	void CustomHumanControlLogic::tick(double dt)
	{
		//tick있어야 setHeading 처리가됨
		DtHumanControlLogic::tick(dt);

		
		//Test Program
		if (enableSimMonitoring)
		SendDataToSim();

		
		/*
		if (myWeaponList.assigned())
		{
			const std::vector<std::string>& weapons = *myWeaponList;
			std::cout << "[WeaponDebug]" << "weapon count: " << weapons.size() << std::endl;
			for (size_t i = 0; i < weapons.size(); ++i)
			{
				std::cout << "[WeaponDebug]" << "  [" << i << "] " << weapons[i] << std::endl;
			}
		}
		*/



	}


	const char* CustomHumanControlLogic::type() const
	{
		return customHumanCtlPluginType;
	}


	void CustomHumanControlLogic::InitPlugin()
	{

		tdmRecvBuffer.clear();
		packetHandlers.clear();
		gTopoHeadingRad = 0;
		gStopRequested = false;

		//Get ConfigFile Data
		{

			char databuf[256] = {};

			//Treadmill, TDM --UDP_Multicast--> VRIS
			GetPrivateProfileString("Treadmill_Move", "tdmMoveIP", INI_STRING_NOT_FOUND_DEFAULT.c_str(), databuf, sizeof(databuf), CONFIG_FILE_PATH.c_str());
			tdmMulticastIP = databuf;
			tdmMulticastPort = GetPrivateProfileInt("Treadmill_Move", "tdmMovePort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());


			////DSMS, VRIS --UDP_Unicast--> DSMS
			//GetPrivateProfileString("DSMS", "dsmsIP", INI_STRING_NOT_FOUND_DEFAULT.c_str(), databuf, sizeof(databuf), CONFIG_FILE_PATH.c_str());
			//dsmsIP = databuf;
			//dsmsPort = GetPrivateProfileInt("Treadmill_Move", "dsmsPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());


			if (tdmMulticastPort == INI_INT_NOT_FOUND_DEFAULT || tdmMulticastIP == INI_STRING_NOT_FOUND_DEFAULT)
				LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[TDMMovePlugin][Error] Failed to read data from INI file. Using default value.\n";


			std::cout << "[TDMMovePlugin][Debug] tdmMulticastIP: " << tdmMulticastIP << ", tdmMulticastPort: " << tdmMulticastPort << std::endl;

		}


		//Store packet data(데이터 받는 패킷만 처리 => 파싱, 콜백 처리용)
		{

			//Weapon -> VRIS
			//소형 이동 정보 패킷
			RegisterPacketHandler<TDMDataPacket>(packetHandlers, (uint8_t)DeviceID::Stms, (uint8_t)TreadmillMsgID::TreadmillMoveMsgID, tdmDataPkt,
				[this](const TDMDataPacket& packet)
				{
					OnTdmDataPkt(packet);
				}
			);

			//중형 이동 정보 패킷
			RegisterPacketHandler<TDMDataPacket>(packetHandlers, (uint8_t)DeviceID::Mtms, (uint8_t)TreadmillMsgID::TreadmillMoveMsgID, tdmDataPkt,
				[this](const TDMDataPacket& packet)
				{
					OnTdmDataPkt(packet);
				}
			);
		}


		//Create and start TCP/UDP
		{

			//YSU --TCP--> VRIS
			netThread = std::thread([this]() {

				//TDM --UDP_Multicast--> VRIS
				tdmUdpMcastServer = new nesslabUdpServer(tdmMulticastPort, nesslabUdpServer::PacketTransmissionMode::Multicast, tdmMulticastIP);
				tdmUdpMcastServer->SetOnDataReceived(
					[this](unsigned char* data, int len)
					{
						OnDataReceived(data, len);
					});
				tdmUdpMcastServer->ServerStart();


				////VRIS --UDP_Unicast--> Dsms
				//dsmsUdpUcastClient = new nesslabUdpClient(dsmsPort, dsmsIP.data(), nesslabUdpClient::PacketTransmissionMode::Unicast);
				//dsmsUdpUcastClient->Connect();


				
				if (enableSimMonitoring)
				{
					simUcastClient = new nesslabUdpClient(simPort, simIP, nesslabUdpClient::PacketTransmissionMode::Unicast);
					simUcastClient->Connect();
				}

			});

		}

	}



	/*
							※※※매니퓰레이터 회전 메모※※※

	- 매니퓰레이터 회전값 => 소수점 2자리까지 표시하기위해서 트레드밀에서 보낼때 100곱해서 송신
	ex) 26.15f => 2615

	- 트레드밀은 북쪽방향이 정면이면 매니퓰레이터는 동쪽이 정면(매니퓰레이터는 왼쪽방향이 +, 오른쪽방향이 -)

	- 트레드밀,MAK 정면 = 북쪽, 왼쪽 -, 오른쪽 + (0 ~ 360)
	- 매니퓰레이터 정면 = 동쪽,	왼쪽 +, 오른쪽 - (매니퓰레이터 정면 = Mak에서 90도)(-180 ~ 180)

	*/
	float CustomHumanControlLogic::ConvertManipAngle(float oldAngle) {

		//소수점 2자리
		//oldAngle /= 100.0f;
		oldAngle *= 0.01f;

		//Mak 회전각이랑 같게 매니퓰레이터 회전각 변환
		//ex) 0 -> 90, 90 -> 360, -90 -> 180, 180 -> 270 (매니퓰레이터 -> Mak)
		float newAngle = oldAngle - 90;
		if (newAngle < 0)
			newAngle += 360;

		newAngle = 360 - newAngle;


		return newAngle;
	}


	constexpr double MPS_TO_KMH = 3.6;
	constexpr double KMH_TO_MPS = 1.0 / 3.6;

	void CustomHumanControlLogic::OnTdmDataPkt(const TDMDataPacket& packet)
	{
		
		if (gStopRequested)
			return;


		//매니퓰레이터 회전각 변경(북쪽방향 정면기준, 오른쪽방향이 + 되도록 변환, 0~360)
		float manipRotAng = ConvertManipAngle(packet.manipRotAng);//headingDeg

		//headingDeg -> Radian
		const float headingRad = manipRotAng * (M_PI / 180.0f);

		//Set Heading
		setCurrentHeading(headingRad);
		gTopoHeadingRad = headingRad;

		

		/*
		{

			//현재 VRE 캐릭터 속도 -> 2.1.1b
			DtVector32 playerVel = playerAttributeStore()->body().myVelocity;//m/s
			const double chSpeedKmh = std::sqrt(playerVel.x() * playerVel.x() + playerVel.y() * playerVel.y() + playerVel.z() * playerVel.z()) * MPS_TO_KMH;
			float tdmVelXkmh = packet.velXKmh;
			float tdmVelYkmh = packet.velYKmh;
			const double tdmSpeedKmh = std::hypot(tdmVelXkmh, tdmVelYkmh);//평면기준 목표 속력(월드 벡터 크기)

			std::cout << "[Treadmill][Debug] headingDeg: " << manipRotAng
					  << ", tdmSpeedkmh: " << tdmSpeedKmh << "kmh"//평면 목표 속력
					  << ", chSpeedkmh: " << chSpeedKmh << "kmh"//현재 캐릭터 속도
					  << std::endl;
		}
		*/

		

		/*
		{
			//현재 VRE 캐릭터 속도 -> 2.2
			makVre::DtAttributeHandle handle = playerAttributeStore()["ownship"]["velocity"];
			if (handle.assigned())
			{
				DtVector32 playerVel = handle->get<DtVector32>();
				const double chSpeedKmh = std::sqrt(playerVel.x() * playerVel.x() + playerVel.y() * playerVel.y() + playerVel.z() * playerVel.z()) * MPS_TO_KMH;
				float tdmVelXkmh = packet.velXKmh;
				float tdmVelYkmh = packet.velYKmh;
				const double tdmSpeedKmh = std::hypot(tdmVelXkmh, tdmVelYkmh);//평면기준 목표 속력(월드 벡터 크기)

				std::cout << "[Treadmill][Debug] headingDeg: " << manipRotAng
					<< ", tdmSpeedkmh: " << tdmSpeedKmh << "kmh"//평면 목표 속력
					<< ", chSpeedkmh: " << chSpeedKmh << "kmh"//현재 캐릭터 속도
					<< std::endl;

			}
		}
		*/



	}



	void CustomHumanControlLogic::OnDataReceived(unsigned char* data, int len)
	{
		//std::cout << "[TDMMovePlugin][Debug] OnDataReceived() \n";

		//Sub_Vris에서 트레드밀로 회전값 보내는거 리턴처리(UDP_Multicast)
		BYTE msgID = data[2];
		if (msgID == 2) return;

		/*
		std::cout << "[TDMMovePlugin][DEBUG] Entering function: ReceiveData() " << std::endl;
		//std::cout << "[트레드밀 수신] 데이터 : ";
		std::cout << "[Treadmill]  Received Data : ";
		std::cout << "Data Length : " << len << ", Data : ";
		std::cout << std::hex << std::uppercase;
		for (int i = 0; i < len; ++i)
			std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
		std::cout << std::dec << std::endl;


					std::cout << "[트레드밀 수신 데이터] STX :" << (int)tdmDataPkt.header.stx
				<< ", DEVICE_ID : " << (int)tdmDataPkt.header.deviceID
				<< ", MSG_ID : " << (int)tdmDataPkt.header.msgID
				<< ", MSG_TYPE : " << (int)tdmDataPkt.header.msgType
				<< ", LENGTH : " << (int)tdmDataPkt.header.length
				<< ", x속도 : " << tdmDataPkt.velXKmh
				<< ", Y속도 : " << tdmDataPkt.velYKmh
				<< ", 방향 : " << tdmDataPkt.oriDeg
				<< ", 복합속도 : " << tdmDataPkt.velKmh
				<< ", 가속도 : " << tdmDataPkt.accMs2
				<< ", 소음 : " << tdmDataPkt.soundDba

				<< ", X경사도 : " << tdmDataPkt.xRoll
				<< ", Y경사도 : " << tdmDataPkt.yPitch
				<< ", 트레드밀 가동상태 : " << (int)tdmDataPkt.tdmOpStat
				<< ", 트레드밀 준비상태 : " << (int)tdmDataPkt.tdmRdStat
				<< ", 매니퓰레이터 가동상태 : " << (int)tdmDataPkt.manipOpStat
				<< ", 매니퓰레이터 준비상태 : " << (int)tdmDataPkt.manipRdStat
				<< ", 매니퓰레이터 회전각 : " << (int)tdmDataPkt.manipRotAng
				<< ", Active 매니퓰레이터 반응속도 : " << (int)tdmDataPkt.manipReactSpd
				<< ", Active 매니퓰레이터 지연시간 : " << (int)tdmDataPkt.manipDelayTime
				<< ", ETX : " << (int)tdmDataPkt.trailer.etx
				<< std::endl;
				*/

		tdmRecvBuffer.insert(tdmRecvBuffer.end(), data, data + len);

		ParseReceiveBuffer(packetHandlers, tdmRecvBuffer);

	}


	//※탄약 이름 처리 방식이(2.1.1.b, 2.2 ver에서 서로 다름)
	//이동하면서 수류탄 투척 기능, 하지만 수류탄 투척 애니메이션은 재생되지않는다.
	//멈춰야 수류탄 투척하는 이유 문의 답변(yklee@nesslab.com, PS-14008)
	// => 수류탄 투척은 전신 동작이므로, 저희 애니메이션 시스템은 동작 속도를 고려하여 캐릭터의 이동 속도에 맞춰 애니메이션을 적용합니다. 
	// 따라서 수류탄 투척 애니메이션이 실행되면 캐릭터는 완전히 멈춥니다.
	void CustomHumanControlLogic::throwGrenade()
	{

		if (!playerAttributeStore()->getAttributeOr<bool>("weaponReady", false))
		{
			return;
		}

		
		//enum makVre::Equipment 3=>GRENADE 
		std::cout << "playerAttributeStore()->getAttributeOr<Equipment>(currentEquipment, NONE) =>  " << playerAttributeStore()->getAttributeOr<Equipment>("currentEquipment", NONE) << std::endl;
		if (playerAttributeStore()->getAttributeOr<Equipment>("currentEquipment", NONE) != GRENADE)
		{
			LOG_INFO("HumanLogic") << "throwGrenade called when grenade is inactive" << std::endl;
			return; // perhaps weapon was switched while stopping to throw grenade
		}


		if (getCurrentAmmo() <= 0.0)
		{
			return;
		}


		//VRE-2.2 Ver(munition name) ex) WeaponId=<FirstAvailable>;AmmoType=resource:other-4|M18-green
		std::string munitionName = "WeaponId=<FirstAvailable>;AmmoType=" + myCurrentResourceHandle->name();

		/* VRE-2.1.1b Ver(munition name)		
		std::string munitionName = playerAttributeStore()->getAttributeOr<std::string>("currentMunition", "");
		if (munitionName.empty()){
			LOG_FATAL("HumanLogic") << "Attempt to fire undefined grenade" << std::endl;
			return;
		}
		munitionName = "Grenade:" + munitionName;
		*/


		DtVector	 pos = playerAttributeStore()->getAttribute<DtVector>("weaponLocation");
		DtTaitBryan	 ori = playerAttributeStore()->getAttribute<DtTaitBryan>("weaponOrientation");
		DtVector	 loc;

		if (makVre::DtVrvIntersector(*myDe).getIntersection(pos, ori, loc))
		{
			makVre::VrfScriptedEntityTaskMessage* msg = makVre::VrfScriptedEntityTaskMessage::create();
			msg->setEntityId(myPlayer->entityId());
			msg->setScriptName("Fire_For_Effect_on_Location");
			msg->addAttribute<DtVector>("location", loc);
			msg->addAttribute<std::string>("munition", munitionName);
			msg->addAttribute<int>("numRnds", 1);
			msg->addAttribute<double>("heightAboveTerrain", 0.0);

			makVre::DtVreMessageManager::instance().queueMessage(msg);


			/* => VRF animation
			
			VrfTaskAnimationMessage* amsg = VrfTaskAnimationMessage::create();
			amsg->setEntityId(myPlayer->entityId());
			amsg->setAnimation("throw_grenade");
			DtVreMessageManager::instance().queueMessage(amsg);

			*/

			//std::cout << "getCurrentAmmo: " << getCurrentAmmo() << std::endl;



			/*
			
			//VRE2.2 엔진 버그(yklee@nesslab.com 계정 PS-14074 문의 확인)
			//-> 수류탄 투척 후 리소스 개수 및 UI 업데이트 딜레이
			//-> VRE2.2c 버전에서 해결됨
			//임시 해결방법
			//수류탄을 던지는 즉시 HUD 오버레이가 업데이트
			//HUD에 표시되는 값을 결정하는 currentAmount속성을 사전에 업데이트하기 위해서 추가 
			 
			// Decrement grenade count after a successful throw
			const double newAmount = getCurrentAmmo() - 1.0;
			myCurrentResourceHandle["currentAmount"]->set(newAmount);

			*/

		}


	}



	//#include <framework/vrePlayerStation/kinetic.h>

	//테스트용	
	void CustomHumanControlLogic::SendDataToSim()
	{

		//Tdm
		simPkt.velXKmh = tdmDataPkt.velXKmh;
		simPkt.velYKmh = tdmDataPkt.velYKmh;
		simPkt.velKmh = tdmDataPkt.velKmh;
		simPkt.manipRotAng = tdmDataPkt.manipRotAng;




		//현재 VRE 캐릭터 속도 -> 2.2
		makVre::DtAttributeHandle handle = playerAttributeStore()["ownship"]["velocity"];
		if (handle.assigned())
		{
			//VRE 현재 캐릭터 속도
			DtVector32 playerVel = handle->get<DtVector32>();//mps
			const double chSpeedKmh = std::sqrt(playerVel.x() * playerVel.x() + playerVel.y() * playerVel.y() + playerVel.z() * playerVel.z()) * MPS_TO_KMH;//kmh
			//float tdmVelXkmh = packet.velXKmh;
			//float tdmVelYkmh = packet.velYKmh;
			//const double tdmSpeedKmh = std::hypot(tdmDataPkt.velXKmh, tdmDataPkt.velYKmh);//평면기준 목표 속력(월드 벡터 크기)
			float chHeadingDeg = ConvertManipAngle(tdmDataPkt.manipRotAng);

			simPkt.vreChXkmh = playerVel.x();
			simPkt.vreChYkmh = playerVel.y();
			simPkt.vreChZkmh = playerVel.z();
			simPkt.vreChVelKmh = chSpeedKmh;
			simPkt.vreChRotAng = chHeadingDeg;


			//strcut -> btye*
			const size_t pktSize = sizeof(TreadmillSimMonitorPacket);
			uint8_t pktData[pktSize];
			memcpy(pktData, &simPkt, pktSize);

			simUcastClient->SendData(pktData, pktSize);
		}

		//std::cout << "[Treadmill][Debug] headingDeg: " << chHeadingDeg
		//	<< ", tdmSpeedkmh: " << tdmSpeedKmh << "kmh"//평면 목표 속력
		//	<< ", chSpeedkmh: " << chSpeedKmh << "kmh"//현재 캐릭터 속도
		//	<< std::endl;
	}


	//트레드밀 -> VRIS, 플러그인 정지 요청
	makVre::DtVreMessageResult CustomHumanControlLogic::HandleStopPluginMessage(makVre::DtVreMessage* msg)
	{

		//ASSERT_TYPE(msg, CustomMessage, rMsg);
		std::cout << "[CustomHuman] recv HandleStopPluginMessage \n";


		makVre::StopAllPluginsMessage* stopPluginMsg = dynamic_cast<makVre::StopAllPluginsMessage*>(msg);

		if (!stopPluginMsg)
		{
			std::cout << "Check1\n";
			return makVre::IGNORED;
		}

		// Make sure that this message is meant for our engaged entity.
		if (stopPluginMsg->getSender() != player().entityId())
		{
			std::cout << "Check2\n";
			std::cout << "stopPluginMsg->getSender():" << stopPluginMsg->getSender() << ", player().entityId():" << player().entityId() << std::endl;
			return makVre::IGNORED;
		}

		std::cout << "[CustomHuman] StopAllPluginsMessage->getStopRequested(): <<" << std::boolalpha << stopPluginMsg->getStopRequested() << std::endl;

		gStopRequested = stopPluginMsg->getStopRequested();


		return makVre::HANDLED;
	}

}
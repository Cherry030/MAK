#include "ActionPlugin.h"
#include "Nesslab/nesslabTcpServer.h"

//#include <chrono>

using namespace makVre;
using namespace makVrv;
using namespace nesslab_common;


extern std::vector<WeaponInfo> gMyWeapons;
//extern bool gStopMovementFlag;


// YSU 행동분석 장비 --Tcp--> VRIS
namespace ysu_to_vris_packets
{
#pragma pack(push, 1)
	struct ActionLabelPacket
	{
		PacketHeader header;

		//Payload
		int actionLabel = 0;

		PacketTrailer trailer;

		ActionLabelPacket()
		{
			//deviceID => 0x54 인프라, 0x68 세미인프라
			//msgID    => 0x57 인프라-행동, 0x68 세미인프라-행동
			header.length = sizeof(ActionLabelPacket);//0x11
		}
	};
#pragma pack(pop)


	//TODO => Payload 부분에 클라이언트 상태 하나 더 추가 + SubVRIS(스켈레톤+무기 SubVRIS 프로그램)에서 패킷 전송
	struct DeviceStatusPacket
	{
		PacketHeader header;

		//Payload
		BYTE clientStatus = 0x00;//0=전원OFF, 1=전원ON, 2=모션ON, 3=모션ERROR, 4=비상정지, 5=RGB 1(5) 카메라 비활성화, 6=RGB 2(6) 카메라 비활성화, 7=RGB 3(7) 카메라 비활성화, 8=RGB 4(8) 카메라 비활성화
		BYTE masterStatus = 0x00;//0=전원OFF, 1=전원ON, 2=모션ON, 3=모션ERROR, 4=비상정지

		PacketTrailer trailer;

		DeviceStatusPacket()
		{
			//deviceID => 0x54 인프라, 0x68 세미인프라
			//msgID    => 0x58 인프라-장비상태, 0x69- 세미인프라-장비상태
			header.length = sizeof(DeviceStatusPacket);//0x09
		}
	};

}


namespace nesslab_frontend_plugins
{

	namespace {

		//행동(Master) 장비 --TCP--> VRIS, Port = 5990
		nesslabTcpServer* actionTcpServer = nullptr;
		int					actionTcpPort = -1;

		//Action Packets
		ysu_to_vris_packets::ActionLabelPacket  ysuActionLabelPkt;
		std::vector<BYTE> actionRecvBuffer;
		nesslab_common::PacketHandlers packetHandlers;


		//DeviceStatus
		ysu_to_vris_packets::DeviceStatusPacket ysuDeviceStatusPkt;
	}


	ActionPlugin::ActionPlugin()
		:DtPlayerComponent()
	{
		std::cout << "[ActionPlugin][Trace] Constructor " << std::endl;
	}


	ActionPlugin::~ActionPlugin()
	{
		std::cout << "[ActionPlugin][Trace] Destructor " << std::endl;
	}


	bool ActionPlugin::initialize(makVre::DtPlayerStation* player, makVre::DtInitTable& config)
	{
		std::cout << "[ActionPlugin][Trace] initialize \n ";
		//do base class init.  Shouldn't fail
		if (!DtPlayerComponent::initialize(player, config))
		{
			LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[ActionPlugin][Error] Failed to initialize base class. " << std::endl;
			return false;
		}

		InitPlugin();


		return true;
	}




	bool ActionPlugin::postInitialize()
	{
		std::cout << "[ActionPlugin][Trace] postInitialize \n ";

		humanControLogic = myPlayer->findComponent<DtHumanControlLogic>();

		if (humanControLogic == nullptr)
		{
			LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[ActionPlugin][Error] Could not find DtHumanControlLogic component." << std::endl;
			return false;
		}


		/*
		//Custom Message는 등록 필요
		//BackEnd -> FrontEnd = FrontEnd Message 등록x
		//FrontEnd -> BackEnd = 양쪽다 Message 등록
		makVre::DtVreMessageManager::instance().factory().registerMessage<makVre::StopAllPluginsMessage>();
		auto* fMsg = makVre::ForwardMessageMessage::create();
		fMsg->setMessageName(makVre::StopAllPluginsMessage::theType());
		makVre::DtVreMessageManager::instance().queueMessage(fMsg);
		*/
		


		return DtPlayerComponent::postInitialize();
	}



	void ActionPlugin::tick(double dt)
	{
	}


	void ActionPlugin::shutdown()
	{
		std::cout << "[ActionPlugin][Trace] shutdown " << std::endl;



		//base class shutdown
		DtPlayerComponent::shutdown();

		if (netThread.joinable())
			netThread.join();

		if (actionTcpServer != nullptr)
		{
			actionTcpServer->ServerStop();
			delete actionTcpServer;
			actionTcpServer = nullptr;
		}
	}


	const char* ActionPlugin::type() const
	{
		return actionPluginType;
	}



	void ActionPlugin::InitPlugin()
	{

		actionRecvBuffer.clear();
		packetHandlers.clear();

		//Get ConfigFile Data
		{
			//YSU
			actionTcpPort = GetPrivateProfileInt("YSU_Action", "actionPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());

			if (actionTcpPort == INI_INT_NOT_FOUND_DEFAULT)
				LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[ActionPlugin][Error] Failed to read data from INI file. Using default value" << std::endl;

			std::cout << "[ActionPlugin][Debug] actionPort: " << actionTcpPort << std::endl;

		}

		//Store packet data(파싱, 콜백 처리용)
		{

			//Action -> VRIS
			//Infra Action Label Packet
			RegisterPacketHandler<ysu_to_vris_packets::ActionLabelPacket>(packetHandlers, (int)DeviceID::Iars, (uint8_t)ActionMsgID::InfraActionInfoMsgID, ysuActionLabelPkt,
				[this](const ysu_to_vris_packets::ActionLabelPacket& packet) {
					OnActionLabelPkt(packet);
				});


			//Semi-Infra Action Label Packet
			RegisterPacketHandler<ysu_to_vris_packets::ActionLabelPacket>(packetHandlers, (int)DeviceID::Sars, (uint8_t)ActionMsgID::SemiInfraActionInfoMsgID, ysuActionLabelPkt,
				[this](const ysu_to_vris_packets::ActionLabelPacket& packet) {
					OnActionLabelPkt(packet);

				});


			//int INF_DEVICE_STATUS_MSG_ID  = 0x54;
			int SEMI_DEVICE_STATUS_MSG_ID = 0x58;


			RegisterPacketHandler<ysu_to_vris_packets::DeviceStatusPacket>(packetHandlers, (int)DeviceID::Iars, SEMI_DEVICE_STATUS_MSG_ID, ysuDeviceStatusPkt,
				[this](const ysu_to_vris_packets::DeviceStatusPacket& packet) {
					OnDeviceStatusPkt(packet);
				}
			);//Infra Device Status Packet(0x54, 0x58)

			//INF_DEVICE_STATUS_MSG_ID = 0x68;
			SEMI_DEVICE_STATUS_MSG_ID = 0x69;


			RegisterPacketHandler<ysu_to_vris_packets::DeviceStatusPacket>(packetHandlers, (int)DeviceID::Sars, SEMI_DEVICE_STATUS_MSG_ID, ysuDeviceStatusPkt,
				[this](const ysu_to_vris_packets::DeviceStatusPacket& packet)
				{
					OnDeviceStatusPkt(packet);
				}
			);//Semi-Infra Device Status Packet(0x68, 0x69)

		}




		//Create and start TCP/UDP
		{

			netThread = std::thread([this]() {

				//YSU --TCP--> VRIS
				actionTcpServer = new nesslabTcpServer(actionTcpPort);
				actionTcpServer->SetOnDataReceived(
					[this](const std::string& client, const uint8_t* data, int len)
					{
						OnDataReceived(client, data, len);
					});
				actionTcpServer->ServerStart();

				});
		}

	}


	//YSU -> VRIS, Action Label Callback(Infra, semiInfra)
	void ActionPlugin::OnActionLabelPkt(const ysu_to_vris_packets::ActionLabelPacket& packet)
	{

		//마지막 라벨이랑 같으면 리턴처리
		if (lastActionLabel == packet.actionLabel) return;


		lastActionLabel = packet.actionLabel;


		if (packet.header.deviceID == (int)DeviceID::Iars)
		{
			std::cout << "[ActionPlugin][Debug] 인프라 행동 정보 데이터 수신, ActionIndex : " << (int)packet.actionLabel << std::endl;

			//Dsms(Infra)
			//dsmsActionLabelPkt.header.msgID = (uint8_t)VrisMsgID::InfraActionInfoMsgID;

		}
		else if (packet.header.deviceID == (int)DeviceID::Sars)
		{
			std::cout << "[ActionPlugin][Debug] 세미인프라 행동 정보 데이터 수신, ActionLabelIndex :" << (int)packet.actionLabel << std::endl;


			//Dsms(semi-Infra)
			//dsmsActionLabelPkt.header.msgID = (uint8_t)VrisMsgID::SemiInfraActionInfoMsgID;

		}
		else
		{
			LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[ActionPlugin][Error] Unknown deviceID,     deviceID : 0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(packet.header.deviceID) << std::dec << std::endl;
			return;
		}


		//Test
		//SendVreMessage();


		//행동 이벤트 실행
		HandleActionLabel(packet.actionLabel);

	}



	void ActionPlugin::OnDataReceived(const std::string& clientAddr, const uint8_t* data, int len)
	{

		//std::cout << "[ActionPlugin][DEBUG] Received Data from ActionPosutre Device";
		//std::cout << "Data Length : " << len << ", Data : ";
		//std::cout << std::hex << std::uppercase;
		//for (int i = 0; i < len; ++i)
		//	std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
		//std::cout << std::dec << std::endl;


		actionRecvBuffer.insert(actionRecvBuffer.end(), data, data + len);
		ParseReceiveBuffer(packetHandlers, actionRecvBuffer);

	}



	constexpr int weaponSwitchDelayMs{ 400 };


	void ActionPlugin::HandleActionLabel(int newActionLabel)
	{
		if (gMyWeapons.size() == 0) return;


		int prevWeaponIndex = playerAttributeStore()->getAttribute<int>("weaponIndex");

		//수류탄 인덱스 1회 저장
		if (grenadeWeaponIndex == -1)
		{
			for (const WeaponInfo& weapon : gMyWeapons)
			{
				if (weapon.weaponType == WeaponType::GRENADE && weapon.weaponName == "Frag Grenade")
					grenadeWeaponIndex = weapon.weaponIndex;
			}
		}


		//현재 무기가 수류탄이면 prevWeaponIndex에 소총 인덱스 저장
		if (prevWeaponIndex == grenadeWeaponIndex)
		{
			for (const WeaponInfo& weapon : gMyWeapons)
			{
				if (weapon.weaponType == WeaponType::RIFLE)
					prevWeaponIndex = weapon.weaponIndex;
			}
		}


		//무기 상태 변경
		int currentWeaponStateIndex = humanControLogic->playerAttributeStore()->getAttribute<int>("weaponState");
		if (currentWeaponStateIndex == (int)WeaponStateType::weaponStow_True)
			SetWeaponState(WeaponStateType::weaponStow_false);
		SetWeaponState(WeaponStateType::weaponUp);


		/*
			
			▶ DI-Guy Gesture(FE)
			API
			- humanControLogic->playDiguyGesture("");
			메모
			- 함수 호출 후 1~2초 내에 다시 호출하면 무시됨


			▶ DI-Guy Action(FE)
			API
			- humanControLogic->playDiguyAction("");
			메모
			- C:\MAK\vrengage2.1.1b\appData\settings\vrfSim\character_animations_table.mtl 경로에서
			animation-name 과 일치해야 하며 VR-Forces컴퓨터에서도 보이게하려면 VRF 컴퓨터에서 VRF 경로에서
			character_animations_table.mtl 똑같이 수정
			※ animation-name 밑에 taskable-or-intrinsic-animation "taskable＂라고 되어있는 애니메이션만 호출가능

		*/
		//std::cout << "[행동데이터]  LabelIndex: " << newActionLabel << std::endl;
		ActionLableIndex label = static_cast<ActionLableIndex>(newActionLabel);
		switch (label)
		{

			//훈련자 서서 쏴
		case ActionLableIndex::StandingShoot:
			std::cout << "[행동데이터] 훈련자 서서 쏴" << std::endl;

			//humanControLogic->playDiguyGesture("diguy_signal_hold");
			//humanControLogic->playDiguyGesture("dst_hand_hold_l");
			//humanControLogic->playDiguyGesture("dst_hand_hold_r");
			//humanControLogic->playDiguyAction("stand_ready_talk_3");


			SetPosture(HumanPostureType::STANDING);

			break;

			//훈련자 지향사격 자세
		case ActionLableIndex::AimedShootingStance:
			std::cout << "[행동데이터] 훈련자 지향사격 자세" << std::endl;

			//humanControLogic->playDiguyGesture("diguy_signal_cease_fire");			
			//humanControLogic->playDiguyAction("drive");

			break;


			//훈련자 무릎 쏴
		case ActionLableIndex::KneelingShoot:
			std::cout << "[행동데이터] 훈련자 무릎 쏴" << std::endl;

			//humanControLogic->playDiguyGesture("diguy_signal_assemble");

			//SetPosture(HumanPostureType::KNEELING);			
			//humanControLogic->playDiguyAction("step_backward");

			break;

			//훈련자 앉아 쏴
		case ActionLableIndex::SittingShoot:
			std::cout << "[행동데이터] 훈련자 앉아 쏴" << std::endl;


			//humanControLogic->playDiguyAction("dead");

			break;

			//훈련자 걷기
		case ActionLableIndex::Walking:
			std::cout << "[행동데이터] 훈련자 걷기" << std::endl;


			//humanControLogic->playDiguyAction("side_step_R_aim");


			//TODO: 추후에 애니메이션 필요하면 walk 복사해서 설정바꿔서 시도
			//humanControLogic->playDiguyAction("walk");//intrinsic, 동작X

			break;

			//빠른 걷기
		case ActionLableIndex::FastWalking:
			std::cout << "[행동데이터] 훈련자 빠른 걷기" << std::endl;

			//humanControLogic->playDiguyAction("side_step_L_aim");


			//humanControLogic->playDiguyAction("run");//intrinsic, 동작X
			//humanControLogic->move(1);//동작x
			break;

			//걸으며 지향사격
		case ActionLableIndex::WalkingAimedShoot:
			std::cout << "[행동데이터] 훈련자 걸으며 지향사격" << std::endl;

			//humanControLogic->playDiguyAction("walk_aim");//intrinsic
			break;

			//훈련자 뛰며 지향사격
		case ActionLableIndex::RunningAimedShoot:
			std::cout << "[행동데이터] 훈련자 뛰며 지향사격" << std::endl;

			break;

			//훈련자 찔러 총
		case ActionLableIndex::RifleButtThrust:
			std::cout << "[행동데이터] 훈련자 찔러 총" << std::endl;

			break;

			//훈련자 때려 총
		case ActionLableIndex::RifleButtStrike:
			std::cout << "[행동데이터] 훈련자 때려 총" << std::endl;

			break;

			//앉은 채 수류탄 투척
		case ActionLableIndex::SeatedGrenadeThrow:


			//if (grenadeWeaponIndex == -1 || grenadeWeaponIndex >= gMyWeapons.size() || prevWeaponIndex >= gMyWeapons.size())
			if (grenadeWeaponIndex == -1)
			{
				LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[ActionPlugin][Error] WeaponIndex out of range.  grenadeWeaponIndex = " << grenadeWeaponIndex << ", prevWeaponIndex: " << prevWeaponIndex << std::endl;
				return;
			}

			std::cout << "[행동데이터] 훈련자 앉은 채 수류탄 투척" << std::endl;


			SetPosture(HumanPostureType::KNEELING);
			SetWeaponIndex(grenadeWeaponIndex);


			//던지기전에 캐릭터 이동 정지 => 안해주면 가끔 공격 무시됨
			//gStopMovementFlag = true;


			//대기 없으면 무기 변경되기전에 이전 무기 발사 => 재장전 속도 조절하고 테스트
			std::this_thread::sleep_for(std::chrono::milliseconds(weaponSwitchDelayMs));

			humanControLogic->fire(1);

			//대기 안하면 prevWeaponIndex무기로 변경한 후 발사됨
			std::this_thread::sleep_for(std::chrono::milliseconds(weaponSwitchDelayMs));

			//이전 무기로 변경
			SetWeaponIndex(prevWeaponIndex);


			//gStopMovementFlag = false;

			break;

			//서서 수류탄 투척
		case ActionLableIndex::StandingGrenadeThrow:


			//if (grenadeWeaponIndex == -1 || grenadeWeaponIndex >= gMyWeapons.size() || prevWeaponIndex >= gMyWeapons.size())
			if (grenadeWeaponIndex == -1)
			{
				LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[ActionPlugin][Error] WeaponIndex out of range.  grenadeWeaponIndex = " << grenadeWeaponIndex << ", prevWeaponIndex: " << prevWeaponIndex << std::endl;
				return;
			}

			std::cout << "[행동데이터] 훈련자 서서 수류탄 투척" << std::endl;

			SetPosture(HumanPostureType::STANDING);
			SetWeaponIndex(grenadeWeaponIndex);


			//gStopMovementFlag = true;

			std::this_thread::sleep_for(std::chrono::milliseconds(weaponSwitchDelayMs));

			humanControLogic->fire(1);

			std::this_thread::sleep_for(std::chrono::milliseconds(weaponSwitchDelayMs));

			SetWeaponIndex(prevWeaponIndex);

			//gStopMovementFlag = false;

			break;


			//훈련자 사격 중 탄창 교체
		case ActionLableIndex::ReloadDuringFire:

			std::cout << "[행동데이터] 훈련자 사격 중 탄창 교체" << std::endl;
			humanControLogic->reload(1);

			break;

			//소총교체(권총->소총)
		case ActionLableIndex::WeaponSwitchToRifle:

			std::cout << "[행동데이터] 소총교체(권총->소총)" << std::endl;
			SetWeaponIndex(0);

			break;

			//권총교체(소총->권총)
		case ActionLableIndex::WeaponSwitchToPistol:

			std::cout << "[행동데이터] 권총교체(소총->권총)" << std::endl;
			SetWeaponIndex(1);

			break;

		default:
			break;

		}
	}

	//YSU -> VRIS, Device Status Callback(Infra, semiInfra)
	void ActionPlugin::OnDeviceStatusPkt(const ysu_to_vris_packets::DeviceStatusPacket& packet)
	{

		if (packet.header.deviceID == (int)DeviceID::Iars)
		{
			std::cout << "[ActionPlugin][Debug] 인프라 장비 상태 데이터 수신, Client Computer: ";

			//dsmsDeviceStatusPkt.header.msgID = DSMS_INF_DEVICE_STATUS_MSG_ID;

			//Infra
			switch (packet.clientStatus)
			{
			case 0:
				std::cout << "전원 OFF";
				break;
			case 1:
				std::cout << "전원 ON";
				break;
			case 2:
				std::cout << "모션 ON";
				break;
			case 3:
				std::cout << "모션 ERROR";
				break;
			case 4:
				std::cout << "비상 정지";
				break;
			case 5:
				std::cout << "RGB 1(5)";
				break;
			case 6:
				std::cout << "RGB 2(6)";
				break;
			case 7:
				std::cout << "RGB 3(7)";
				break;
			case 8:
				std::cout << "RGB 4(8)";
				break;
			default:
				std::cout << "Error";
				return;
			}

		}
		else if (packet.header.deviceID == (int)DeviceID::Sars)
		{

			std::cout << "[ActionPlugin][Debug] 세미 인프라 장비 상태 데이터 수신 : Mash Computer : ";

			//dsmsDeviceStatusPkt.header.msgID = DSMS_SEMI_DEVICE_STATUS_MSG_ID;

			//semi-Infra
			switch (packet.clientStatus)
			{
			case 0:
				std::cout << "전원 OFF";
				break;
			case 1:
				std::cout << "전원 ON";
				break;
			case 2:
				std::cout << "모션 ON";
				break;
			case 3:
				std::cout << "모션 ERROR";
				break;
			case 4:
				std::cout << "비상 정지";
				break;
			case 5:
				std::cout << "RGB 1";
				break;
			case 6:
				std::cout << "RGB 2";
				break;
			case 7:
				std::cout << "RGB 3(7)";
				break;
			case 8:
				std::cout << "RGB 4(8)";
				break;
			default:
				std::cout << "Error";
				return;
			}

		}
		else
		{
			std::cerr << "[ActionPlugin][Error] Unknown deviceID,     deviceID : 0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(packet.header.deviceID) << std::dec << std::endl;
			return;
		}


		std::cout << ", Master : ";
		switch (packet.masterStatus)
		{
		case 0:
			std::cout << "전원 OFF";
			break;
		case 1:
			std::cout << "전원 ON";
			break;
		case 2:
			std::cout << "모션 ON";
			break;
		case 3:
			std::cout << "모션 ERROR";
			break;
		case 4:
			std::cout << "비상 정지";
			break;

		default:
			std::cout << "Error";
			return;
		}
		std::cout << "\n";



		////Vris -> Dsms
		//dsmsDeviceStatusPkt.header.deviceID = VRIS_DEVICE_ID;
		//dsmsDeviceStatusPkt.clientStatus = packet.clientStatus;
		//dsmsDeviceStatusPkt.masterStatus = packet.masterStatus;
		//const size_t size = sizeof(dsmsDeviceStatusPkt);
		//BYTE sendData[size];
		//std::memcpy(&sendData, &dsmsDeviceStatusPkt, size);
		////Vris -> DSMS
		//SendDataToDsms(sendData, size);


	}



	/*
	bool stopRequested = false;
	void ActionPlugin::SendVreMessage()
	{
		std::cout << "[ActionPlugin][Trace] SendVreMessage \n";


		//Message 생성
		makVre::StopAllPluginsMessage* msg = makVre::StopAllPluginsMessage::create();	
		msg->setSender(player().entityId());

		stopRequested = (stopRequested == true) ? false : true;
		msg->setStopRequested(stopRequested);

		makVre::DtVreMessageManager::instance().queueMessage(msg);
	}
	*/



	//무기장착 상태
	void ActionPlugin::SetWeaponState(WeaponStateType stateType)
	{

		switch (stateType)
		{
		case WeaponStateType::weaponStow_false:
			humanControLogic->weaponStow(false);
			break;

		case WeaponStateType::weaponStow_True:
			humanControLogic->weaponStow(true);
			break;

		case WeaponStateType::weaponDown:
			humanControLogic->weaponDown();
			break;

		case WeaponStateType::weaponUp:
			humanControLogic->weaponUp();
			break;
		default:
			LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[ActionPlugin][Error] WeaponStateType out of range.  WeaponState =" << (int)stateType << std::endl;
			break;
		}

	}


	//※무기변경 일어나면 WeaponPlugin.cpp 에서 무기변경 콜백받아서 모니터링에 데이터 전송	
	void ActionPlugin::SetWeaponIndex(unsigned int index)
	{
		if (gMyWeapons.size() == 0 || index >= gMyWeapons.size())
		{
			LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[ActionPlugin][Error] weapon index out of range. " << "index=" << index << ", gMyWeapons.size() =" << gMyWeapons.size() << "\n";
			return;
		}

		//weaponIndex(0~4)
		humanControLogic->selectWeapon(index);
	}




	//※ 무기내린상태에서 웅크린 자세 <-> 무릎 꿇기 자세 변경 안됨
	//HumanPosture => SITTING, JUMPING, PARACHUTING 사용불가(Q&A PS-9940)
	void ActionPlugin::SetPosture(HumanPostureType index)
	{

		HumanPosture posture = (HumanPosture)4;

		bool isPosture = true;

		switch (index)
		{
		case HumanPostureType::PRONE:
			posture = (HumanPosture)0;
			break;

		case HumanPostureType::CROUCHING:
			posture = (HumanPosture)2;
			break;

		case HumanPostureType::KNEELING:
			posture = (HumanPosture)3;
			break;

		case HumanPostureType::STANDING:
			posture = (HumanPosture)4;
			break;

		default:
			isPosture = false;
			break;
		}


		if (!isPosture)
		{
			LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[ActionPlugin][Error] PostureIndex Over.. Index:  " << (int)index << std::endl;
			return;
		}


		makVre::DtHumanControlLogic::DtPostureTransition postureTransition = DtHumanControlLogic::POSTURE_UP;
		humanControLogic->setPosture(1, posture, postureTransition);


	}
}
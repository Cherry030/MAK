#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <map>
//#include <fstream>
#include <functional>


namespace nesslab_common
{


#pragma region Constants

	const std::string CONFIG_FILE_PATH = "Config\\nesslabPluginConfig.ini";

	constexpr uint8_t PACKET_STX = 0x02;
	constexpr uint8_t PACKET_ETX = 0x03;


	// Reconnect delay in seconds after a TCP/UDP connection is lost.
	constexpr uint8_t  RECONNECT_DELAY_SEC = 2;

	//Config File(.ini)
	constexpr int		INI_INT_NOT_FOUND_DEFAULT = 0;//반환값이 UINT라 음수x
	const std::string	INI_STRING_NOT_FOUND_DEFAULT = "NULL";

	const std::string	LOG_WARN_CHANNEL_NAME = "VRIS";



#pragma endregion



//Types/Struct/Enum
#pragma region Type


	enum class WeaponType :uint8_t
	{
		RIFLE,
		PISTOL,
		GRENADE,
		LAUNCHER
	};

	//VRE
	enum class WeaponStateType :uint8_t
	{
		weaponStow_false,
		weaponStow_True,//무기보관,집어넣기
		weaponDown,
		weaponUp,
	};


	//VRE
	enum class HumanPostureType :uint8_t
	{
		PRONE,
		CROUCHING,
		KNEELING,
		STANDING,
	};


	enum class DeviceID
	{
		//VR-Engage
		Vris = 0x71,

		//인프라/세미인프라 기반 3D 자세 인식 장비
		Ipms = 0x53,
		Spms = 0x67,

		//인프라/세미인프라 기반 총기 자세 인식 장비
		Ifes = 0x55,
		Sfes = 0x69,

		//인프라/세미인프라 기반 행동 인식 장비
		Iars = 0x54,
		Sars = 0x68,

		//소형/중형 트레드밀
		Stms = 0x01,
		Mtms = 0x02,

		//매니퓰레이터
		Pmms = 0x31,
		Amms = 0x41,

		//전자탄창
		Hagh = 0x81,

		//햅틱슈트
		Hsth = 0x84,

		//기동보조
		Madh = 0x87,

		//RFID
		Rfid = 0xca,

		//심리분석
		Tits = 0x72,

		//SubVRIS
		Subvris = 0x71

	};



	//발신 Device => VRIS
	enum class VrisMsgID
	{
		//------------------ VRIS -> DEVICE ------------------

		//Haptic
		DamageEffectControlMsgID = 0x73,//피탄/피폭 효과 제어 정보
		HitInfoMsgID = 0x91,//피격 정보

		//Treadmill
		TerrainSlopeMsgID = 0x01, //지형 기울기정보

		//ManeuverAssistant(기동보조장치)	
		MobilityResistanceMsgID = 0x74,//지형 속성에 따른 기동 저항 정보 

		//TITS(심리분석)
		TraineeInfoMsgID = 0x78,

		//------------------ VRIS -> DSMS ------------------

		//Terrain
		SoilTypeMsgID = 0xa1,//지형타입 정보


		//Action/Posture
		InfraActionInfoMsgID = 0x51,//인프라기반 행동 정보
		SemiInfraActionInfoMsgID = 0x61,//세미인프라기반 행동 정보

		//RFID
		TraineeIdMsgID = 0xb1,//훈련자ID 정보

		//Treadmill
		StmsMoveMsgID = 0x11,//소형 이동 정보
		MtmsMoveMsgID = 0x21,//중형 이동 정보

		//HAGH(전자탄창)
		weaponInfoMsgID = 0x81,//총기 정보

		//TITS
		TraineeStateMsgID = 0x7a,//생체 정보(4종)

		DeviceStateMsgID = 0xD1,

		//------------------ sub VRIS -> VRIS ------------------//Sub에서 받은 장비 상태 정보
		//자세 행동 
		InfraPostureStateMsgID = 0x57,		//IARS
		SemiInfraPostureStateMsgID = 0x62,	//MAES

		//모의총기
		GunStateMsgID = 0x83,				//HAGH

		//햅틱슈트
		HapticStateMsgID = 0x84,			//HSTH

		//기동보조
		ManeuverStateMsgID = 0x87,			//MADH

		//패치디바이스
		PatchStateMsgID = 0x79				//TITS(HWDH, WPBH, 손 패치)
	};


	//발신 Device => 인프라/세미인프라 행동 인식 장비
	enum class ActionMsgID
	{
		InfraActionInfoMsgID = 0x57,//인프라기반 행동 정보 
		SemiInfraActionInfoMsgID = 0x68,//세미인프라기반 행동 정보 
	};



	//발신 Device => 인프라/세미인프라 3D 자세 인식 장비
	enum class PoseMsgID
	{
		Infra3DPoseMsgID = 0x54,//인프라기반 3D 자세 정보
		SemiInfra3DPoseMsgID = 0x65,//세미인프라기반 3D 자세 정보
	};


	//발신 Device => 인프라/세미인프라 총기 자세 인식 장비
	enum class WeaponPoseMsgID
	{
		InfraWeaponPoseMsgID = 0x5a,
		SemiInfraWeaponPoseMsgID = 0x6b,

	};


	//발신 Device => 트레드밀
	enum class TreadmillMsgID
	{
		TreadmillMoveMsgID = 0x01, //트레드밀 이동 정보
		StopAllPluginsMessageID = 0x02,//임시

	};


	//발신 Device => Hagh(전자탄창, 제어기)
	enum class HaghMsgID
	{
		WeaponFireMsgID = 0x81,//격발 정보
		WeaponReloadMsgID = 0x82//재장전 정보(임시값)

	};


	
	//행동라벨(IDD 57p)
	enum class ActionLableIndex :uint8_t
	{
		StandingShoot = 1,      // 서서 쏴
		AimedShootingStance,    // 지향사격 자세
		KneelingShoot,          // 무릎 쏴
		SittingShoot,           // 앉아 쏴
		Walking,                // 정상 걷기
		FastWalking,            // 빠른 걷기
		WalkingAimedShoot,      // 걸으며 지향사격
		RunningAimedShoot,      // 뛰며 지향사격
		RifleButtThrust,        // 찔러 총
		RifleButtStrike = 10,   // 때려 총

		SeatedGrenadeThrow,     //앉은 채 수류탄 투척
		StandingGrenadeThrow,   //서서 수류탄 투척
		ReloadDuringFire,       //사격 중 탄창 교체
		WeaponSwitchToRifle,	//소총교체(권총->소총)
		WeaponSwitchToPistol,	//권총교체(소총->권총)
		Disperse,				//산개
		AssembleOrRally,		//집합
		JoinMe,					//이리와
		IncreaseSpeed,			//증속
		WedgeFormation = 20,	//쐐기대형

		ChemicalAttack,			//가스
		FixBayonets,			//착검
		EnemyInSight,			//적발견
		QuickTime,				//신속하게
		Halt,					//정지
		TakeAKnee,				//무릎앉아
		MoveToTheProne,			//포복으로 이동
		PaceCount,				//보측
		RadiotelephoneOperatorForward,//통신병 앞으로
		HeadCount = 30,			//인원파악

		DangerArea,				//위험지역 도착
		Freeze,					//움직이지마!
		StopLookListenSmell,	//정지! 사주경계
		MessageAcknowledged,	//수신완료
		CommenceFiringOrChangeRateOfFire, //사격개시/(기관총)사격속도 조절
		ChangeDirectionOrElevation, //방향 전환
		MoveOverOrShiftFire,	//사격방향 전환
		CeaseFiring,			//사격중지
		GranadeUnderThrow,		//수류탄 하단 투척

		ActionLabelCount		// 행동라벨 개수(ActionLabelCount -1해주기)
	};

	/*
	//행동라벨(IDD 57p) 20260511 수정전
	enum class ActionLableIndex :uint8_t
	{
		StandingShoot = 1,      // 서서 쏴
		AimedShootingStance,    // 지향사격 자세
		KneelingShoot,          // 무릎 쏴
		SittingShoot,           // 앉아 쏴
		Walking,                // 걷기
		FastWalking,            // 빠른 걷기
		WalkingAimedShoot,      // 걸으며 지향사격
		RunningAimedShoot,      // 뛰며 지향사격
		RifleButtThrust,        // 찔러 총
		RifleButtStrike,        // 때려 총
		SeatedGrenadeThrow,     // 앉은 채 수류탄 투척
		StandingGrenadeThrow,   // 서서 수류탄 투척
		ReloadDuringFire,       // 사격 중 탄창 교체

		ActionLabelCount		// 행동라벨 개수(ActionLabelCount -1해주기)
	};
	
	*/


	//햅틱 슈트 피해 부위(햅틱 슈트 및 전자탄창 통신 패킷 정의 문서)
	enum class HapticHitZone :uint8_t
	{
		TorsoFront = 0x01,//상체 앞
		TorsoLeft,
		TorsoRight,
		TorsoRear,
		Head,
		Legs,
		All,

		HapticHitZoneCount//피해 부위 개수(-1해주기)
	};

	//햅틱슈트 피해 상태 값(햅틱 슈트 및 전자탄창 통신 패킷 정의 문서)
	enum class DamageState : uint8_t
	{
		Normal = 0,   // 정상
		Light,        // 경상
		Serious,      // 중상
		Dead,         // 사망

		Count         // 상태 개수
	};


#pragma pack(push,1)
	struct PacketHeader
	{
		std::uint8_t  stx = PACKET_STX;//Fixed
		std::uint8_t  deviceID = 0;
		std::uint8_t  msgID = 0;
		std::uint8_t  msgType = 0;//Reserved
		std::uint16_t length = 0;//전체 프레임길이
	};


	struct PacketTrailer
	{
		std::uint8_t etx = PACKET_ETX; //Fixed
	};

#pragma pack(pop)


	//Subscription?
	struct PacketHandler {
		std::uint8_t deviceID;
		std::uint8_t msgID;
		void* frameStorage;// Parsed frame data will be written here before calling onEvent
		std::uint16_t payloadLen;//payload Length
		std::uint16_t frameSize;//sizeof(Frame)

		std::function<void()> onEvent;//Callback Event
	};

	struct WeaponInfo
	{
		//std::string fullName = "";//ex) weapon-2|M16A2-556mm, other-4|M67 (ID+|+name)
		//std::string weaponName = "";//ex) M16A2-556mm
		

		std::string weaponName		= "";//ex) K2, M84,
		std::string munitionType	= "";//ex) 2 8 225 2 1 1 0, 
		std::string attributeName	= "";//ex) clip:weapon-2 (총기는 clip: 시작, 투척류는 resource: 시작)

		std::uint16_t weaponIndex	= 0;
		WeaponType weaponType		= WeaponType::RIFLE;
	};


#pragma endregion


#pragma region Utils

	using PacketHandlers = std::vector<PacketHandler>;

	//최소 헤더 크기
	static constexpr uint8_t PACKET_HEADER_SIZE = sizeof(PacketHeader);

	template<class Frame>
	inline std::size_t PayloadSize() {
		return sizeof(Frame) - sizeof(PacketHeader) - sizeof(PacketTrailer);
	}

	//handles에 PacketHandler저장
	template<class Frame>
	void RegisterPacketHandler(PacketHandlers& handlers, uint8_t deviceID, uint8_t msgID, Frame& storage, std::function<void(const Frame&)> callback)
	{
		if (sizeof(Frame) < sizeof(PacketHeader) + sizeof(PacketTrailer))
		{
			std::cerr << "[RegisterPacketHandler][Error] Frame size too small (must be >= header + trailer)." << std::endl;
			return;
		}

		PacketHandler packetHandler;
		packetHandler.deviceID = deviceID;
		packetHandler.msgID = msgID;

		packetHandler.frameStorage = &storage;
		packetHandler.payloadLen = static_cast<std::uint16_t>(PayloadSize<Frame>());
		packetHandler.frameSize = static_cast<std::uint16_t>(sizeof(Frame));

		if (callback) {

			Frame* packet = &storage;
			packetHandler.onEvent = [packet, cb = std::move(callback)]() {
				cb(*packet);
			};
		}

		handlers.push_back(std::move(packetHandler));
	}


	//buffer에 저장된 데이터를 handlers에 저장된 패킷으로 파싱
	void ParseReceiveBuffer(PacketHandlers& handlers, std::vector<uint8_t>& buffer);


	// Clamps value to the [min_value, max_value] range.
	template <typename T>
	T Clamp(T value, T minValue, T maxValue)
	{
		if (value < minValue) return minValue;
		if (value > maxValue) return maxValue;
		return value;
	}

#pragma endregion



}
//#include "TDMMovePlugin.h"
//
//#include "Nesslab/nesslabUdpServer.h"
//#include "Nesslab/nesslabUdpClient.h"
//#include "Nesslab/nesslabCommon.h"
//#include<math.h>
//#include<algorithm>
//
//
//using namespace makVre;
//using namespace makVrv;
//using namespace nesslab_common;
//
//
///*								메모
//
//				지형별 속도 저항 및 피로도(fatigue) 조절
//파일 경로 = C:\MAK\vrengage2.1.1b\data\simulationModelSets\VR-Engage\vrfSim\systems\movement\human.sysde
//피로도 수치 조절 => (fatigue-increase-rate-high 1.000000) -> (fatigue-increase-rate-high 0.000000)
//지형별 속도 조절 => (soil-type "paved-road")(max-speed-factor 1.000000)
//
//							캐릭터 위치
//DtVector	localLocation		= playerAttributeStore()->body().myPosition;//geoc
//DtTaitBryan localOrientation	= playerAttributeStore()->body().myOrientation;//geoc
//
//*/
//
////수류탄 투척할때 매니퓰레이터 회전각을 이용해서 yaw 조절
//float gTopoHeadingRad = 0;//0~2π
//
////캐릭터가 멈춰야 수류탄 투척이가능해서 flag 만듬
//bool  gStopMovementFlag = false;
//
//
////초실감 인터페이스 정의(v1.0)-20250520 문서확인
////TDM -> VRIS -> DSMS 
//#pragma pack(push, 1)
//struct TDMDataPacket
//{
//	PacketHeader header;
//
//	//Payload
//	float       velXKmh = 0; // km/h (x속도)
//	float       velYKmh = 0; // km/h (y속도)
//	float       oriDeg = 0; // 0~360 degree (방향)
//	float       velKmh = 0; // km/h (복합속도)
//	float       accMs2 = 0; // m/s2 (가속도)
//	float       soundDba = 0; // dBA (장비소음)
//
//	float		xRoll = 0;//X 경사도(ROLL)(-30 ~ 30)
//	float		yPitch = 0;//Y 경사도(PITCH)(-30 ~ 30)
//
//	byte		tdmOpStat = 0x00;//트레드밀 가동상태, 0x00 미가동, 0x01 전원, 0x02 모션가동, 0x03 비상정지(모션에러)
//	byte		tdmRdStat = 0x00;//트레드밀 준비상태, 0x00 준비전, 0x01 준비완료
//	byte		manipOpStat = 0x00;//매니퓰레이터 가동상태, 0x00 미가동, 0x01 전원, 0x02 모션가동, 0x03 비상정지(모션에러)
//	byte		manipRdStat = 0x00;//매니퓰레이터 준비상태, 0x00 준비전, 0x01 준비완료
//	int16_t		manipRotAng = 0;//매니퓰레이터 회전각, -180 ~ 180도
//	uint16_t	manipReactSpd = 0;//Active 매니퓰레이터 반응속도, ms (milliseconds)
//	uint16_t	manipDelayTime = 0;//Active 매니퓰레이터 지연시간, ms (milliseconds)
//
//
//	PacketTrailer trailer;
//
//	TDMDataPacket()
//	{
//		header.deviceID = 0x00;// 전방향 이동장치 => 0x01 소형, 0x02 중형(요청 예정)?
//		header.msgID	= (uint8_t)TreadmillMsgID::TreadmillMoveMsgID;// 이동 정보
//		header.msgType	= 0;//Reserved
//		header.length	= sizeof(TDMDataPacket);//0x31(49)
//	}
//
//};
//#pragma pack(pop)
//
//
//
//namespace nesslab_frontend_plugins {
//
//	namespace {
//
//		//TDM --UDP_Multicast--> VRIS, MulticastIP = 234.2.3.24, Port = 2324
//		nesslabUdpServer*	tdmUdpMcastServer = nullptr;
//		std::string			tdmMulticastIP   = "127.0.0.1";
//		int					tdmMulticastPort = -1;
//		TDMDataPacket		tdmDataPkt;
//
//
//		//VRIS --UDP_Unicast--> DSMS, Port = 13000
//		nesslabUdpClient*	dsmsUdpUcastClient = nullptr;
//		std::string			dsmsIP = "127.0.0.1";
//		int					dsmsPort = -1;
//
//		std::vector<BYTE>   tdmRecvBuffer{};
//
//		PacketHandlers packetHandlers{};
//	}
//
//
//	//CTOR
//	TDMMovePlugin::TDMMovePlugin()
//		:DtPlayerComponent("DtNesslabTDM")
//	{
//		std::cout << "[TDMMovePlugin][Trace] Constructor " << std::endl;
//
//	}
//
//
//	//DTOR
//	TDMMovePlugin::~TDMMovePlugin()
//	{
//		std::cout << "[TDMMovePlugin][Trace] Destructor " << std::endl;
//	}
//
//
//
//	bool TDMMovePlugin::initialize(makVre::DtPlayerStation* player, makVre::DtInitTable& config)
//	{
//
//		//do base class init.  Shouldn't fail
//		if (!DtPlayerComponent::initialize(player, config))
//		{
//			LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[TDMMovePlugin][Error]do base class init.  Shouldn't fail \n";
//			return false;
//		}
//		std::cout << "[TDMMovePlugin][Trace] Entering function: initialize()" << std::endl;
//
//
//		InitPlugin();
//
//
//		while (humanControLogic == nullptr)
//		{
//			humanControLogic = player->findComponent<DtHumanControlLogic>();
//			std::this_thread::sleep_for(std::chrono::milliseconds(10));
//		}
//
//
//		return true;
//	}
//
//
//	constexpr double MPS_TO_KMH = 3.6;
//	constexpr double KMH_TO_MPS = 1.0 / 3.6;
//	void TDMMovePlugin::tick(double dt)
//	{
//		//DtVector32 playerVelocity = playerAttributeStore()->body().myVelocity;//mps
//
//		
//
//
//		//Speed(mps->kmh)
//		//const float playerSpeedKmh = std::hypot(playerVelocity.y(), playerVelocity.z()) * MPS_TO_KMH;//VRF Speed UI
//		//const double playerSpeedKmh = std::sqrt(playerVelocity.x() * playerVelocity.x() + playerVelocity.y() * playerVelocity.y() + playerVelocity.z() * playerVelocity.z()) * MPS_TO_KMH;
//
//
//		//현재 실제 속도(VRE)
//		//std::cout << "playerVelocity.x = "	<< playerVelocity.x() * MPS_TO_KMH	<<"kmh"		<< std::endl;
//		//std::cout << "playerVelocity.y = "	<< playerVelocity.y() * MPS_TO_KMH	<<"kmh"		<< std::endl;
//		//std::cout << "playerVelocity.z = "	<< playerVelocity.z() * MPS_TO_KMH	<<"kmh"		<< std::endl;
//		//std::cout << "playerSpeedKmh = "	<< playerSpeedKmh					<<"kmh \n"	<< std::endl;
//
//
//
//		//packet 수신했을때 바로 처리하니까 backEnd에서 캐릭터 이동처리할때 가끔 1프레임씩 밀리는듯한? 움찔거리는 현상이 있어서 tick에서 처리하니까 되는듯?테스트 진행 중
//		humanControLogic->setCurrentHeading(gTopoHeadingRad);//Set Heading
//
//	}
//
//	void TDMMovePlugin::shutdown()
//	{
//		std::cout << "[TDMMovePlugin][Trace] Entering function: shutdown()" << std::endl;
//
//		if (netThread.joinable())
//			netThread.join();
//
//
//		if (tdmUdpMcastServer != nullptr)
//		{
//			tdmUdpMcastServer->ServerStop();
//			delete tdmUdpMcastServer;
//			tdmUdpMcastServer = nullptr;
//		}
//
//
//		if (dsmsUdpUcastClient != nullptr)
//		{
//			dsmsUdpUcastClient->Disconnect();
//			delete dsmsUdpUcastClient;
//			dsmsUdpUcastClient = nullptr;
//		}
//	}
//
//
//	void TDMMovePlugin::InitPlugin()
//	{
//
//		tdmRecvBuffer.clear();
//		packetHandlers.clear();
//		gStopMovementFlag = false;
//		gTopoHeadingRad = 0;
//
//
//		//Get ConfigFile Data
//		{
//
//			char databuf[256] = {};
//
//			//Treadmill, TDM --UDP_Multicast--> VRIS
//			GetPrivateProfileString("Treadmill_Move", "tdmMoveIP", INI_STRING_NOT_FOUND_DEFAULT.c_str(), databuf, sizeof(databuf), CONFIG_FILE_PATH.c_str());
//			tdmMulticastIP = databuf;
//			tdmMulticastPort = GetPrivateProfileInt("Treadmill_Move", "tdmMovePort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());
//
//
//			//DSMS, VRIS --UDP_Unicast--> DSMS
//			GetPrivateProfileString("DSMS", "dsmsIP", INI_STRING_NOT_FOUND_DEFAULT.c_str(), databuf, sizeof(databuf), CONFIG_FILE_PATH.c_str());
//			dsmsIP = databuf;
//			dsmsPort = GetPrivateProfileInt("Treadmill_Move", "dsmsPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());
//
//
//			if (tdmMulticastPort == INI_INT_NOT_FOUND_DEFAULT || dsmsPort == INI_INT_NOT_FOUND_DEFAULT || tdmMulticastIP == INI_STRING_NOT_FOUND_DEFAULT || dsmsIP == INI_STRING_NOT_FOUND_DEFAULT)
//				LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[TDMMovePlugin][Error] Failed to read data from INI file. Using default value.\n";
//
//
//			std::cout << "[TDMMovePlugin][Debug] tdmMulticastIP: " << tdmMulticastIP << ", tdmMulticastPort: " << tdmMulticastPort
//				<< ", dsmsIP: " << dsmsIP << ", dsmsPort: " << dsmsPort << std::endl;
//
//		}
//
//
//		//Store packet data(데이터 받는 패킷만 처리 => 파싱, 콜백 처리용)
//		{
//
//			//Weapon -> VRIS
//			//소형 이동 정보 패킷
//			RegisterPacketHandler<TDMDataPacket>(packetHandlers, (uint8_t)DeviceID::Stms, (uint8_t)TreadmillMsgID::TreadmillMoveMsgID, tdmDataPkt,
//				[this](const TDMDataPacket& packet)
//				{
//					OnTdmDataPkt(packet);
//				}
//			);
//
//			//중형 이동 정보 패킷
//			RegisterPacketHandler<TDMDataPacket>(packetHandlers, (uint8_t)DeviceID::Mtms, (uint8_t)TreadmillMsgID::TreadmillMoveMsgID, tdmDataPkt,
//				[this](const TDMDataPacket& packet)
//				{
//					OnTdmDataPkt(packet);
//				}
//			);
//		}
//
//
//		//Create and start TCP/UDP
//		{
//
//			//YSU --TCP--> VRIS
//			netThread = std::thread([this]() {
//
//				//TDM --UDP_Multicast--> VRIS
//				tdmUdpMcastServer = new nesslabUdpServer(tdmMulticastPort, nesslabUdpServer::PacketTransmissionMode::Multicast, tdmMulticastIP);
//				tdmUdpMcastServer->SetOnDataReceived(
//					[this](unsigned char* data, int len)
//					{
//						OnDataReceived(data, len);
//					});
//				tdmUdpMcastServer->ServerStart();
//
//
//				//VRIS --UDP_Unicast--> Dsms
//				dsmsUdpUcastClient = new nesslabUdpClient(dsmsPort, dsmsIP.data(), nesslabUdpClient::PacketTransmissionMode::Unicast);
//				dsmsUdpUcastClient->Connect();
//
//			});
//
//		}
//
//	}
//
//
//
//
//	/*
//	 
//		Joystick
//			Move(+1) =  16.12 kmh
//			Move(-1) = -11.28 kmh * BACKWARD_SCALE = 16.12 kmh
//
//			Strafe(+1,-1) = 9.73 kmh * STRAFE_SCALE = 16.12 kmh
//
//	*/
//
//	//축별 스케일 계수
//	constexpr float STRAFE_SCALE	= 1.6556f;  // 좌우
//	constexpr float BACKWARD_SCALE	= 1.428646f;// 뒤
//
//	void TDMMovePlugin::PlayerMove(double val)
//	{
//
//		// 뒤로갈 때만 보정
//		if (val < 0) val *= BACKWARD_SCALE;
//	
//		std::cout << "MoveJoystickValue: " << val << std::endl;
//		humanControLogic->sendJoystickCommand("Human Movement", "Move", val);
//	}
//
//
//	void TDMMovePlugin::PlayerStrafe(double val)
//	{
//
//		val *= STRAFE_SCALE;//or 1.666666
//		std::cout << "StrafeJoystickValue: " << val << std::endl;
//		humanControLogic->sendJoystickCommand("Human Movement", "Strafe", val);
//
//	}
//
//
//
//
//
//	/*
//								※※※매니퓰레이터 회전 메모※※※
//
//		- 매니퓰레이터 회전값 => 소수점 2자리까지 표시하기위해서 트레드밀에서 보낼때 100곱해서 송신
//		ex) 26.15f => 2615
//
//		- 트레드밀은 북쪽방향이 정면이면 매니퓰레이터는 동쪽이 정면(매니퓰레이터는 왼쪽방향이 +, 오른쪽방향이 -)
//
//		- 트레드밀,MAK 정면 = 북쪽, 왼쪽 -, 오른쪽 + (0 ~ 360)
//		- 매니퓰레이터 정면 = 동쪽,	왼쪽 +, 오른쪽 - (매니퓰레이터 정면 = Mak에서 90도)(-180 ~ 180)
//
//	*/
//	static float ConvertManipAngle(float oldAngle) {
//
//		//소수점 2자리
//		oldAngle /= 100.0f;
//
//		//Mak 회전각이랑 같게 매니퓰레이터 회전각 변환
//		//ex) 0 -> 90, 90 -> 360, -90 -> 180, 180 -> 270 (매니퓰레이터 -> Mak)
//		float newAngle = oldAngle - 90;
//		if (newAngle < 0)
//			newAngle += 360;
//
//		newAngle = 360 - newAngle;
//
//
//		return newAngle;
//	}
//
//
//
//
//	//constexpr float KMH_TO_MPS = 1000.0f / 3600.0f;
//
//	void TDMMovePlugin::OnTdmDataPkt(const TDMDataPacket& packet)
//	{
//		//std::cout << "[TDMMovePlugin][Trace] OnTdmDataPkt() \n";
//
//		//매니퓰레이터 회전각 변경(북쪽방향 정면기준, 오른쪽방향이 + 되도록 변환, 0~360)
//		float manipRotAng = ConvertManipAngle(packet.manipRotAng);
//
//
//
//		//TEST
//		//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
//		//VRF maxspeed = 32.241 정도
//		//float tdmVelX = packet.velXKmh;//kmh
//		//float tdmVelY = packet.velYKmh;//kmh
//
//				//float tdmVelX = packet.velXKmh * KMH_TO_MPS;
//				//float tdmVelY = packet.velYKmh * KMH_TO_MPS;
//				//std::cout << "recv data moveValue => " << tdmVelY << ", strafeValue=> " << tdmVelX << std::endl;
//
//				//
//				///* => 테스트용
//				//*/
//
//				//humanControLogic->setCurrentHeading(0);//Set Heading
//
//				//PlayerMove(tdmVelY);
//				//PlayerStrafe(tdmVelX);
//
//				////현재 캐릭터 속도(kmh)
//				//DtVector32 playerVelocity = playerAttributeStore()->body().myVelocity;
//				//std::cout << "playerVelocity.y = " << playerVelocity.y() * 3.6f << std::endl;	//현재 실제 속도
//				//std::cout << "playerVelocity.z = " << playerVelocity.z() * 3.6f << std::endl;	//현재 실제 속도
//
//				//const float playerSpeedKmh = std::hypot(playerVelocity.y(), playerVelocity.z()) * 3.6f;
//				//std::cout << "playerSpeedKmh = " << playerSpeedKmh << std::endl;	//현재 실제 속도
//				//std::cout << "\n";
//				//return;
//		//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
//		
//		//VRF maxspeed = 32.241 정도
//		float tdmVelX = packet.velXKmh;//kmh
//		float tdmVelY = packet.velYKmh;//kmh
//
//		SetCharacterMovement(manipRotAng, tdmVelX, tdmVelY);
//		SendDataToDsms();
//
//	}
//
//
//
//
//	void TDMMovePlugin::OnDataReceived(unsigned char* data, int len)
//	{
//		//std::cout << "[TDMMovePlugin][Debug] OnDataReceived() \n";
//
//		//Sub_Vris에서 트레드밀로 회전값 보내는거 리턴처리(UDP_Multicast)
//		BYTE msgID = data[2];
//		if (msgID == 2) return;
//
//
//		//std::cout << "[TDMMovePlugin][DEBUG] Entering function: ReceiveData() " << std::endl;
//		////std::cout << "[트레드밀 수신] 데이터 : ";
//		//std::cout << "[Treadmill]  Received Data : ";
//		//std::cout << "Data Length : " << len << ", Data : ";
//		//std::cout << std::hex << std::uppercase;
//		//for (int i = 0; i < len; ++i)
//		//	std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
//		//std::cout << std::dec << std::endl;
//
//
//		tdmRecvBuffer.insert(tdmRecvBuffer.end(), data, data + len);
//
//		ParseReceiveBuffer(packetHandlers, tdmRecvBuffer);
//
//
//	}
//
//
//
//
//
//
//	void TDMMovePlugin::SendDataToDsms()
//	{
//
//		const size_t size = sizeof(TDMDataPacket);
//		unsigned char sendData[size];
//		std::memcpy(sendData, &tdmDataPkt, size);
//		//unsigned char* bytePtr = reinterpret_cast<unsigned char*>(&tdmDataPkt);
//
//
//		//0x01 (소형 트레드밀), 0x02(중형 트레드밀)
//		BYTE deviceID = sendData[1];		
//		if (deviceID == 0x01)
//		{
//			//소형 이동데이터 메시지
//			sendData[2] = (uint8_t)VrisMsgID::StmsMoveMsgID;
//		}
//		
//		else if (deviceID == 0x02)
//		{
//
//			//중형 이동데이터 메시지
//			sendData[2] = (uint8_t)VrisMsgID::MtmsMoveMsgID;
//		}
//
//
//		//device ID => 0x71(VRIS)
//		sendData[1] = (uint8_t)DeviceID::Vris;
//
//
//		BOOL result = dsmsUdpUcastClient->SendData(sendData, size);
//
//		if (result)
//		{
//			/*
//			
//			std::cout << "[트레드밀 송신] 데이터 : ";
//			//std::cout << "data length : " << len << ", data : ";
//			std::cout << std::hex << std::uppercase;
//			for (int i = 0; i < size; ++i)
//				std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(sendData[i]) << " ";
//			std::cout << std::dec << std::endl;
//
//			
//			cout << "[트레드밀 송신 데이터] STX :" << (int)TDMDataPacket._STX
//				<< ", DEVICE_ID : " << (int)TDMDataPacket.DEVICE_ID
//				<< ", MSG_ID : " << (int)TDMDataPacket.MSG_ID
//				<< ", MSG_TYPE : " << (int)TDMDataPacket.MSG_TYPE
//				<< ", LENGTH : " << (int)TDMDataPacket.LENGTH
//				<< ", x속도 : " << TDMDataPacket.VEL_X_KMH
//				<< ", Y속도 : " << TDMDataPacket.VEL_Y_KMH
//				<< ", 방향 : " << TDMDataPacket.ORI_DEG
//				<< ", 복합속도 : " << TDMDataPacket.VEL_KMH
//				<< ", 가속도 : " << TDMDataPacket.ACC_MS2
//				<< ", 소음 : " << TDMDataPacket.SOUND_DBA
//
//				<< ", X경사도 : " << TDMDataPacket.xROLL
//				<< ", Y경사도 : " << TDMDataPacket.yPITCH
//				<< ", 트레드밀 가동상태 : " << (int)TDMDataPacket.tdmOpStat
//				<< ", 트레드밀 준비상태 : " << (int)TDMDataPacket.tdmRdStat
//				<< ", 매니퓰레이터 가동상태 : " << (int)TDMDataPacket.manipOpStat
//				<< ", 매니퓰레이터 준비상태 : " << (int)TDMDataPacket.manipRdStat
//				<< ", 매니퓰레이터 회전각 : " << (int)TDMDataPacket.manipRotAng
//				<< ", Active 매니퓰레이터 반응속도 : " << (int)TDMDataPacket.manipReactSpd
//				<< ", Active 매니퓰레이터 지연시간 : " << (int)TDMDataPacket.manipDelayTime
//				<< ", ETX : " << (int)TDMDataPacket._ETX
//				<< endl;
//				*/
//
//
//		}
//		else
//		{
//			//std::cerr << "[TDMMovePlugin][Error] Failed to send data to the dsms ";
//		}
//
//
//	}
//
//
//	
//	//kmh <-> mps
//	//constexpr float KMH_TO_MPS = 1000.0f / 3600.0f;
//	//constexpr float MPS_TO_KMH = 3600.0f / 1000.0f;
//
//
//	//입력값당 km/h 맞춰주기위해서 사용 
//	constexpr float JOYSTICK_SCALE = 16.122f;
//	
//
//	//속도를 0으로 간주할 최소 값
//	constexpr float MIN_SPEED_KMH = 0.05;
//
//
//	//VRF 최대속도
//	constexpr float VRF_MAX_SPEED_KMH = 32.30;
//
//
//	float			speedScale	= 1.0f;//속도 차이 비율
//	constexpr float SPEED_SCALE_MIN = 0.80f;
//	constexpr float SPEED_SCALE_MAX = 1.50f;
//	
//	constexpr float	ALPHA			= 0.3f; // 업데이트 속도
//	constexpr float	STEP_CLAMP		= 0.10f;// 프레임당 변화 제한(±10%)
//	constexpr float JOYSTICK_LIMIT	= 2.0f;	// 조이스틱 Clamp값
//
//
//	//static int tdmTickCount = 0;
//
//	//tdm => 트레드밀 장비 기준 북쪽(고정), local => 훈련자 기준 정면
//	void TDMMovePlugin::SetCharacterMovement(float headingDeg, const float& tdmVelXkmh, const float& tdmVelYkmh)
//	{
//		//const double tdmSpeedKmh = std::sqrt(tdmVelXkmh * tdmVelXkmh + tdmVelYkmh * tdmVelYkmh);
//
//
//		//std::cout << "tdmVelXkmh: " << tdmVelXkmh << "kmh, tdmVelYkmh: " << tdmVelYkmh << std::endl;
//
//
//		//std::cout << "[Treadmill][Debug] headingDeg: " << headingDeg
//		//	<< ", worldSpeedX: " << worldSpeedXKmh << "km / h, worldSpeedY : " << worldSpeedYKmh << "km / h \n";
//
//		//if (tdmTickCount >= 1)
//		//	return;
//		//++tdmTickCount;
//
//		//Heading => Degree(0~360) -> radian(0~2π)
//		const float headingRad = headingDeg * (M_PI / 180.0f);
//		humanControLogic->setCurrentHeading(headingRad);//Set Heading
//
//		gTopoHeadingRad = headingRad;
//
//
//		//const double tdmSpeedKmh = std::sqrt(tdmVelXkmh * tdmVelXkmh + tdmVelYkmh * tdmVelYkmh) * MPS_TO_KMH;
//		const double tdmSpeedKmh = std::sqrt(tdmVelXkmh * tdmVelXkmh + tdmVelYkmh * tdmVelYkmh);
//
//
//
//
//		////test
//		//return;
//
//
//
//
//		//Check Speed 0 and gStopMovementFlag
//		const float tdmVelMax = std::max(std::fabs(tdmVelXkmh), std::fabs(tdmVelYkmh));
//		if (tdmVelMax < MIN_SPEED_KMH || gStopMovementFlag) {
//			PlayerMove(0.0f);
//			PlayerStrafe(0.0f);
//			return;
//		}
//
//
//		//tdm(world) -> local (km/h) [localX = strafe, localY = move]	
//		//2D 회전 행렬(2D Rotation Matrix),시계방향(CW) 기준 회전 변환
//		float cosAngle = cosf(headingRad);
//		float sinAngle = sinf(headingRad);
//		float localVelXkmh = cosAngle * tdmVelXkmh - sinAngle * tdmVelYkmh;//localX_kmh
//		float localVelYkmh = sinAngle * tdmVelXkmh + cosAngle * tdmVelYkmh;//localY_kmh
//		//std::cout << "[Treadmill][Debug] localSpeedX: " << localSpeedXkmh << "km/h, localSpeedY: " << localSpeedYkmh << "km/h" << std::endl;
//
//
//		//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
//		
//		//VRF 속도 => 16.12 x localMaxSpeed(지배축)kmh
//
//
//		//maxSpeedScale
//		const float targetSpeedKmh   = std::hypot(tdmVelXkmh, tdmVelYkmh);//평면기준 목표 속력(월드 벡터 크기)
//		const float localMaxSpeedKmh = std::max(std::fabs(localVelXkmh), std::fabs(localVelYkmh)); //지배축(두 값의 절대값 중 최대값)
//		float maxSpeedScale			 = (localMaxSpeedKmh > MIN_SPEED_KMH) ? (targetSpeedKmh / localMaxSpeedKmh) : 1.0f;//지배축 보정 스케일
//
//
//
//		/*
//			localSpeedXkmh / kJoystickScale =>   km/h -> 조이스틱 입력값 1:1 
//
//			ex)
//			localX = 3, localY = 4  => VRF는 MAX값을 기준으로 speed 출력 => 4kmh
//			targetSpeedKmh = 5      => 목표 속도
//			localMaxSpeedKmh = max(3,4) = 4
//			maxSpeedScale = 1.25
//
//			(4/16.12) * (5/4) * 1
//
//
//			speedScale => 실제 속도 피드백 기반 보정
//
//		*/
//
//
//		
//		//Joystick Move,Strafe 둘 중에 하나의값을 targetSpeedKmh로 맞추기
//		float joyStrafe = (localVelXkmh / JOYSTICK_SCALE) * maxSpeedScale * speedScale;
//		float joyMove   = (localVelYkmh / JOYSTICK_SCALE) * maxSpeedScale * speedScale;
//
//		joyMove   = Clamp(joyMove, -JOYSTICK_LIMIT, JOYSTICK_LIMIT);
//		joyStrafe = Clamp(joyStrafe, -JOYSTICK_LIMIT, JOYSTICK_LIMIT);
//
//		PlayerMove(joyMove);
//		PlayerStrafe(joyStrafe);
//
//		
//		
//		
//
//		//현재 캐릭터 속도
//		DtVector32 playerVelocity = playerAttributeStore()->body().myVelocity;//mps
//		//Speed(mps->kmh)
//		//const float playerSpeedKmh = std::hypot(playerVelocity.y(), playerVelocity.z()) * MPS_TO_KMH;//VRF Speed UI
//		const double playerSpeedKmh = std::sqrt(playerVelocity.x() * playerVelocity.x() + playerVelocity.y() * playerVelocity.y() + playerVelocity.z() * playerVelocity.z()) * MPS_TO_KMH;
//		//std::cout << "playerSpeedKmh: " << playerSpeedKmh << "kmh \n";
//		
//		// 목표/실제 비율
//		float speedRatio = 1.0f;
//		if (playerSpeedKmh > MIN_SPEED_KMH)
//			speedRatio = targetSpeedKmh / playerSpeedKmh;
//
//
//		//speedScaleClamped, 프레임당 변화 제한
//		speedRatio = std::max(1.0f - STEP_CLAMP, std::min(1.0f + STEP_CLAMP, speedRatio));//kStepClamp => (±10%)
//
//		
//		
//		
//		/*
//		SpeedScale = 현재까지 누적된 스케일 값(이전 프레임값)
//		speedRatio = 이번 프레임에 계산한 원하는 비율
//		kAlpha = 따라가는 속도(값이 커지면 빨리 반응하는대신 값이 흔들림, 낮으면 느리지만 안정적)
//		
//		speedRatio 그대로쓰면 여러 영향으로 값이 흔들릴 수 있어서 밑에 계산식 사용
//
//		지수이동 평균으로 부드럽게 업데이트
//		*/
//		speedScale = (1.0f - ALPHA) * speedScale + ALPHA * speedRatio;
//		
//
//		speedScale = Clamp(speedScale, SPEED_SCALE_MIN, SPEED_SCALE_MAX);
//
//		if (tdmTickCount > 10)
//		{
//			tdmTickCount = 0;
//
//			std::cout << "[Treadmill][Debug] targetSpeed: " << targetSpeedKmh//평면 목표 속력
//				<< ", playerSpeedKmh: " << playerSpeedKmh	//현재 캐릭터 속도
//				//<< ", SpeedScale=" << speedScale << "\n";//속도 스케일
//				<< ", SpeedScale: " << speedScale //속도 스케일
//			    << ", tdmSpeedKmh: " << tdmSpeedKmh << std::endl;
//		
//		
//		}
//		else
//			tdmTickCount++;
//
//	}
//}
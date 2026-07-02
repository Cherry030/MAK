#include "MyHumanMovementActuator.h"

#include "Nesslab/nesslabUdpServer.h"
#include "Nesslab/nesslabUdpClient.h"
#include "Nesslab/nesslabTcpServer.h"
#include "Nesslab/StopAllPluginsMessage.h"

#include <cmath>

using namespace nesslab_common;

/*

				지형별 속도 저항 및 피로도(fatigue) 조절
파일 경로 = C:\MAK\vrengage2.1.1b\data\simulationModelSets\VR-Engage\vrfSim\systems\movement\human.sysde
피로도 수치 조절 => (fatigue-increase-rate-high 1.000000) -> (fatigue-increase-rate-high 0.000000)
지형별 속도 조절 => (soil-type "paved-road")(max-speed-factor 1.000000)

*/

//이동 멈춰야 차량탑승가능
//vreHumanMovementActuator.h = > tick() 주석에 최대속도표시되어있음
//최대 속도를 9.0 m / s = > 32.4kmh


#pragma pack(push, 1)
struct TDMDataPacket
{
	PacketHeader header;

	//Payload
	float       velXKmh = 0; // km/h (x속도)
	float       velYKmh = 0; // km/h (y속도)
	float       oriDeg	= 0; // 0~360 degree (방향)
	float       velKmh	= 0; // km/h (복합속도)
	float       accMs2	= 0; // m/s2 (가속도)
	float       soundDba = 0; // dBA (장비소음)

	float		xRoll	= 0;//X 경사도(ROLL)(-30 ~ 30)
	float		yPitch	= 0;//Y 경사도(PITCH)(-30 ~ 30)

	uint8_t		tdmOpStat	= 0x00;//트레드밀 가동상태, 0x00 미가동, 0x01 전원, 0x02 모션가동, 0x03 비상정지(모션에러)
	uint8_t		tdmRdStat	= 0x00;//트레드밀 준비상태, 0x00 준비전, 0x01 준비완료
	uint8_t		manipOpStat = 0x00;//매니퓰레이터 가동상태, 0x00 미가동, 0x01 전원, 0x02 모션가동, 0x03 비상정지(모션에러)
	uint8_t		manipRdStat = 0x00;//매니퓰레이터 준비상태, 0x00 준비전, 0x01 준비완료

	int16_t		manipRotAng		= 0;//매니퓰레이터 회전각, -180 ~ 180도
	uint16_t	manipReactSpd	= 0;//Active 매니퓰레이터 반응속도, ms (milliseconds)
	uint16_t	manipDelayTime	= 0;//Active 매니퓰레이터 지연시간, ms (milliseconds)

	PacketTrailer trailer;

	TDMDataPacket()
	{
		header.deviceID = 0x00;// 전방향 이동장치 => 0x01 소형, 0x02 중형(요청 예정)?
		header.msgID	= (uint8_t)TreadmillMsgID::TreadmillMoveMsgID;// 이동 정보
		header.msgType	= 0;//Reserved
		header.length	= sizeof(TDMDataPacket);//0x31(49)
	}

};
#pragma pack(pop)



#pragma pack(push, 1)
struct StopAllPluginsRequestPacket
{
	PacketHeader header;

	bool stopRequested = false;

	PacketTrailer trailer;

	StopAllPluginsRequestPacket()
	{

		header.deviceID = 0x00;// 전방향 이동장치 => 0x01 소형, 0x02 중형(요청 예정)?
		header.msgID	= (uint8_t)TreadmillMsgID::StopAllPluginsMessageID;//임시값
		header.msgType	= 0;//Reserved
		header.length	= sizeof(StopAllPluginsRequestPacket);

	}
};
#pragma pack(pop)


namespace {

	//TDM --UDP_Multicast--> VRIS, MulticastIP = 234.2.3.24, Port = 2324
	nesslabUdpServer*	tdmUdpMcastServer = nullptr;
	std::string			tdmMulticastIP = "127.0.0.1";
	int					tdmMulticastPort = -1;
	TDMDataPacket		tdmDataPkt;

	//VRIS --UDP_Unicast--> DSMS, Port = 13000
	nesslabUdpClient*	dsmsUdpUcastClient = nullptr;
	std::string			dsmsIP = "127.0.0.1";
	int					dsmsPort = -1;


	//TDM --TCP--> VRIS, 훈련 일시정지 패킷
	nesslabTcpServer* tdmTcpServer = nullptr;
	int				  tdmPort = -1;
	StopAllPluginsRequestPacket stopPluginsPkt;

}



void MyHumanMovementActuator::InitPlugin()
{

	//Get ConfigFile Data
	{
		char databuf[256] = {};

		//Treadmill, TDM --UDP_Multicast--> VRIS
		GetPrivateProfileString("Treadmill_Move", "tdmMoveIP", INI_STRING_NOT_FOUND_DEFAULT.c_str(), databuf, sizeof(databuf), CONFIG_FILE_PATH.c_str());
		tdmMulticastIP		= databuf;
		tdmMulticastPort	= GetPrivateProfileInt("Treadmill_Move", "tdmMovePort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());

		

		tdmPort = 2325;//임시값
	}


	//Store packet data(데이터 받는 패킷만 처리 => 파싱, 콜백 처리용)
	{
				
		//소형 트레드밀 -> 이동 정보 패킷
		RegisterPacketHandler<TDMDataPacket>(packetHandlers, (uint8_t)DeviceID::Stms, (uint8_t)TreadmillMsgID::TreadmillMoveMsgID, tdmDataPkt,
			[this](const TDMDataPacket& packet)
			{
				OnTdmDataPkt(packet);
			}
		);

		//중형 트레드밀 -> 이동 정보 패킷
		RegisterPacketHandler<TDMDataPacket>(packetHandlers, (uint8_t)DeviceID::Mtms, (uint8_t)TreadmillMsgID::TreadmillMoveMsgID, tdmDataPkt,
			[this](const TDMDataPacket& packet)
			{
				OnTdmDataPkt(packet);
			}
		);



		//소형 트레드밀 -> 이동 정보 패킷
		RegisterPacketHandler<StopAllPluginsRequestPacket>(packetHandlers, (uint8_t)DeviceID::Stms, (uint8_t)TreadmillMsgID::StopAllPluginsMessageID, stopPluginsPkt,
			[this](const StopAllPluginsRequestPacket& packet)
			{
				OnStopAllPluginsPkt(packet);
			}
		);

		//중형 트레드밀 -> 훈련 일시정지 패킷
		RegisterPacketHandler<StopAllPluginsRequestPacket>(packetHandlers, (uint8_t)DeviceID::Mtms, (uint8_t)TreadmillMsgID::StopAllPluginsMessageID, stopPluginsPkt,
			[this](const StopAllPluginsRequestPacket& packet)
			{
				OnStopAllPluginsPkt(packet);
			}
		);

	}


	//Create and start TCP/UDP
	{

		netThread = std::thread([this]() {

			//TDM --UDP_Multicast--> VRIS
			tdmUdpMcastServer = new nesslabUdpServer(tdmMulticastPort, nesslabUdpServer::PacketTransmissionMode::Multicast, tdmMulticastIP);
			tdmUdpMcastServer->SetOnDataReceived(
				[this](unsigned char* data, int len)
				{
					OnDataReceived(data, len);
				});
			tdmUdpMcastServer->ServerStart();


			//TDM --TCP-->VRIS
			tdmTcpServer = new nesslabTcpServer(tdmPort);
			tdmTcpServer->SetOnDataReceived(
				[this](const std::string& client, const uint8_t* data, int len)
				{
					OnStopDataReceived(client, data, len);
				});

			tdmTcpServer->ServerStart();

		});


		//VRIS --UDP_Unicast--> Dsms
		dsmsUdpUcastClient = new nesslabUdpClient(dsmsPort, dsmsIP.data(), nesslabUdpClient::PacketTransmissionMode::Unicast);
		dsmsUdpUcastClient->Connect();

	}

}




//캐릭터 생성
MyHumanMovementActuator::MyHumanMovementActuator(const DtString& name, DtLocalObject* owner,
	DtSimulationServices* simManager, DtComponentDescriptor* desc, DtReaderWriterRegistry* parentRegistry)
	:DtVreHumanMovementActuator(name,owner,simManager,desc,parentRegistry)
{
	std::cout << "[MyHumanMovementActuator][Trace] Constructor \n";



	//테스트 진행 중 
	{
		// => vre객체가 여러개가있을때 내 sim객체를 확인하기 어려워서 추가
		// => 이름은 10자리만가능한듯 
		// ex) VreSimlationEngine=> VreSimlati, VRF에서도 캐릭터 이름 변경됨
		owner->setNextFrameObjectName("VreSim");
	}

	makVre::DtVreMessageManager::instance().addHandler(makVre::StopAllPluginsMessage::theType(),
		makVre::DtVreMessageDelegate(this, &MyHumanMovementActuator::HandleStopPluginMessage));
	
}


MyHumanMovementActuator::~MyHumanMovementActuator()
{
	std::cout << "[MyHumanMovementActuator][Trace] Destructor \n";

	makVre::DtVreMessageManager::instance().removeHandler(makVre::StopAllPluginsMessage::theType(),
		makVre::DtVreMessageDelegate(this, &MyHumanMovementActuator::HandleStopPluginMessage));


	if (netThread.joinable())
		netThread.join();


	if (tdmUdpMcastServer != nullptr)
	{
		tdmUdpMcastServer->ServerStop();
		delete tdmUdpMcastServer;
		tdmUdpMcastServer = nullptr;
	}


	if (dsmsUdpUcastClient != nullptr)
	{
		dsmsUdpUcastClient->Disconnect();
		delete dsmsUdpUcastClient;
		dsmsUdpUcastClient = nullptr;
	}

	
	if (tdmTcpServer != nullptr)
	{
		tdmTcpServer->ServerStop();
		delete tdmTcpServer;
		tdmTcpServer = nullptr;
	}

}


bool MyHumanMovementActuator::init()
{
	if (!DtVreHumanMovementActuator::init())
		return false;

	std::cout << "[MyHumanMovementActuator][Trace] init \n";

	InitPlugin();


	return true;
}

const char* MyHumanMovementActuator::type() const
{
	return makVre::DtVreHumanMovementActuatorType;
}




DtAnalogPortData speedPortData;//speed
DtAnalogPortData directionPortData;//direction

//Test
#include "vrfobjcore/vrfMovingObjectStateRepository.h"
#include "vrfobjparam/humanMovementActuatorDescriptor.h"


//이동중에 차량 탑승불가(속도가 0일때는 탑승가능)
void MyHumanMovementActuator::tick()
{
	/*
	//maxSpeed
	DtVrfMovingObjectStateRepository* movingSR = localStateRepository()->as<DtVrfMovingObjectStateRepository>();
	double maxSpeed = -1;
	double maxReverseSpeed = -1;
	if (movingSR != nullptr)
	{
		using namespace std;

		//maxSpeed = ;
		//maxReverseSpeed = ;
		std::cout << "\n";
		cout << "currentSpeed" << movingSR->currentSpeed() << endl;
		cout << "forwardSpeed" << movingSR->forwardSpeed() << endl;
		cout << "localSpeed" << movingSR->localSpeed() << endl;
		cout << "worldSpeed" << movingSR->worldSpeed() << endl;
		cout << "maxSpeed => " << movingSR->maxSpeed() << endl;
		cout << "maxReverseSpeed => " << movingSR->maxReverseSpeed() << endl;
		cout << "maxSlope => " << movingSR->maxSlope() << endl;
		cout << "currentPitch => " << movingSR->currentPitch() << endl;
		cout << "pitch => " << movingSR->pitch() << endl;


		movingSR->terrainAttachedTo

		std::cout << "myPercentOfMaxSpeed => " << *myPercentOfMaxSpeed << std::endl;
		std::cout << "calculateMaxSpeed => " << calculateMaxSpeed() << std::endl;
		std::cout << "dropSpeed => " << dropSpeed() << std::endl;
		std::cout << "maxUprightTerrainPitch => " << maxUprightTerrainPitch() << std::endl;
		std::cout << "myMutablePitch => " << myMutablePitch << std::endl;
		std::cout << "myMutablePitchValid => " << myMutablePitchValid << std::endl;
		//std::cout << "myMutablePitchValid => " << myDesiredStaticPostureInputPort << std::endl;
		std::cout << "\n";


	if (myHumanMovementActuatorDescriptor)
		std::cout << "maxUprightTerrainPitch => " << myHumanMovementActuatorDescriptor->maxUprightTerrainPitch() << std::endl;



		//movingSR->setCurrentHeading
	}
	*/

	




	if (!stopRequested)
	{

		//사용하는 이유 => Engage 풀었을때 backend에서 아래부분 잡고있으면 차량 탑승할때 이동이 불가능해보임 VRF에서 이동 명령내렸을때(task) 이동안함(제자리에 있음 속도0인것처럼), 속도0일때도 동작x
		//아니면 속도가0일때만 처리해줘도 괜찮을듯? 아니면 나중에 UDP 연결끊긴거 확인하는부분에서 해줘도 괜찮을듯 1초에 한번씩이라던지
		if (isPlayerControlled("Human")) //return false;
		{

			//데이터 수신말고 tick에서 처리하는 이유 
			// => 수신받는쪽에서 처리하면 마우스영향도 많이받고 가끔 부자연스러운 부분이 보임
			//트레드밀 장비 사용하면 true처리 => 평소에는 키보드사용하기위해서
			if (isTreadmillActive)
			{

				// 엔티티가 원하는 이동 방향(radian)(0 ~ 2*pi), 실제 캐릭터 heading값이 변경x		
				if (myMovementDirectionInputPort)
				{

					directionPortData.setValue(myDesiredMovementDirection);
					myMovementDirectionInputPort->receiveData(&directionPortData);
				}


				//엔티티의 원하는 속도(m/s)(0이상)
				if (mySpeedInputPort)
				{
					speedPortData.setValue(myDesiredSpeed);
					mySpeedInputPort->receiveData(&speedPortData);
				}
			}
		}
		else
		{
			myDesiredMovementDirection = 0;
			myDesiredSpeed = 0;
		}
	}


	//기존 tick
	DtVreHumanMovementActuator::tick();
	
}




DtSimComponent* MyHumanMovementActuator::creator(const DtString& name, DtLocalObject* owner, DtSimulationServices* simManager, DtComponentDescriptor* desc, DtReaderWriterRegistry* parentRegistry)
{
	std::cout << "[MyHumanMovementActuator][Trace] creator \n";

	//return nullptr;
	return new MyHumanMovementActuator(name, owner, simManager, desc, parentRegistry);
}



/*
							※※※매니퓰레이터 회전 메모※※※

	- 매니퓰레이터 회전값 => 소수점 2자리까지 표시하기위해서 트레드밀에서 보낼때 100곱해서 송신
	  ex) 26.15f => 2615

	- 트레드밀은 북쪽방향이 정면이면 매니퓰레이터는 동쪽이 정면(매니퓰레이터는 왼쪽방향이 +, 오른쪽방향이 -)

	- 트레드밀,MAK 정면 = 북쪽, 왼쪽 -, 오른쪽 + (0 ~ 360)
	- 매니퓰레이터 정면 = 동쪽,	왼쪽 +, 오른쪽 - (매니퓰레이터 정면 = Mak에서 90도)(-180 ~ 180)

*/


//회전각 변환(매니퓰레이터 -> MAK)
float MyHumanMovementActuator::ConvertManipAngleToMak(float oldAngle) {

	//소수점 2자리를 표시하기위해서
	oldAngle /= 100.0f;

	//Mak 회전각이랑 같게 매니퓰레이터 회전각 변환
	//ex) 0 -> 90, 90 -> 360, -90 -> 180, 180 -> 270 (매니퓰레이터 -> Mak)
	float newAngle = oldAngle - 90;
	if (newAngle < 0)
		newAngle += 360;

	newAngle = 360 - newAngle;

	return newAngle;
}


/*
							※※※ 속도 관련 메모※※※

	VRF maxspeed = 9.0m/s(32.4kmh)
	- vreHumanMovementActuator.h => tick() 주석에 최대속도표시되어있음
	- TODO: lua에서도 확인가능했던것같은데 나중에 시간되면 찾아보기(+조절도가능한지)

*/
void MyHumanMovementActuator::OnTdmDataPkt(const TDMDataPacket& packet)
{
	//트레드밀 장비 사용
	isTreadmillActive = true;


	//Manipulator -> mak
	float manipRotAng = ConvertManipAngleToMak(packet.manipRotAng);


	float tdmVelXkmh = packet.velXKmh;
	float tdmVelYkmh = packet.velYKmh;

	SetCharacterMovement(manipRotAng, tdmVelXkmh, tdmVelYkmh);
	SendDataToDsms();

}

void MyHumanMovementActuator::OnDataReceived(unsigned char* data, int len)
{

	//연세대 -> SubVris -> TDM 으로 가는 스켈레톤 데이터 보내는거 리턴처리(UDP_Multicast)
	BYTE msgID = data[2];
	if (msgID == 2) return;

	tdmUdpRecvBuffer.insert(tdmUdpRecvBuffer.end(), data, data + len);

	ParseReceiveBuffer(packetHandlers, tdmUdpRecvBuffer);
}


void MyHumanMovementActuator::OnStopDataReceived(const std::string& client, const uint8_t* data, int len)
{
	std::cout << "[MyHumanMovementActuator][Trace] OnStopDataReceived \n";

	tdmTcpRecvBuffer.insert(tdmTcpRecvBuffer.end(), data, data + len);

	ParseReceiveBuffer(packetHandlers, tdmTcpRecvBuffer);

}

void MyHumanMovementActuator::OnStopAllPluginsPkt(const StopAllPluginsRequestPacket& packet)
{
	std::cout << "[MyHumanMovementActuator][TRACE] OnStopAllPluginsPkt \n";


	
	stopRequested = packet.stopRequested;


	//VRE 캐릭터 속도0
	{
		myDesiredMovementDirection = 0;
		myDesiredSpeed = 0;

		if (myMovementDirectionInputPort)
		{

			directionPortData.setValue(myDesiredMovementDirection);
			myMovementDirectionInputPort->receiveData(&directionPortData);
		}


		//엔티티의 원하는 속도(m/s)(0이상)
		if (mySpeedInputPort)
		{
			speedPortData.setValue(myDesiredSpeed);
			mySpeedInputPort->receiveData(&speedPortData);
		}
	}


	SendVreMessage();

}


//km/h -> m/s
constexpr double KMH_TO_MPS		= 1.0 / 3.6;
constexpr double MIN_SPEED_KMH	= 0.01;
constexpr double twoPi			= 2.0 * M_PI;


void MyHumanMovementActuator::normalizeDirection0To2Pi(double& directionRad)
{
	if (directionRad < 0.0)
		directionRad += twoPi;
	else if (directionRad >= twoPi)
		directionRad -= twoPi;
}


makVre::DtVreMessageResult MyHumanMovementActuator::HandleStopPluginMessage(makVre::DtVreMessage* msg)
{
	std::cout << "[MyHumanMovementActuator][TRACE] HandleCustomMessage \n";

	//makVre::CustomMessage* customMsg = dynamic_cast<makVre::CustomMessage*>(msg);
	makVre::StopAllPluginsMessage* customMsg = dynamic_cast<makVre::StopAllPluginsMessage*>(msg);

	if (!customMsg)
	{
		return makVre::IGNORED;
	}

	if (customMsg->getSender() != entity()->entityId())
	{
		return makVre::IGNORED;
	}


	std::cout << "[MyHumanMovementActuator] customMsg->getStopRequested(): <<" << std::boolalpha <<customMsg->getStopRequested() << std::endl;


	return makVre::HANDLED;
}


void MyHumanMovementActuator::SendVreMessage()
{


	////ForwardMessageMessage -> Send to start forwarding the specified event manager message type to the network. -> Front+BackEnd
	//auto* fMsg = makVre::ForwardMessageMessage::create();
	//fMsg->setMessageName(makVre::StopAllPluginsMessage::theType());
	//makVre::DtVreMessageManager::instance().queueMessage(fMsg);

	std::cout << "[VreMessageTest] Send Message \n";

	//Message 생성
	makVre::StopAllPluginsMessage* msg = makVre::StopAllPluginsMessage::create();
	msg->setSender(entity()->entityId());
	msg->setStopRequested(stopRequested);

	makVre::DtVreMessageManager::instance().queueMessage(msg);

}



void MyHumanMovementActuator::SetCharacterMovement(float headingDeg, const float& tdmVelXkmh, const float& tdmVelYkmh)
{

	//Heading => Degree(0~360) -> radian(0~2π)
	const float headingRad = headingDeg * (M_PI / 180.0f);
	//humanControLogic->setCurrentHeading(headingRad);//Set Heading
	//gTopoHeadingRad = headingRad;
	//localStateRepository()->setCurrentHeading(headingRad);

	

	//tdm(topo) -> local (km/h) [localX = strafe, localY = move]
	//2D 회전 행렬(2D Rotation Matrix),시계방향(CW) 기준 회전 변환
	float cosAngle = cosf(headingRad);
	float sinAngle = sinf(headingRad);
	float localVelXkmh = cosAngle * tdmVelXkmh - sinAngle * tdmVelYkmh;//localX_kmh
	float localVelYkmh = sinAngle * tdmVelXkmh + cosAngle * tdmVelYkmh;//localY_kmh
	

	//X,Y Speed(kmh -> Mps)
	localVelXkmh = localVelXkmh * KMH_TO_MPS;
	localVelYkmh = localVelYkmh * KMH_TO_MPS;


	//atan2 = x, y 좌표(또는 벡터 성분)로부터 각도(angle)를 구하는 함수
	double speed = std::sqrt(localVelXkmh * localVelXkmh + localVelYkmh * localVelYkmh);
	

	//월드 좌표계에서의 최종 이동 방향 = (플레이어가 바라보는 방향) + (그 기준에서 이동 벡터가 틀어진 각도)
	//ex) 플레이어가 북쪽(heading = 0°)을 보고 있을 때, 로컬에서 “오른쪽으로 이동”이면
	//	→ 월드 기준으로는 동쪽(90°) 쪽으로 이동하는 식으로 변환
	//manip = 0,  headingRad = 6.28319, std::atan2(localVelXkmh, localVelYkmh): 0.785398, direction: 0.785398
	//manip = 92, headingRad: 1.6057, std::atan2(localVelXkmh, localVelYkmh): -1.8342, direction: 6.05469
	double direction = headingRad + std::atan2(localVelXkmh, localVelYkmh);
	normalizeDirection0To2Pi(direction);//0~2pi

	//이동속도
	myDesiredSpeed				= speed;

	//이동방향
	myDesiredMovementDirection = direction;


	//std::cout << "headingDeg: " << headingDeg <<",speed: "<< myDesiredSpeed << ", direction: " << myDesiredMovementDirection << std::endl;

	
}




void MyHumanMovementActuator::SendDataToDsms()
{

	const size_t size = sizeof(TDMDataPacket);
	unsigned char sendData[size];
	std::memcpy(sendData, &tdmDataPkt, size);
	//unsigned char* bytePtr = reinterpret_cast<unsigned char*>(&tdmDataPkt);

	//0x01 (소형 트레드밀), 0x02(중형 트레드밀)
	BYTE deviceID = sendData[1];
	if (deviceID == 0x01)
	{
		//소형 이동데이터 메시지
		sendData[2] = (uint8_t)VrisMsgID::StmsMoveMsgID;
	}

	else if (deviceID == 0x02)
	{

		//중형 이동데이터 메시지
		sendData[2] = (uint8_t)VrisMsgID::MtmsMoveMsgID;
	}


	//device ID => 0x71(VRIS)
	sendData[1] = (uint8_t)DeviceID::Vris;


	BOOL result = dsmsUdpUcastClient->SendData(sendData, size);

	if (result)
	{


		std::cout << "[트레드밀 송신] 데이터 : ";
		//std::cout << "data length : " << len << ", data : ";
		std::cout << std::hex << std::uppercase;
		for (int i = 0; i < size; ++i)
			std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(sendData[i]) << " ";
		std::cout << std::dec << std::endl;

		/*
		cout << "[트레드밀 송신 데이터] STX :" << (int)TDMDataPacket._STX
			<< ", DEVICE_ID : " << (int)TDMDataPacket.DEVICE_ID
			<< ", MSG_ID : " << (int)TDMDataPacket.MSG_ID
			<< ", MSG_TYPE : " << (int)TDMDataPacket.MSG_TYPE
			<< ", LENGTH : " << (int)TDMDataPacket.LENGTH
			<< ", x속도 : " << TDMDataPacket.VEL_X_KMH
			<< ", Y속도 : " << TDMDataPacket.VEL_Y_KMH
			<< ", 방향 : " << TDMDataPacket.ORI_DEG
			<< ", 복합속도 : " << TDMDataPacket.VEL_KMH
			<< ", 가속도 : " << TDMDataPacket.ACC_MS2
			<< ", 소음 : " << TDMDataPacket.SOUND_DBA

			<< ", X경사도 : " << TDMDataPacket.xROLL
			<< ", Y경사도 : " << TDMDataPacket.yPITCH
			<< ", 트레드밀 가동상태 : " << (int)TDMDataPacket.tdmOpStat
			<< ", 트레드밀 준비상태 : " << (int)TDMDataPacket.tdmRdStat
			<< ", 매니퓰레이터 가동상태 : " << (int)TDMDataPacket.manipOpStat
			<< ", 매니퓰레이터 준비상태 : " << (int)TDMDataPacket.manipRdStat
			<< ", 매니퓰레이터 회전각 : " << (int)TDMDataPacket.manipRotAng
			<< ", Active 매니퓰레이터 반응속도 : " << (int)TDMDataPacket.manipReactSpd
			<< ", Active 매니퓰레이터 지연시간 : " << (int)TDMDataPacket.manipDelayTime
			<< ", ETX : " << (int)TDMDataPacket._ETX
			<< endl;
		*/


	}
	//else
	//{
	//	//std::cerr << "[TDMMovePlugin][Error] Failed to send data to the dsms ";
	//}

}
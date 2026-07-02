

#include "Skeleton.h"
#include <vreUtil\logger.h>
#include <vrfobjcore\vrfMovingObjectStateRepository.h>
#include <vrvDiGuy\DtDiGuyArtPartLinkMapper.h>
#include "vrfMsgTransport/diGuyActionEvent.h"

#include "Nesslab/nesslabUdpServer.h"
#include "Nesslab/nesslabUdpClient.h"
#include "Nesslab/nesslabCommon.h"
#include "Nesslab/StopAllPluginsMessage.h"



#include "vlpi/entityIdentifier.h"
#include "vlpi/entityIdentifier.inl"
#include "vrfobjcore/localObject.h"

using namespace makVrf;
using namespace makVrfEvents;
using namespace nesslab_common;

float gSkHeadPosZ = 0;


struct EulerData
{
	float Azimuth	= 0.0f;//Z축
	float Elevation	= 0.0f;//Y축
	float Rotation	= 0.0f;//X축
};


//199 byte
//Motive or YSU --(UDP_Unicast)--> SubVRIS --(UDP_Unicast)--> VRIS
#pragma pack(push, 1)
struct SkeletonDataPacket
{
	PacketHeader header;

	//Payload => Rigidbody 15개(Euler + radian)
	//중앙3
	EulerData base;
	EulerData back;
	EulerData cervical;

	//왼쪽6
	EulerData leftShoulder;
	EulerData leftElbow;
	EulerData leftWrist;
	EulerData leftHip;
	EulerData leftKnee;
	EulerData leftAnkle;

	//오른쪽 6
	EulerData rightShoulder;
	EulerData rightElbow;
	EulerData rightWrist;
	EulerData rightHip;
	EulerData rightKnee;
	EulerData rightAnkle;


	float reserved_1 = 0;//reserved(사용x)
	float reserved_2 = 0;//head,cervical Z	=> BoundingVolume(피격박스) 높이 조절
	float reserved_3 = 0;//basePosZ			=> 스켈레톤 높이 조절


	PacketTrailer trailer;


	SkeletonDataPacket()
	{
		header.deviceID = 0;//0x53 = IPMS((인프라기반 자세/행동 인식 장비)), 0x67 = SPMS(세미인프라기반 자세/행동 인식 장비)
		header.msgID	= 0;//0x54 = 인프라기반 3D 자세 정보,				 0x65 = 세미인프라기반 3D 자세 정보
		header.length	= sizeof(SkeletonDataPacket);
	}

};
#pragma pack(pop)



namespace makVre
{

	namespace {

		//Motive or YSU --(UDP_Unicast)--> SubVRIS --(UDP_Unicast)--> VRIS, Port = 2800
		nesslabUdpServer*	skUdpUcastServer = nullptr;
		int					skUdpPort = -1;//2800
		SkeletonDataPacket	skeletonDataPkt;

		std::vector<BYTE> skRecvBuffer{};

		PacketHandlers packetHandlers{};
	}



	//Test
	namespace {

		nesslabUdpClient* testUcastClient = nullptr;
		std::string		  testIP = "127.0.0.1";
		int				  testPort = 9997;

		//사용할거면 true로해주기
		bool			  enableSimMonitoring = false;

		//Test
		void OnDataRecv(const std::uint8_t* data, int len)
		{
			memcpy(&skeletonDataPkt, data, len);
		}

	}



	//Constructor => Enage Human 스켈레톤 캐릭터 생성하면 실행
	SkeletonPlugin::SkeletonPlugin(const DtString& name,
		DtLocalObject* owner,
		DtSimulationServices* simManager,
		DtComponentDescriptor* desc,
		DtReaderWriterRegistry* parentRegistry)
		: DtVreSimComponent<DtActuatorComponent>(name, owner, simManager, desc, parentRegistry)
		, myPlatformLocalObjectFacade(owner)
		//, myLocalState(nullptr)
		, myPlayerControlled(false)
		//, myCachedHandItem("")
		, myLeftShoulder(0)
		, myLeftWrist()
		, myAnimationInterval(4)
		, myTimeInInterval(0)
	{
		std::cout << "[SkeletonPlugin][Trace] Constructor " << std::endl;
		skBoundingVolume = new SkBoundingVolume(owner);

	}



	// destructor => Human 아닌 오브젝트를 생성하거나 다른 인간 캐릭터 생성하면 실행
	SkeletonPlugin::~SkeletonPlugin()
	{
		std::cout << "[SkeletonPlugin][Trace] Destructor " << std::endl;


		if (netThread.joinable())
			netThread.join();

		delete skBoundingVolume;

		if (skUdpUcastServer != nullptr)
		{
			skUdpUcastServer->ServerStop();
			delete skUdpUcastServer;
			skUdpUcastServer = nullptr;
		}


		if (enableSimMonitoring)
		{
			if (testUcastClient != nullptr)
			{
				testUcastClient->Disconnect();
				delete testUcastClient;
				testUcastClient = nullptr;
			}

		}
	}



	bool SkeletonPlugin::init()
	{
		if (!DtActuatorComponent::init())
		{
			return false;
		}
	
		InitPlugin();

		return true;
	}



	// This returns a string from compTypes.h, and identifies the type of component
	const char* SkeletonPlugin::type() const
	{
		return SKELETON_PLUGIN_TYPE;
	}

	DtSimComponent* SkeletonPlugin::creator(const DtString& name,
		DtLocalObject* owner,
		DtSimulationServices* simManager,
		DtComponentDescriptor* desc,
		DtReaderWriterRegistry* parentRegistry)
	{

		//return MarkThreadSafe(new DtExampleHumanHandWeaponControlActuator(name, owner, simManager, desc, parentRegistry));
		return new SkeletonPlugin(name, owner, simManager, desc, parentRegistry);
	}




	void SkeletonPlugin::tick()
	{

		//// Check to see if the simulation is paused.
		//if (dT() == 0.)
		//{
		//	return;
		//}

		if (stopRequested)
			return;

		if (!isPlayerControlled())
		{
			return;
		}


		SetSkeleton();
		
		//
		if (enableSimMonitoring)
		{
			//skeletonDataPkt
			const size_t dataSize = sizeof(SkeletonDataPacket);
			uint8_t sendData[dataSize];
			memcpy(sendData, &skeletonDataPkt, dataSize);


			testUcastClient->SendData(sendData, dataSize);


		}


	}




	void SkeletonPlugin::InitPlugin()
	{
		std::cout << "[SkeletonPlugin][Trace] initSkeleton " << std::endl;


		skRecvBuffer.clear();
		packetHandlers.clear();

		//Get ConfigFile Data
		{

			//Skeleton
			skUdpPort = GetPrivateProfileInt("YSU_Skeleton", "skPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());


			if (skUdpPort == INI_INT_NOT_FOUND_DEFAULT)
				LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[SkeletonPlugin][Error] Failed to read data from INI file. Using default value.\n";


			std::cout << "[SkeletonPlugin][Debug] skeletonUdpPort: " << skUdpPort << std::endl;

		}

		

		//Store packet data(파싱, 콜백 처리용)
		{

			//인프라 3D자세 데이터 -> VRIS
			RegisterPacketHandler<SkeletonDataPacket>(packetHandlers, (int)DeviceID::Ipms, (uint8_t)PoseMsgID::Infra3DPoseMsgID, skeletonDataPkt,
				NULL);

			//세미인프라 3D자세 데이터 -> VRIS
			RegisterPacketHandler<SkeletonDataPacket>(packetHandlers, (int)DeviceID::Spms, (uint8_t)PoseMsgID::SemiInfra3DPoseMsgID, skeletonDataPkt,
				NULL);

		}



		//Create and start TCP/UDP
		{

			netThread = std::thread([this]() {

				skUdpUcastServer = new nesslabUdpServer(skUdpPort, nesslabUdpServer::PacketTransmissionMode::Unicast);
				skUdpUcastServer->SetOnDataReceived(
					[this](unsigned char* data, int len) {
						OnDataReceived(data, len);
					}
				);

				skUdpUcastServer->ServerStart();

			});


			//Test
			if (enableSimMonitoring)
			{
				testUcastClient = new nesslabUdpClient(testPort, testIP, nesslabUdpClient::PacketTransmissionMode::Unicast);
				testUcastClient->SetOnDataReceived(OnDataRecv);
				testUcastClient->Connect();
			}



		}





		//Init Skeleton
		{

			/*
				Look up the articulated part associated with the elbow.  The art part types for all joints on the DI-Guy
				skeleton are defined in DtDiGuyArtPartLinkMapper.h. Note that an entry for the part must also exist for
				the entity in the .entity file.  See the list of art parts in TestHuman.entity.

				팔꿈치에 연결된 관절 파트를 조회합니다.
				DI-Guy 스켈레톤의 모든 관절에 대한 아트 파트 유형은 DtDiGuyArtPartLinkMapper.h에 정의되어 있습니다.
				.entity 파일에 있는 엔티티에도 해당 파트에 대한 항목이 있어야 합니다.
				TestHuman.entity에서 아트 파트 목록을 참조하세요.
			*/


			//중앙(3개)
			myBase		= myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartBase));//골반 중앙
			myBack		= myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartBack));//명치 약간 아래
			myCervical	= myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartCervical));//머리와 목 사이

			//왼쪽(6개)
			myLeftShoulder	= myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartShoulderLeft));//왼쪽 어깨
			myLeftElbow		= myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartElbowLeft));//왼쪽 팔꿈치
			myLeftWrist		= myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartWristLeft));//왼쪽 손목
			myLeftHip		= myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartHipLeft));//왼쪽 엉덩이
			myLeftKnee		= myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartKneeLeft));//왼쪽 무릎
			myLeftAnkle		= myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartAnkleLeft));//왼쪽 발목

			//오른쪽(6개)
			myRightShoulder = myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartShoulderRight));//오른쪽 어깨
			myRightElbow	= myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartElbowRight));//오른쪽 팔꿈치
			myRightWrist	= myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartWristRight));//오른쪽 손목
			myRightHip		= myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartHipRight));//오른쪽 엉덩이
			myRightKnee		= myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartKneeRight));//오른쪽 무릎
			myRightAnkle	= myPlatformLocalObjectFacade.nextFramePart(static_cast<DtArtPartType>(makVrv::DiGuyDisArtPartAnkleRight));//오른쪽 발목


			mySkeletons = { myBase, myBack, myCervical,
						  myLeftShoulder,  myLeftElbow,  myLeftWrist,  myLeftHip,  myLeftKnee,  myLeftAnkle ,
						  myRightShoulder, myRightElbow, myRightWrist, myRightHip, myRightKnee, myRightAnkle 
			};


		}

	}



	void SkeletonPlugin::SetSkeleton()
	{
		if (!simulationServices())
		{
			objectConsoleError()("Uninitialized simulation services detected by component: %s\n", name().c_str());
			return;
		}

		/*
		z축 = azimuth (yaw)
		y축 = elevation (pitch)
		x축 = rotation (roll)
		*/

		std::uint8_t* bytes = reinterpret_cast< std::uint8_t*>(&skeletonDataPkt);//패킷 맨 앞 바이트 주소
		std::uint8_t* index = bytes + PACKET_HEADER_SIZE;//StartIndex 주소+헤더
		constexpr std::size_t dataSize = sizeof(float);


		for (makVrf::DtArticulatedPartStateRepository* sk : mySkeletons)
		{

			float azimuth, elevation, rotation;

			std::memcpy(&azimuth, index, dataSize);//Z
			index += dataSize;

			std::memcpy(&elevation, index, dataSize);//Y
			index += dataSize;

			std::memcpy(&rotation, index, dataSize);//X
			index += dataSize;

			sk->setAzimuth(azimuth);
			sk->setElevation(elevation);
			sk->setRotation(rotation);

		}



		//※※※ vrfSim\ExampleHumanHandWeaponControl.entity파일에서 코드 추가 필요 => 설정안되면 회전만 처리돼서 앉거나 누웠을 때 공중에 뜬 것처럼 보이는 현상 발생  ※※
		// Adjust skeleton height on Z axis
		DtVector localPos = { 0,0,skeletonDataPkt.reserved_3 };
		myBase->setTranslation(localPos);



		//Sim bounding Volume => 피격처리 하는 박스(VRF에서 크기 확인가능)
		gSkHeadPosZ = skeletonDataPkt.reserved_2;
		skBoundingVolume->updateBoundingVolumeBasedOnPosture();

	}



	void SkeletonPlugin::OnDataReceived(unsigned char* data, int len)
	{
		//std::cout << "[SkeletonPlugin][DEBUG] Received Data from Motive";
		//std::cout << "Data Length : " << len << ", Data : ";
		//std::cout << std::hex << std::uppercase;
		//for (int i = 0; i < len; ++i)
		//	std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
		//std::cout << std::dec << std::endl;


		skRecvBuffer.insert(skRecvBuffer.end(), data, data + len);
		ParseReceiveBuffer(packetHandlers, skRecvBuffer);


	}


	makVre::DtVreMessageResult SkeletonPlugin::HandleStopPluginMessage(makVre::DtVreMessage* msg)
	{

		//ASSERT_TYPE(msg, CustomMessage, rMsg);
		std::cout << "[SkeletonPlugin][Trace] recv HandleCustomMessage \n";


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


		std::cout << "[SkeletonPlugin][Info] StopAllPluginsMessage->getStopRequested(): <<" << std::boolalpha << stopPluginMsg->getStopRequested() << std::endl;

		stopRequested = stopPluginMsg->getStopRequested();


		return makVre::HANDLED;

	}

}

/*
//Gesture
{


	DtDiGuyActionEvent leftHandAction;//왼손 제스처
	DtDiGuyActionEvent rightHandAction;//오른손 제스처


	// 왼손 제스처
	leftHandAction.setAction(DtDiGuyActionEvent::StartGesture);		// 제스처 재생
	leftHandAction.setEntityId(entity()->uuid());					// 엔티티 ID 설정
	leftHandAction.setReps(1);										// 제스처 반복 횟수 : 1회
	leftHandAction.setOverallDuration(60);							// 제스처 재생 길이 : 60초 (수정 필요)

	// 오른손 제스처
	rightHandAction.setAction(DtDiGuyActionEvent::StartGesture);	// 제스처 재생
	rightHandAction.setEntityId(entity()->uuid());					// 엔티티 ID 설정
	rightHandAction.setReps(1);										// 제스처 반복 횟수 : 1회
	rightHandAction.setOverallDuration(60);							// 제스처 재생 길이 : 60초 (수정 필요)
}


	std::ostringstream handLeft;
	std::ostringstream handRight;

	void SkeletonPlugin::SetGesture()
	{



		//TODO: 현재 재생 중인 제스처와 같을 경우 return
		//int order = 0;
		//int count = 0;
		//if (order == count) return;
		//order = count;



		// 왼손
		handLeft.str("");
		handLeft << "diguy_hand_l_" << std::setw(2) << std::setfill('0') << (int)skeletonDataPkt.reserved_1 % 29;
		leftHandAction.setGesture(handLeft.str());						// 재생 할 제스처 이름
		simulationServices()->addEvent(leftHandAction);					// 제스처 실행
		std::cout << handLeft.str() << std::endl;

		// 오른손
		handRight.str("");
		handRight << "diguy_hand_r_" << std::setw(2) << std::setfill('0') << (int)skeletonDataPkt.reserved_1 % 29;
		rightHandAction.setGesture(handRight.str());					// 재생 할 제스처 이름
		simulationServices()->addEvent(rightHandAction);				// 제스처 실행
		std::cout << handRight.str() << std::endl;

	}






*/
#include "MyHumanGunJoyActuator.h"

#include "Nesslab/nesslabTcpServer.h"

#include "vrfutil/simResourceManager.h"
#include "vrfutil/simResource.h"


#include "vlutil/vlUtil.h"
#include "vrfcore/simulationServices.h"
#include "vrfobjcore/vrfBalGunPSR.h"
#include "vrfobjcore/resourceMonitor.h"
#include <vreUtil/logger.h>
#include <vreMessageManager/vreMessageManager.h>
#include "vrfExtensions/vreVrfobjcore/vreMunitionLoader.h"


/*				메모
 
BackEnd에서 Properties을 찾고 접근 하는 방법

//Properties 목록 콘솔에 출력
 std::cout << myEntity->nextFrameStateProperties().stringRep().c_str() << std::endl;

 //Console
 state-properties = { HasLaser = False; laserCode = -1; AIEnabled = True; Invulnerable = False; Attention-Grabbing = 0; FreezeMovement = False; Skill-Level = 1;
 Emergency-Transmitter = False; FreezeMunitionLevels = False; NavigationPriority = 1; Undetectable = False; OrderedHeading = 0; OrderedAltitude = 0;
 OrderedSpeed = 0; BlockParachute = False; OverallHealth = 100; MobilityHealth = 100; FirepowerHealth = 100; Fatigue = 0; PercentOfMaxSpeed = 0;
 AmmoInClips = { weapon = { CurrentAmount = 29; FullAmount = 30;  }weapon1 = { CurrentAmount = 15; FullAmount = 15;  } }ReloadTimer = 0;  }


//Get
boost::optional<int> property = myEntity->nextFrameStateProperties().findPropertyValue<int>("Skill-Level");
if(property)
	std::cout << "property: " << property.value() << std::endl;


//Set
const DtString propertyName = "AmmoInClips.weapon.CurrentAmount";
auto property = myEntity->nextFrameStateProperties().findPropertyValue<int>(propertyName);
if (property)
{
	bool result = myEntity->nextFrameStateProperties().findAndSetPropertyValue<int>(propertyName, myProcessState->munitionLoader().roundsLeftInMagazine());
	if (!result)
		std::cout << "[MyHumanGunJoyActuator][Error] Property not found. propertyName:" << propertyName << std::endl;
}

*/


bool gReloadBlocked = true;
const char* gK2MunitionType = "2:8:225:2:1:1:2";

//			메모
//모의총기 플러그인(FE)와 재장전 플러그인(BE)에서 받는 패킷 데이터는 같음(포트만다름)

//재장전 신호 데이터(임시)
#pragma pack(push, 1)
struct WeaponReloadPacket
{
	PacketHeader header;

	//Payload
	uint8_t ammoInMag	= 0;//잔탄량	=> 0 ~ 100
	uint8_t fireCounter = 0;//격발량	=> 0 ~ 100
	uint8_t magStatus	= 0;//탄창 상태 => 0=정상, 1=JAM(기능고장), 2=잔탄량 0, 3=재장전(임시)
	uint8_t battery		= 0;//배터리	=> 0 ~ 100(5단위)

	PacketTrailer trailer;


	WeaponReloadPacket()
	{
		header.deviceID = (int)DeviceID::Hagh;//전자탄창, 제어기(TODO: VRIS로 수정)
		header.msgID	= (uint8_t)HaghMsgID::WeaponReloadMsgID;//격발 재장전 신호(임시값)
		header.msgType	= 0;//Reserved
		header.length	= sizeof(WeaponReloadPacket);
	}

};
#pragma pack(pop)


#include "HumanGunJoyActuatorCmds.h"

namespace {

	//VRIS -> UDP_Client -> Sub_VRIS
	nesslabTcpServer*	reloadTcpServer = nullptr;
	int					reloadPort = -1;
	WeaponReloadPacket	reloadPkt;

}



//소총, 권총 각 각 생성자 생성
MyHumanGunJoyActuator::MyHumanGunJoyActuator(const DtString& name, DtLocalObject* owner, DtSimulationServices* simManager, 
	DtComponentDescriptor* desc, DtReaderWriterRegistry* parentRegistry)
	:DtHumanGunJoyActuator(name, owner,simManager,desc,parentRegistry) 
{
	std::cout << "[MyHumanGunJoyActuator][Trace] Constructor " << std::endl;
	

}

MyHumanGunJoyActuator::~MyHumanGunJoyActuator()
{
	std::cout << "[MyHumanGunJoyActuator][Trace] Destructor " << std::endl;


	if (netThread.joinable())
		netThread.join();


	if (reloadTcpServer != nullptr)
	{
		reloadTcpServer->ServerStop();
		delete reloadTcpServer;
		reloadTcpServer = nullptr;
	}

}



static int testTickCount = 0;

bool MyHumanGunJoyActuator::init()
{
	if (!DtHumanGunJoyActuator::init())
		return false;

	std::cout << "[MyHumanGunJoyActuator][Trace] init \n";

	/* -> VRE 2.1.1b
	//탄이름 
	std::cout << "[MyHumanGunJoyActuator][Info] name: => "		   << myProcessState->munitionLoader().currentMunition().name() << std::endl;
	std::cout << "[MyHumanGunJoyActuator][Info] variableName: => " << myProcessState->munitionLoader().currentMunition().variableName() << std::endl;
	std::cout << "[MyHumanGunJoyActuator][Info] munitionType: => " << myProcessState->munitionLoader().currentMunition().munitionType() << std::endl;
	std::cout << "[MyHumanGunJoyActuator][Info] munitionType: => " << myProcessState->currentMunitionType().string() << std::endl;
	std::cout << "[MyHumanGunJoyActuator][Info] munitionType: => " << myVreMunitionLoader->currentMunition().munitionType() << std::endl;

	//총기이름
	std::cout << "[MyHumanGunJoyActuator][Info] displayName: " << myProcessState->displayName() << std::endl;


	std::string weaponName = myProcessState->displayName();
	std::cout << "weaponName: " << weaponName << std::endl;

	//훈련용 무기(K2)일때만 처리
	//if (weaponName == "M16 rifle" || weaponName == "K2 rifle")//M9 Pistol
	if (weaponName == "K2 rifle")//M9 Pistol
	{
		InitPlugin();

		//자동재장전 방지
		gReloadBlocked = true;
	}
	*/

	testTickCount = 0;

	return true;
}


const char* MyHumanGunJoyActuator::type() const
{
	return makVre::DtHumanGunJoyActuatorType;
}


void MyHumanGunJoyActuator::tick()
{

	if (testTickCount == 0)
	{

		//훈련용 무기(K2)일때만 처리
		if (myVreMunitionLoader->currentMunition().munitionType() == gK2MunitionType)
		{
			InitPlugin();

			//자동재장전 방지
			gReloadBlocked = true;
		}
		testTickCount++;

	}


	DtHumanGunJoyActuator::tick();
}


DtSimComponent* MyHumanGunJoyActuator::creator(const DtString& name, DtLocalObject* owner, DtSimulationServices* simManager, DtComponentDescriptor* desc, DtReaderWriterRegistry* parentRegistry)
{
	std::cout << "[MyHumanGunJoyActuator][Trace] creator \n";

	return new MyHumanGunJoyActuator(name, owner, simManager, desc, parentRegistry);
}



// 현재 탄창의 남은 탄 수 설정(탄창 내 잔탄 수)
// - 탄창 최대 탄 수 넘어가면 최대 탄수로 Set
// - 키보드 R키처럼 재장전 사용하려면 0으로 만들어주기
void MyHumanGunJoyActuator::SetRoundsInCurrentMagazine(int roundsToSet)
{

	std::cout << "myProcessState->munitionLoader().roundsPerMagazine(): " << myProcessState->munitionLoader().roundsPerMagazine() << std::endl;

	
	//roundsToSet값이 탄창 최대 탄 수 넘어가면 roundsToSet값을 탄창 최대 탄수로 변경 
	if (roundsToSet > myProcessState->munitionLoader().roundsPerMagazine())
		roundsToSet = myProcessState->munitionLoader().roundsPerMagazine();

	
		

	//roundsToSet값이 전체탄수보다많으면 roundsToSet = 전체 탄 수
	//DtSimResource* munitionResource = resourceManager().lookupResource(myProcessState->currentMunitionName());
	DtSimResource* munitionResource = entity()->resourceManager().lookupResource(myProcessState->currentMunitionType());
	if (munitionResource)
	{
		//DtMunitionResource* resource = static_cast<DtMunitionResource*>(munitionResource);
		//if (resource && roundsToSet > resource->amount())
		if (roundsToSet > munitionResource->amount())
			roundsToSet = munitionResource->amount();
	}



	std::cout << "myProcessState->munitionLoader().roundsLeftInMagazine()=> " << myProcessState->munitionLoader().roundsLeftInMagazine() << std::endl;	
	
	//제거할 탄 수= 탄창에 남은 탄 수 - 내가 원하는 탄 수
	int roundsToDeplete = myProcessState->munitionLoader().roundsLeftInMagazine() - roundsToSet;
	if (roundsToDeplete <= 0)
		roundsToDeplete = 0;

	std::cout << "roundsToDeplete => " << roundsToDeplete << std::endl;

	/*
	std::cout << "[MyHumanGunJoyActuator][Info] roundsLeftInMagazine: " << myProcessState->munitionLoader().roundsLeftInMagazine()
			  << ",reloadPkt.ammoInMag: " << reloadPkt.ammoInMag
			  << ",roundsToDeplete: " << roundsToDeplete << std::endl;

	
	std::cout << "[MyHumanGunJoyActuator][Info] currentMunition: " << myProcessState->munitionLoader().currentMunition().munitionName()//탄이름
			  << ", displayName: " << myProcessState->displayName()//총기이름
			  << std::endl;


	//StateProperties
	std::cout << myEntity->nextFrameStateProperties().stringRep().c_str() << std::endl;
	*/


	std::cout << "roundsToSet: " << roundsToSet << std::endl;
	// 현재 탄창 탄 수 설정
	myProcessState->munitionLoader().loadComplete(roundsToSet);


	std::cout << "roundsToDeplete: " << roundsToDeplete << std::endl;

	// 전체 탄 수로 반환 된(현재 탄창에 남은 탄 수)를 제거
	// => loadComplete(roundsToSet)를하면 roundsToSet 개수 만큼 전체 탄 수(총기 UI 오른쪽 값 30/180)가 추가됨
	// => depleteResource()를 사용해서 추가된 개수 만큼 제거
	depleteResource(roundsToDeplete);



	/*
	기존 탄창에 남아있는 탄 반환안하고 싶으면 abs처리
	int roundsToDeplete = myProcessState->munitionLoader().roundsLeftInMagazine() - roundsToSet;
	roundsToDeplete = abs(roundsToDeplete);
	depleteResource(roundsToDeplete);

	//ex) 
	// 기존방식: 5/100 -> SetRoundsInCurrentMagazine(10) -> 10/95
	// 위에방식: 5/100 -> SetRoundsInCurrentMagazine(10) -> 10/90
	*/
	



	/* 	Update UI to reflect changes. -> VRE 2.1.1b
	//UI를 업데이트하기 위해서는 systemName이 필요한데 무기마다 다를 수 있음(weaponSystemName 중간에 .weapon-2 값)	
	// systemName 확인 방법은 1. 캐릭터.entity 파일 -> 2. 총기 무기  <componentSystem systemName="weapon-2" 부분에서 systemName값
	// ex)
	// <componentSystem systemName="weapon" platform="@(system-dir)\weapons\handheld-K2.sysdef">    //weapon
	// <componentSystem systemName="weapon1" platform="@(system-dir)\weapons\handheld-K5.sysdef">   //weapon1
	// <componentSystem systemName="weapon-2" platform="@(system-dir)\weapons\handheld-M16.sysdef"> //weapon-2

	//weaponSystemName: base-system.weapon-2.handheld-M16.main-gun-control.weapon-interface
	std::string weaponSystemName = myProcessState->weaponInterface()->name();

	size_t firstDot  = weaponSystemName.find('.');
	size_t secondDot = weaponSystemName.find('.', firstDot + 1);

	if (firstDot != std::string::npos && secondDot != std::string::npos)
	{
		weaponSystemName = weaponSystemName.substr(firstDot + 1, secondDot - firstDot - 1);
		std::cout << "[MyHumanGunJoyActuator][Info] weaponSystemName:" << weaponSystemName << std::endl;//ex) weapon-2


		//AmmoInClips.무기별 SystemName.CurrentAmount
		std::string propertyName = "AmmoInClips.";
		propertyName.append(weaponSystemName);
		propertyName.append(".CurrentAmount");
		std::cout << "[MyHumanGunJoyActuator][Info] propertyName:" << propertyName << std::endl;//ex) AmmoInClips.weapon-2.CurrentAmount


		auto property = myEntity->nextFrameStateProperties().findPropertyValue<int>(propertyName);
		if (property)
		{
			// UI가 변경사항을 반영하도록 업데이트
			bool result = myEntity->nextFrameStateProperties().findAndSetPropertyValue<int>(propertyName, myProcessState->munitionLoader().roundsLeftInMagazine());
			if (!result)
				std::cout << "[MyHumanGunJoyActuator][Error] Property not found. propertyName:" << propertyName << std::endl;
		}

	}
	*/

	// Update UI to reflect changes. -> VRE 2.2
	std::string currentName = std::string("AmmoInClips.") + myProcessState->munitionLoader().currentMunition().munitionType().string() +
		".CurrentAmount";
	myEntity->nextFrameStateProperties().findAndSetPropertyValue<int>(currentName, myProcessState->munitionLoader().roundsLeftInMagazine());



}



void MyHumanGunJoyActuator::depleteResource(int amount)
{

	//DtSimResource* munitionResource = resourceManager().lookupResource(myProcessState->currentMunitionName());
	DtSimResource* munitionResource = entity()->resourceManager().lookupResource(myProcessState->currentMunitionType());
	if (munitionResource)
	{
		munitionResource->deplete(DtMin(amount, (int)munitionResource->amount()));
		simulationServices()->resourceMonitor()->resourcesChangedFor(entity());// 리소스 변경사항 동기화

		/*
		DtMunitionResource* resource = static_cast<DtMunitionResource*>(munitionResource);
		if (resource)
		{
			resource->deplete(DtMin(amount, (int)resource->amount()));
			simulationServices()->resourceMonitor()->resourcesChangedFor(entity());// 리소스 변경사항 동기화
		}
		else
		{
			objectConsoleWarn(type()) << "could not decrement munition resource" << std::endl;
			objectConsoleWarn(type()) << "resource " << myProcessState->currentMunitionName() << " not found" << std::endl;
		}
		*/



	}
	else
	{
		std::cout << "[MyHumanGunJoyActuator][Error] could not decrement munition munitionResource ";
		std::cout << "[MyHumanGunJoyActuator][Error] resourceType: " << myProcessState->currentMunitionType() << "not found \n";

		//objectConsoleWarn(type()) << "could not decrement munition resource" << std::endl;
		//objectConsoleWarn(type()) << "resource " << myProcessState->currentMunitionType() << " not found" << std::endl;
		//std::cout << "[MyHumanGunJoyActuator][Error] Resource not found. resourceName: " << myProcessState->currentMunitionName() << std::endl;

	}


}


//총 잔탄(total rounds remaining) 설정
void MyHumanGunJoyActuator::SetTotalRoundsRemaining(int amount)
{

	//DtSimResource* munitionResource = resourceManager().lookupResource(myProcessState->currentMunitionName());
	DtSimResource* munitionResource = resourceManager().lookupResource(myProcessState->currentMunitionType());
	if (munitionResource)
	{
		//DtMunitionResource* resource = static_cast<DtMunitionResource*>(munitionResource);
		//resource->setAmount(amount);//총 잔탄 수 변경	
		
		munitionResource->setAmount(amount);//총 잔탄 수 변경


		
		
		//100 + myProcessState->munitionLoader().roundsLeftInMagazine()에서 탄창에 남은 탄 수를 더해주는 이유
		//=> resource->setAmount()를 호출하면 내부적으로 먼저“현재 탄창 탄 수”를 DtVreMunitionLoader::myRoundsPerMagazine 기준으로 맞추고, 
		//   나머지를 총 잔탄으로 처리하는 방식이 섞여 있기 때문
		//인자값 = 남은 탄 수 + 탄창에 남은 탄 수
		//ex) 17/360 -> setAmount(100+탄창에 남은 탄 수) -> 17/100
		//resource->setAmount(100 + myProcessState->munitionLoader().roundsLeftInMagazine());//총 잔탄 수
		

		//인자값  = 남은 탄 수
		//ex) 17/360 -> setAmount(100) -> 17/83
		//resource->setAmount(100);//총 잔탄 수


		// 리소스 변경사항 동기화
		simulationServices()->resourceMonitor()->resourcesChangedFor(entity());

	}
}




void MyHumanGunJoyActuator::OnWeaponReload(const WeaponReloadPacket& packet)
{
	std::cout << "[MyHumanGunJoyActuator][Trace] OnWeaponReload() \n";

	//재장전신호
	if (reloadPkt.magStatus == 3)
	{	

		/*
		//기존 엔진 재장전 방식
		{
			SetRoundsInCurrentMagazine(0);
			gReloadBlocked = false;
			return;
		}
		*/


		//탄 창에 남은 탄 수 및 전체 탄수를 직접 Set하는 방식
		{
			//탄창에 남은 탄 수 변경
			int roundsToSet = reloadPkt.ammoInMag;
			SetRoundsInCurrentMagazine(roundsToSet);


			//전체 탄수 변경
			//pkt.battery로 테스트
			int amount = reloadPkt.battery + myProcessState->munitionLoader().roundsLeftInMagazine();
			SetTotalRoundsRemaining(amount);

			//자동재장전(AutoReloadBlocker.h) 방지 풀기
			gReloadBlocked = false;
		}


	}
	

}



void MyHumanGunJoyActuator::OnDataReceived(const std::string& clientAddr, const uint8_t* data, int len)
{
	std::cout << "[MyHumanGunJoyActuator][Debug] OnDataReceived \n";

	reloadRecvBuffer.insert(reloadRecvBuffer.end(), data, data + len);
	ParseReceiveBuffer(packetHandlers, reloadRecvBuffer);

}


void MyHumanGunJoyActuator::InitPlugin()
{

	//Get ConfigFile Data
	{

		char databuf[256] = {};

		//Reload
		reloadPort = GetPrivateProfileInt("Weapon", "weaponReloadPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());

		if (reloadPort == INI_INT_NOT_FOUND_DEFAULT)
			LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[MyHumanGunJoyActuator][Error] Failed to read data from INI file. Using default value.\n";

		std::cout << "[MyHumanGunJoyActuator][Info] weaponPort: " << (int)reloadPort << std::endl;
	}


	//Store packet data(데이터 받는 패킷만 처리 => 파싱, 콜백 처리용)
	{
		//Weapon -> VRIS
		RegisterPacketHandler<WeaponReloadPacket>(packetHandlers, reloadPkt.header.deviceID, reloadPkt.header.msgID, reloadPkt,
			[this](const WeaponReloadPacket& packet)
			{
				OnWeaponReload(packet);
			}
		);//Fire Info  Packet
	}



	//Create and start TCP/UDP
	{
		netThread = std::thread([this]() {
			reloadTcpServer = new nesslabTcpServer(reloadPort);
			reloadTcpServer->SetOnDataReceived([this](const std::string& clientAddr, const uint8_t* data, int len)
				{
					OnDataReceived(clientAddr, data, len);
				});
			reloadTcpServer->ServerStart();
		});
	}

}


/* 
=> 격발 하는법(message받아서 격발 구현은했는데 격발위치랑 회전값 조절해줘야할듯 현재는 자기자신이 피격됨
=> 개발 멈춘이유: 다른 무기 일때도 격발신호들어오면 격발처리가 되고 backend에서 하는것보다 frontend에서 하는게 안전해보임(내부적으로 처리하는 관점에서)
//※주의: 격발을 여기서 처리해줘도되는데 만약에 다른 무기들고있는데 이쪽으로 격발신호들어오면 현재 무기가아니라 현재 데이터 받은 플러그인 무기가 격발됨
ex) 권총들고있는데 k2 격발


//Test
#include "ForceFireCmd.h"
#include <vrfutil/rwIndividualLifeFormWeaponStateType.h>
#include <vrftasks/setLifeFormWeaponStateRequest.h>
#include <vrfmsgs/setDataRequestMessage.h>
#include <vrfobjcore/setDataManager.h>

makVre::DtVreMessageResult MyHumanGunJoyActuator::HandleForceFireCmd(makVre::DtVreMessage* msg)
{

	//makVre::ForceFireMessage* forceFireMsg = dynamic_cast<makVre::ForceFireMessage*>(msg);
	//if (!forceFireMsg){
	//	return makVre::IGNORED;
	//}

	//if (forceFireMsg->getTarget() != entity()->entityId()){
	//	return makVre::IGNORED;
	//}
	//myTriggerMunition = myProcessState->munitionLoader().currentMunition().munitionName();
	//SetWeaponState(DtWeaponInFirePosition);
	//fire();
	//return makVre::HANDLED;



makVre::ForceFireMessage* forceFireMsg = dynamic_cast<makVre::ForceFireMessage*>(msg);
if (!forceFireMsg)
{
	return makVre::IGNORED;
}
if (forceFireMsg->getTarget() != entity()->entityId())
{
	return makVre::IGNORED;
}


				////myTriggerMunition = myProcessState->munitionLoader().currentMunition().munitionType();
				//myTriggerMunition = myProcessState->munitionLoader().currentMunition().munitionName();
				//SetWeaponState(DtWeaponInFirePosition);
				//
				//// Update trigger world position to this entity's world position.
				//myTriggerWorldPosition = entity()->worldPosition();
				//
				//// Update trigger world orientation to this enetity's world orientation.
				//myTriggerWorldOrientation = entity()->worldOrientation();
				//
				//// Fire after.
				//fire();

OnFire();


return makVre::HANDLED;

}


//Creates fire interaction
void MyHumanGunJoyActuator::OnFire()
{
	std::cout << "[MyHumanGunJoyActuator] OnFire \n";


	//myTriggerMunition = myProcessState->munitionLoader().currentMunition().munitionType();
	myTriggerMunition = myProcessState->munitionLoader().currentMunition().munitionName();

	SetWeaponState(DtWeaponInFirePosition);


	//격발위치 => 캐릭터 위치에서 시작해서 격발하면 내 캐릭터가 피격됨 => weaponPosition값을 넣어줘야할 듯
	// Update trigger world position to this entity's world position.
	//myTriggerWorldPosition = entity()->worldPosition();
	myTriggerWorldPosition = entity()->nextFrameWorldPosition();





	// Update trigger world orientation to this enetity's world orientation.
	//myTriggerWorldOrientation = entity()->worldOrientation();
	myTriggerWorldOrientation = entity()->nextFrameWorldOrientation();

	// Fire after.
	fire();


		//	//Set Location
		//playerAttributeStore()->setAttribute<DtVector>("weaponLocation", aimObsGeocLoc);//geoc
		//playerAttributeStore()->setAttribute<DtVector>("localPosition", aimObsLocalLoc);//local

		////Set Orientation
		//playerAttributeStore()->setAttribute<DtTaitBryan>("weaponOrientation", aimObsGeocOri);//geoc
		//playerAttributeStore()->setAttribute<DtTaitBryan>("localOrientation", aimObsLocalOri);//local




	std::cout << "entity()->nextFrameWorldPosition(): " << entity()->nextFrameWorldPosition() << std::endl;
	std::cout << "calculateGunOffset: " << calculateGunOffset().string() << std::endl;
	std::cout << "myTriggerWorldPosition: " << myTriggerWorldPosition << std::endl;


}



	//커맨드를 입력하면 vremsg를 보내고 그걸 handle함수로 수신받는 구조
	//※주의: 격발을 여기서 처리해줘도되는데 만약에 다른 무기들고있는데 이쪽으로 격발신호들어오면 이 무기가 격발됨
	if (reloadPkt.magStatus == 0)
	{
		std::cout << "WeaponFire!!! \n";

		std::cout << "owner->uuid():" << entity()->uuid().uuidString() << std::endl;

		std::string fireCommand = "ForceFire " + entity()->uuid().uuidString();
		std::cout << "fireCommand: " << fireCommand << std::endl;
		DtConsoleCommandManager::globalConsoleCommandManager()->runCommand(fireCommand);
	}


	if (reloadPkt.magStatus == 0)
	OnFire();




		void MyHumanGunJoyActuator::SetWeaponState(DtWeaponState state)
{
	DtRwIndividualLifeFormWeaponStateType type;
	type.setValue(state);

	DtSetLifeFormWeaponStateRequest req;
	req.setLifeFormWeaponState(type);

	DtSetDataRequestMessage dataReqMsg;
	dataReqMsg.setSetDataRequest(&req);
	entity()->setDataManager()->executeSetDataRequest(dataReqMsg);

}


void MyHumanGunJoyActuator::ForceReload()
{

	//테스트 필요



	
	//// empty magazine to implicitly force a reload
	//int roundsLeft = myProcessState->munitionLoader().roundsLeftInMagazine();
	//depleteResource(roundsLeft);
	//myProcessState->munitionLoader().useRounds(roundsLeft);
}


*/
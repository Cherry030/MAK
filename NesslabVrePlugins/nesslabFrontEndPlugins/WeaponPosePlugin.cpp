
#include "WeaponPosePlugin.h"
#include "vreCommonComponents/stateDefines.h"
#include "vrvCore/DtDe.h"
#include "vrvCore/DtDriverManager.h"
#include "vrvCore/DtInputDriver.h"

#include "Nesslab/nesslabUdpServer.h"
#include "Nesslab/nesslabUdpClient.h"
#include "Nesslab/nesslabCommon.h"


using namespace nesslab_common;


//ScreenPlguin.cpp
extern bool gInitScreenPlugin;
extern std::string weaponAimObsName;
extern std::string aimOriginObsName;

//WeaponPlugin.cpp
//extern std::vector<WeaponInfo> gMyWeapons;
//extern std::uint8_t gCurrentWeaponIndex;

//TDMMovePlugin.cpp(Manipulator)
extern float gTopoHeadingRad;



//UDP_Unicast(YSU -> Sub_VRIS -> VRIS), Port = 13101
#pragma pack(push, 1)
struct WeaponPoseDataPacket
{

	PacketHeader header;

	//Payload
	float posX = 0;
	float posY = 0;
	float posZ = 0;

	//quaternion -> Euler(radian)
	float roll	= 0;//x
	float pitch = 0;//y
	float yaw	= 0;//z


	PacketTrailer trailer;


	WeaponPoseDataPacket()
	{
		//header.deviceID = 0;//Ifes, Sfes
		//header.msgID	  = 0;
		header.msgType = 0;//Reserved
		header.length = sizeof(WeaponPoseDataPacket);
	}

};
#pragma pack(pop)





namespace nesslab_frontend_plugins {

	namespace {

		//weaponPose
		nesslabUdpServer* weaponPoseServer = nullptr;
		int					 weaponPosePort = -1;//13101
		WeaponPoseDataPacket weaponPosePkt;

		std::vector<BYTE>	weaponPoseRecvBuffer{};


		PacketHandlers handlers{};

	}

	
	//Sim Monitoring(테스트용)
	namespace
	{

		nesslabUdpClient*	testServer = nullptr;
		std::string			testServerIP = "127.0.0.1";
		int					testServerPort = 9999;



		/*
		float posX = 0;
		float posY = 0;
		float posZ = 0;
		//quaternion -> Euler(radian)
		float roll	= 0;//x
		float pitch = 0;//y
		float yaw	= 0;//z
		*/

		float recvData[6]{};

		//테스트 프로그램 사용안할거면 false 처리해주기
		bool	enableSimMonitoring = true;

		void OnTestDataReceived(const std::uint8_t* data, int len)
		{
			memcpy(recvData, data, sizeof(recvData));
		}



	}



	WeaponPosePlugin::WeaponPosePlugin()
		:DtHumanWeaponPoseLogic()
	{
		std::cout << "[WeaponPosePlugin][Trace] Entering function: Constructor()" << std::endl;
	}

	WeaponPosePlugin::~WeaponPosePlugin()
	{
		std::cout << "[WeaponPosePlugin][Trace] Entering function: Destructor()" << std::endl;
	}


	bool WeaponPosePlugin::initialize(makVre::DtPlayerStation* player, makVre::DtInitTable& config)
	{

		//do base class init.  Shouldn't fail
		if (!DtPlayerComponent::initialize(player, config))
		{
			std::cout << "[WeaponPosePlugin][Trace]do base class init.  Shouldn't fail \n";
			return false;
		}
		std::cout << "[WeaponPosePlugin][Trace] Entering function: initialize()" << std::endl;


		InitPlugin();



		return true;
	}


	void WeaponPosePlugin::shutdown()
	{
		std::cout << "[WeaponPosePlugin][Trace] Entering function: shutdown()" << std::endl;

		if (netThread.joinable())
			netThread.join();

		if (weaponPoseServer != nullptr)
		{
			weaponPoseServer->ServerStop();
			delete weaponPoseServer;
			weaponPoseServer = nullptr;
		}
	}


	/*
					메모
		Location offset +1 => 현실 1m 정도

		VR - Link에서 지형 좌표계(topographic frame)
		+ X축: 북쪽
		+ Y축 : 동쪽  -> Motive랑 부호 반대
		+ Z축: 아래쪽 -> Motive랑 부호 반대(※※ + 축이 아래쪽 주의하기)


		옵저버 2개존재
		- 실제 조준방향 옵저버
		- 실제 조준방향 옵저버의 원점 옵저버(위치,회전 고정)(캐릭터 원점에 부착)

	*/




	//플러그인 사용안할때는 조준점을 앞으로 처리
	void WeaponPosePlugin::tick(double dt)
	{

		// Flag that shows if the screen plugin observer has been created.
		if (!gInitScreenPlugin)
			return;


		//Init Observer
		if (!isObserverReady)
		{

			//무기 조준점 원점 옵저버
			weaponAimOriginObs = myDe->driverManager().inputDriver().findObserverByName(aimOriginObsName)->agent().findObject();

			//실제 조준점 옵저버
			weaponAimObs = myDe->driverManager().inputDriver().findObserverByName(weaponAimObsName)->agent().findObject();

			if (weaponAimOriginObs == nullptr || weaponAimObs == nullptr)
				return;

			isObserverReady = true;
		}



		//if (!isMotiveConnected)
		//{
		//	std::cout << "tick \n";
		//	//WeaponPosePlugin::updateWeaponPosition()
		//	WeaponPosePlugin::updateWeaponPosition();
		//	WeaponPosePlugin::tick(dt);
		//	return;
		//}



		//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡWeapon Aim Origin Observerㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
		// 무기 조준점 원점 옵저버의 위치,회전값 구하기
		//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
		//weapon Aim Origin Observer(Local Location)
		DtVector originObsLocalLoc;
		weaponAimOriginObs->getLocation(originObsLocalLoc[0], originObsLocalLoc[1], originObsLocalLoc[2]);
		//std::cout << "originObsLocalLoc-> " << originObsLocalLoc.string() << std::endl;


		//weapon Aim Origin Observer(Geoc Location)
		DtVector originObsGeocLoc;
		myDe->sharedState().coordinateSystem().localToNetPos(originObsLocalLoc, originObsGeocLoc);
		//std::cout << "originObsGeocLoc-> " << originObsGeocLoc.string() << std::endl;


		//weapon Aim Origin Observer(local Orientation)
		double psi, theta, phi;
		weaponAimOriginObs->getOrientation(psi, theta, phi);
		DtTaitBryan originObsLocalOri(psi, theta, phi);
		//std::cout << "originObsLocalOri-> " << originObsLocalOri.string() << std::endl;


		//weapon Aim Origin Observer(Geoc Orientation)
		DtTaitBryan originObsGeocOri;
		myDe->sharedState().coordinateSystem().localToNetOri(
			originObsLocalLoc, originObsLocalOri, originObsGeocOri);
		//originObsLocalLoc, DtTaitBryan(psi, theta, phi), originObsGeocOri);

		//std::cout << "originObsGeocOri-> " << originObsGeocOri.string() << std::endl;


		//geocLocation->geodCoord, 지구중심 좌표에 해당하는 위도/경도/고도 좌표
		DtGeodeticCoord originObsGeod;//geod Coord
		originObsGeod.setGeocentric(originObsGeocLoc);


		//Geoc -> Topo coordinates
		//Get GeocToTopo coordinates, DtCoordTransform을 초기화하여 지구 중심 좌표계와 위도 및 경도로 정의된 특정 지형 좌표계 간의 변환을 제공		
		DtCoordTransform originObsGeocToTopo;//무기 원점 옵저버 기준 geocToTopoCoord
		DtGeocToTopoTransform(originObsGeod.lat(), originObsGeod.lon(), &originObsGeocToTopo);


		/*
		DtCoordTransform
		- 위치, 벡터 및 방향을 한 직교 좌표계에서 다른 직교 좌표계로 변환하는 데 사용
		- 지구 중심 좌표계와 지형 좌표계 간의 변환에 사용

		DtGeocToTopoTransform()
		지구 중심 좌표계(geoc) ↔ 어떤 기준점에서의 topo 좌표계 변환 객체(DtCoordTransform)를 초기화해주는 함수
		DtGeodeticCoord 매개변수가 기준이 될 지점의 위도/경도
		*/


		//weapon Aim Origin Observer(Topo Orientation)
		DtTaitBryan originObsTopoEuler;
		originObsGeocToTopo.eulerTrans(originObsGeocOri, &originObsTopoEuler);
		//double heading = originObsTopoEuler.psi(); double pitch	 = originObsTopoEuler.theta(); double roll	 = originObsTopoEuler.phi();
		//std::cout << "originObsTopoEuler-> " << originObsTopoEuler.string() << std::endl;



		//Topo Location(사용x 변환방법 저장용)(문서보고했는데 정확한지는 테스트필요)
		//지구 중심좌표 -> 지형 좌표로 변환
		//originObsGeocToTopo로 변환할때 originObsGeod.alt()값을 계산안해줘서 0,0,-9.17이런식으로 나오는듯?
		DtVector originObsTopoLocation;
		originObsGeocToTopo.coordTrans(originObsGeocLoc, originObsTopoLocation);
		//std::cout << "originObsTopoLocation-> " << originObsTopoLocation.string() << std::endl;

		//DtVector vecTransTopo;
		//originObsGeocToTopo.vecTrans(originObsGeocLoc, vecTransTopo);
		//std::cout << "vecTransTopo-> " << vecTransTopo.string() << "\n" << std::endl;


		//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡAdd Offset(weaponPoseData)ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
		// Motive or TestProgram에서 들어오는 데이터를 Mak에서 사용할 데이터로 변환
		//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ


		if (enableSimMonitoring)
		{		
			//테스트 프로그램 사용(값*0.01해서 송신)

			float testPosX = recvData[0];
			float testPosY = recvData[1];
			float testPosZ = recvData[2];

			float testRoll  = recvData[3];
			float testPitch = recvData[4];
			float testYaw   = recvData[5];
			

			//if (gMyWeapons.size() != 0 && gMyWeapons[gCurrentWeaponIndex].weaponType == WeaponType::GRENADE)
			if (playerAttributeStore()->getAttributeOr<makVre::Equipment>("currentEquipment", makVre::NONE) == makVre::GRENADE)
			{

				//수류탄

				weaponPosePkt.posX = testPosX;
				weaponPosePkt.posY = testPosY;
				weaponPosePkt.posZ = testPosZ;

				weaponPosePkt.roll = testRoll;//topoOri.phi()
				weaponPosePkt.pitch = testPitch;//topoOri.theta()
				weaponPosePkt.yaw = testYaw + gTopoHeadingRad;
			}
			else
			{
				//소총,권총

				weaponPosePkt.posX = testPosX;
				weaponPosePkt.posY = testPosY;
				weaponPosePkt.posZ = testPosZ;

				weaponPosePkt.roll  = testRoll;
				weaponPosePkt.pitch = testPitch;
				weaponPosePkt.yaw   = testYaw;

			}

		}
		else
		{

			//테스트프로그램 사용x, 실제 수류탄 고정값
			if (playerAttributeStore()->getAttributeOr<makVre::Equipment>("currentEquipment", makVre::NONE) == makVre::GRENADE)
			{
				weaponPosePkt.posX = 0;
				weaponPosePkt.posY = 0;
				weaponPosePkt.posZ = -2;

				weaponPosePkt.roll = 0;//topoOri.phi()
				weaponPosePkt.pitch = -0.2;//topoOri.theta() //30 => 0.5236
				weaponPosePkt.yaw = gTopoHeadingRad;//manipulator heading(0~2π)
			}
		}



		//topo -> Geoc
		DtCoordTransform topoToGeoc;
		topoToGeoc.setByInverse(originObsGeocToTopo);


		//Offset(topo Location)
		DtVector topoLocOffset(weaponPosePkt.posX, weaponPosePkt.posY, weaponPosePkt.posZ);
		//std::cout << "topoLocOffset  -> " << topoLocOffset.string() << std::endl;


		//Offset(geoc Location)
		DtVector geocLocationOffset;
		//topoToGeoc.coordTrans(topoLocOffset, geocLocationOffset);	
		topoToGeoc.vecTrans(topoLocOffset, geocLocationOffset);
		//std::cout << "geocLocationOffset ->" << geocLocationOffset.string() << std::endl;


		//weapon Aim Observer(Geoc Location) 
		DtVector aimObsGeocLocation = originObsGeocLoc + geocLocationOffset;
		//std::cout << "aimObsGeocLocation-> " << aimObsGeocLocation.string() << std::endl;


		//weapon Aim Observer(Local Location)
		DtVector aimObsLocalLoc;
		myDe->sharedState().coordinateSystem().netToLocalPos(aimObsGeocLocation, aimObsLocalLoc);
		//std::cout << "aimObsLocalLoc ->: " << aimObsLocalLoc.string() << std::endl;




		//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡSet Location, Orientationㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
		// 실제 격발 방향 처리하는 옵저버에 위치,회전값 적용
		// 추후에 VR로 훈련을 진행하는경우 지형 기울기값을 더해줘야할수도, 산악전투 생각하면될듯 현재는 스크린이라 상관없을 듯
		//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

		//Set weapon Aim Observer(Local Location)
		weaponAimObs->setLocation(aimObsLocalLoc[0], aimObsLocalLoc[1], aimObsLocalLoc[2]);


		//Set weapon Aim Observer(Topo Orientation)
		//yaw(psi), pitch(theta), roll(phi)  => ZYX
		weaponAimObs->setTopographicOrientation(weaponPosePkt.yaw, weaponPosePkt.pitch, weaponPosePkt.roll);


		//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡWeapon Aim Observerㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
		//무기 조준하는 옵저버의 위치,회전값 구해서 playerAttributeStore에서 weaponLocation,localPosition,weaponOrientation,localOrientation 값 적용
		
		//Set Location
		//playerAttributeStore()->setAttribute<DtVector>("weaponLocation", weaponGeocLocation);//geoc		
		//playerAttributeStore()->setAttribute<DtVector>("localPosition", weaponLocalLocation);//local

		//Set Orientation
		//playerAttributeStore()->setAttribute<DtTaitBryan>("weaponOrientation", weaponGeocOri);//geoc
		//playerAttributeStore()->setAttribute<DtTaitBryan>("localOrientation", weaponLocalOri);//local
		//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

		//weapon Aim Observer(local Location)
		DtVector weaponLocalLocation;
		weaponAimObs->getLocation(weaponLocalLocation[0], weaponLocalLocation[1], weaponLocalLocation[2]);
		//std::cout << "weaponLocalLocation-> " << weaponLocalLocation.string() << std::endl;


		//weapon Aim Observer(Geoc Location)
		DtVector weaponGeocLocation;
		myDe->sharedState().coordinateSystem().localToNetPos(weaponLocalLocation, weaponGeocLocation);
		//std::cout << "weaponGeocLocation-> " << weaponGeocLocation.string() << std::endl;


		//weapon Aim Observer(Local Orientation)
		double weaponPsi, weaponTheta, weaponPhi;
		weaponAimObs->getOrientation(weaponPsi, weaponTheta, weaponPhi);
		DtTaitBryan weaponLocalOri(weaponPsi, weaponTheta, weaponPhi);
		//std::cout << "weaponLocalOri-> " << weaponLocalOri.string() << std::endl;



		//weapon Aim Observer(Geoc Orientation)
		DtTaitBryan weaponGeocOri;
		myDe->sharedState().coordinateSystem().localToNetOri(weaponLocalLocation, DtTaitBryan(weaponPsi, weaponTheta, weaponPhi), weaponGeocOri);
		//std::cout << "weaponGeocOri-> " << weaponGeocOri.string() << std::endl;


		//weapon Aim Observer(Topo Orientation)
		DtTaitBryan weaponTopoOri;
		originObsGeocToTopo.eulerTrans(weaponGeocOri, &weaponTopoOri);
		//std::cout << "weaponTopoOri -> " << weaponTopoOri.string() << std::endl;


		DtVector weaponTopoLocation;
		originObsGeocToTopo.coordTrans(weaponGeocLocation, weaponTopoLocation);
		//std::cout << "weaponTopoLocation -> " << weaponTopoLocation.string() << std::endl;


		//Offset
		DtVector weaponOffset = weaponTopoLocation - originObsTopoLocation;
		//std::cout << "weaponLocationOffset -> " << weaponOffset.string() << std::endl;


		//Set Location
		playerAttributeStore()->setAttribute<DtVector>("weaponLocation", weaponGeocLocation);//geoc		
		playerAttributeStore()->setAttribute<DtVector>("localPosition", weaponLocalLocation);//local

		//Set Orientation
		playerAttributeStore()->setAttribute<DtTaitBryan>("weaponOrientation", weaponGeocOri);//geoc
		playerAttributeStore()->setAttribute<DtTaitBryan>("localOrientation", weaponLocalOri);//local



		//Console
		{

			//std::cout << "aimObsGeocLoc  => x:" << aimObsGeocLoc[0] << ", Y: " << aimObsGeocLoc[1] << ", Z: " << aimObsGeocLoc[2] << std::endl;
			//std::cout << "aimObsLocalLoc => x:" << aimObsLocalLoc[0] << ", Y: " << aimObsLocalLoc[1] << ", Z: " << aimObsLocalLoc[2] << std::endl;
			//std::cout << "aimObsGeocOri psi: " << aimObsGeocOri.psi() << ",theta: " << aimObsGeocOri.theta() << ",phi: " << aimObsGeocOri.phi() << std::endl;
			//std::cout << "aimObsLocalOri psi: " << aimObsLocalOri.psi() << ",theta: " << aimObsLocalOri.theta() << ",phi: " << aimObsLocalOri.phi() << std::endl;


			////add Location Offset(Topo)
			//DtVector addTopoOffset;
			//originObsGeocToTopo.vecTrans(geocLocOffset, addTopoOffset);			
			//std::cout << "Add Location Offset => x:" << addTopoOffset.x() << ",y: " << addTopoOffset.y() << ",z: " << addTopoOffset.z() << std::endl;


			////Weapon Observer Orientation  + add Orientation  Offset(Topo)
			//std::cout << "Weapon Observer Orientation =>  psi: " << weaponTopoOri.psi() << ",theta: " << weaponTopoOri.theta() << ",phi: " << weaponTopoOri.phi() << std::endl;

		}




		//SendData(vris->SimulationMonitoringProgram, 테스트용)
		if (enableSimMonitoring)
		{
			float posXDiff = weaponOffset.x();
			float posYDiff = weaponOffset.y();
			float posZDiff = weaponOffset.z();

			float rollDiff  = weaponTopoOri.phi();
			float pitchDiff = weaponTopoOri.theta();
			float yawDiff   = weaponTopoOri.psi();

			uint8_t sendData[sizeof(float) * 12];
			float values[12] =
			{
				weaponPosePkt.posX,
				weaponPosePkt.posY,
				weaponPosePkt.posZ,

				weaponPosePkt.roll,
				weaponPosePkt.pitch,
				weaponPosePkt.yaw,

				posXDiff,
				posYDiff,
				posZDiff,

				rollDiff,
				pitchDiff,
				yawDiff

			};

			memcpy(sendData, values, sizeof(values));
			testServer->SendData(sendData, sizeof(sendData));
		}

	}



	void WeaponPosePlugin::InitPlugin()
	{

		weaponPoseRecvBuffer.clear();
		handlers.clear();


		//Get ConfigFile Data
		{

			weaponPosePort = GetPrivateProfileInt("YSU_WeaponPose", "weaponPosePort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());


			if (weaponPosePort == INI_INT_NOT_FOUND_DEFAULT)
				LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[WeaponPosePlugin][Error] Failed to read data from INI file. Using default value.\n";


			std::cout << "[WeaponPosePlugin][Debug] weaponPosePort: " << weaponPosePort << std::endl;

		}




		//Store packet data(파싱, 콜백 처리용)
		{
			//YSU -> VRIS
			// 인프라 병기 정보 패킷
			RegisterPacketHandler<WeaponPoseDataPacket>(handlers, (uint8_t)DeviceID::Ifes, (uint8_t)WeaponPoseMsgID::InfraWeaponPoseMsgID, weaponPosePkt,
				[this](const WeaponPoseDataPacket& packet)
				{
					OnWeaponPosePkt(packet);
				}
			);


			//세미 인프라 병기 정보 패킷
			RegisterPacketHandler<WeaponPoseDataPacket>(handlers, (uint8_t)DeviceID::Sfes, (uint8_t)WeaponPoseMsgID::SemiInfraWeaponPoseMsgID, weaponPosePkt,
				[this](const WeaponPoseDataPacket& packet)
				{
					OnWeaponPosePkt(packet);
				}
			);

		}

		//Create and start TCP/UDP
		{

			netThread = std::thread([this]() {

				//YSU --(UDP_Uni)--> Sub_VRIS --(UDP_Uni)--> VRIS),
				weaponPoseServer = new nesslabUdpServer(weaponPosePort, nesslabUdpServer::PacketTransmissionMode::Unicast);
				weaponPoseServer->SetOnDataReceived(
					[this](unsigned char* data, int len)
					{
						OnDataReceived(data, len);
					}
				);
				weaponPoseServer->ServerStart();


				//Test
				if (enableSimMonitoring)
				{
					testServer = new nesslabUdpClient(testServerPort, testServerIP, nesslabUdpClient::PacketTransmissionMode::Unicast);
					testServer->SetOnDataReceived(OnTestDataReceived);
					testServer->Connect();
				}

			});

		}

	}





	void WeaponPosePlugin::OnDataReceived(unsigned char* data, int len)
	{
		//std::cout << "[WeaponPosePlugin][Debug] Received Data from YSU, Data Length : " << len << ", Data :";
		//std::cout << std::hex << std::uppercase;
		//for (int i = 0; i < len; ++i)
		//	std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
		//std::cout << std::dec << std::endl;
		//isMotiveConnected = (isMotiveConnected == false) ? true : false;

		weaponPoseRecvBuffer.insert(weaponPoseRecvBuffer.end(), data, data + len);

		ParseReceiveBuffer(handlers, weaponPoseRecvBuffer);

	}


	void WeaponPosePlugin::OnWeaponPosePkt(const WeaponPoseDataPacket& packet)
	{
		//std::cout << "[WeaponPosePlugin][Debug] PacketData PosX: " << packet.posX << ", PosY: " << packet.posY << ", PosZ: " << packet.posZ
		//	<< ", Roll: " << packet.roll << ", Pitch: " << packet.pitch << ", Yaw: " << packet.yaw << std::endl;

	}


}
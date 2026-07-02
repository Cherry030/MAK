#include "TerrainSlope.h"

#include <vrfobjcore/physicalWorld.h>
#include <vrfutil/profiler.h>
#include <tdbutil/mathUtilities.h>
#include <terrainCS/coordSystem.h>
#include <vrvUtil/DtCoordinateConverter.h>
#include <vreMessageManager/forwardMessage.h>
#include <cmath>
#include <vreUtil/logger.h>

#include "Nesslab/nesslabUdpClient.h"
#include "Nesslab/nesslabTcpClient.h"
#include "Nesslab/nesslabCommon.h"


using namespace makVrf;
using namespace makVrv;
using namespace nesslab_common;


/* 메모 => 추후 필요한 데이터 있으면 사용하기

//SetLocation
entity()->setNextFrameLocalPosition(*test);

//GetLocation
entity()->nextFrameLocalPosition()

//maxSpeed
DtVrfMovingObjectStateRepository* movingSR = localStateRepository()->as<DtVrfMovingObjectStateRepository>();
double maxSpeed = -1;
double maxReverseSpeed = -1;
if (movingSR != nullptr)
{
	maxSpeed = movingSR->maxSpeed();
	maxReverseSpeed = movingSR->maxReverseSpeed();

cout << "currentSpeed" << movingSR->currentSpeed() << endl;
cout << "forwardSpeed" << movingSR->forwardSpeed() << endl;
cout << "localSpeed" << movingSR->localSpeed() << endl;
cout << "worldSpeed" << movingSR->worldSpeed() << endl;
cout << "maxSpeed => " << maxSpeed << endl;
cout << "maxReverseSpeed => " << maxReverseSpeed << endl;
}


//Get
"Location: " << entity()->nextFrameLocalPosition()
"Orientation: " << entity()->nextFrameLocalOrientation()
"SoilType (Record): " << record.surface().soilTypeString()
"SoilType (surf): " << surf.soilTypeString()
"SoilType (GetSoilType): " << getSoilTypeString(surf)
"surfaceCharact: " << surf.surfaceCharacteristicsString()
"SurfaceNormal: " << record.normal()
"entity()->speed(): " << entity()->speed()
"entity()->forwardSpeed(): " << entity()->forwardSpeed()
"maxSpeed: " << maxSpeed
//"percentOfMaxSpeed: " << percentOfMaxSpeed
" Quaternion - (" << tmp_Quat.x() << " , " << tmp_Quat.y() << " , " << tmp_Quat.z() << " , " << tmp_Quat.w() << ")" << "\n"


*/


namespace nesslab_backend_plugins {


		//VRIS --UDP_Unicast--> SubVRIS, Port = 13100
#pragma pack(push, 1)
		struct TerrainSlopePacket
		{

			PacketHeader header;

			//payload
			float	xRollDegree  = 0;//-30 ~ 30
			float	yPitchDegree = 0;//-30 ~ 30

			PacketTrailer trailer;

			TerrainSlopePacket()
			{
				header.deviceID = (int)DeviceID::Vris;
				header.msgID	= (uint8_t)VrisMsgID::TerrainSlopeMsgID;
				header.msgType	= 0;//Reserved
				header.length	= sizeof(TerrainSlopePacket);
			}
		};
#pragma pack(pop)


		namespace {

			//VRIS -> UDP_Client -> Sub_VRIS
			nesslabUdpClient*	tdmUdpUcastClient = nullptr;
			std::string			tdmIP = "127.0.0.1";
			int					tdmPort = -1;
			TerrainSlopePacket	terrainSlopePkt;


			//SimTreadmill_Test
			nesslabUdpClient*	simUcastClient = nullptr;
			std::string			simIP = "127.0.0.1";
			int					simPort = 9996;
			bool				enableSimProgram = true;

		}

		
		//Constructor => Enage Human 캐릭터 생성하면 실행
		TerrainSlopePlugin::TerrainSlopePlugin(const DtString& name,
			DtLocalObject* owner,
			DtSimulationServices* simManager,
			DtComponentDescriptor* desc,
			DtReaderWriterRegistry* parentRegistry)
			: DtActuatorComponent(name, owner, simManager, desc, parentRegistry)
		{			
			std::cout << "[TerrainSlopePlugin][Trace] Constructor " << std::endl;
		}

		

		// destructor => Human 아닌 오브젝트를 생성하거나 다른 인간 캐릭터 생성하면 실행
		TerrainSlopePlugin::~TerrainSlopePlugin()
		{
			std::cout << "[TerrainSlopePlugin][Trace] Destructor " << std::endl;

			if (netThread.joinable())
				netThread.join();

			if (tdmUdpUcastClient != nullptr)
			{				
				tdmUdpUcastClient->Disconnect();
				delete tdmUdpUcastClient;
				tdmUdpUcastClient = nullptr;
			}


			//테스트용
			if (simUcastClient != nullptr)
			{
				simUcastClient->Disconnect();
				delete simUcastClient;
				simUcastClient = nullptr;
			}


		}


		bool TerrainSlopePlugin::init()
		{
			if (!DtActuatorComponent::init())
				return false;

			InitPlugin();



			return true;
		}

		// This returns a string from compTypes.h, and identifies the type of component
		const char* TerrainSlopePlugin::type() const
		{

			return TERRAIN_SLOPE_PLUGIN_TYPE;
		}



		void TerrainSlopePlugin::InitPlugin()
		{

			//Get ConfigFile Data
			{

				char databuf[256] = {};

				//TDM
				GetPrivateProfileString("Treadmill_Slope", "tdmSlopeIP", INI_STRING_NOT_FOUND_DEFAULT.c_str(), databuf, sizeof(databuf), CONFIG_FILE_PATH.c_str());
				tdmIP	= databuf;
				tdmPort = GetPrivateProfileInt("Treadmill_Slope", "tdmPort", INI_INT_NOT_FOUND_DEFAULT, CONFIG_FILE_PATH.c_str());

				if (tdmPort == INI_INT_NOT_FOUND_DEFAULT || tdmIP == INI_STRING_NOT_FOUND_DEFAULT)
					LOG_WARN(LOG_WARN_CHANNEL_NAME) << "[TerrainSlopePlugin][Error] Failed to read data from INI file. Using default value.\n";

				std::cout << "[TerrainSlopePlugin][Debug] tdmIP: " << tdmIP << ", tdmPort: " << (int)tdmPort << std::endl;
			}		


			//Create and start TCP/UDP
			{
				netThread = std::thread([this]() {

					tdmUdpUcastClient = new nesslabUdpClient(tdmPort, tdmIP.data(), nesslabUdpClient::PacketTransmissionMode::Unicast);
					tdmUdpUcastClient->Connect();


					if (enableSimProgram)
					{
						simUcastClient = new nesslabUdpClient(simPort, simIP, nesslabUdpClient::PacketTransmissionMode::Unicast);
						simUcastClient->Connect();
					}


				});
			}

		}



		bool AreDoublesEqual(double a, double b, double epsilon = 1e-8)
		{
			return std::fabs(a - b) < epsilon;
		}


#include "vrfobjcore/physicalWorld.h"

		constexpr float TDM_ROLL_MAX_DEG = 30.0;
		constexpr float TDM_PITCH_MAX_DEG = 30.0;
		static int		tickCount	= 0;

		void TerrainSlopePlugin::tick()
		{
			//if (!IsPlayerControlled("Human")) return;

			// Returns true if the input is exactly zero (uses operator==).
			if (DtIsNotZero(dT()) == false)
				return;

			//Tracy 프로파일러에서 현재 스코프(블록/함수)를 타이밍 측정 구간(zone)으로 찍어주는 매크로
			DtPROFILEzone;


			DtVector dbLocation = entity()->nextFrameLocalPosition();
			DtVrfChordIntersectionRecord record;

			// 교차점에 대한 데이터가 더 있어야함
			DtTerrainIntersectStatus interStatus;
			DtTerrainIntersectOptions interOption;
			//double heightAtLocation;


			/*
			
			   //! Same as above closestTerrainLocation, but accepts location as a vector.
				virtual bool closestTerrainLocation(const DtVector& databaseLocation,
				DtVrfChordIntersectionRecord& record,
				DtTerrainIntersectStatus& status, const DtTerrainIntersectOptions& options) const;
			
			*/

			
			//physicalWorld()->terrainHeightAndSurface(entity()->nextFrameLocalPosition(), heightAtLocation, record, interStatus, interOption);//제일 높은 교차점의 높이 반환
			physicalWorld()->closestTerrainLocation(dbLocation, record, interStatus, interOption);//VRE2.2



			DtVector intersectionLocation = record.intersectionVector();
			DtVector normal = record.normal();
			GetTerrainOrientation(intersectionLocation, normal);
			

			//std::cout << "[TerrainSlopePlugin][Debug] treadmillRollDeg_: " << treadmillRollDeg << ", treadmillPitchDeg_: " << treadmillPitchDeg << std::endl;
			//Console
			//if (tickCount >= 500)
			if (tickCount >= 50)
			{
				std::cout << "[TerrainSlopePlugin][Debug] treadmillRollDeg_: " << treadmillRollDeg << ", treadmillPitchDeg_: " << treadmillPitchDeg << std::endl;
				tickCount = 0;
			}
			tickCount++;
			

			//ROLL, PITCH Degree(-30 ~ 30)
			SendDataToTreadmill(treadmillRollDeg, treadmillPitchDeg);


			//테스트용
			if (enableSimProgram)
			{
				//float values[2] = { treadmillRollDeg, treadmillPitchDeg };
				//unsigned char data[sizeof(values)];
				//memcpy(data, values, sizeof(values));

				const size_t dataSize = sizeof(float) * 2;
				unsigned char data[dataSize];
				memcpy(data, &treadmillRollDeg, sizeof(float));
				memcpy(data + sizeof(float), &treadmillPitchDeg, sizeof(float));

				simUcastClient->SendData(data, dataSize);
			}
			
		}



		DtSimComponent* TerrainSlopePlugin::creator(const DtString& name,
			DtLocalObject* owner,
			DtSimulationServices* simManager,
			DtComponentDescriptor* desc,
			DtReaderWriterRegistry* parentRegistry)
		{

			return new TerrainSlopePlugin(name, owner, simManager, desc, parentRegistry);
		}



		void TerrainSlopePlugin::GetTerrainOrientation(const DtVector& intersection, const DtVector& normal)
		{

			DtDcm newLocalDcm;
			DtDcm topoToLocalDcm;

			//intersection(local)
			const DtVector localPosition = intersection;
			physicalWorld()->coordinateSystem()->topoToLocal(localPosition, topoToLocalDcm);
		
			//heading = 0 => 캐릭터 회전에 상관없이 기준 방향 정북
			DtCalculateOrientation(normal, 0, topoToLocalDcm, newLocalDcm);//body(normal,heading)→local 회전행렬			
	
			//local -> topo 
			DtDcm localToTopoDcm;
			DtDcmTranspose(topoToLocalDcm, localToTopoDcm);
			
			//body(nomal,heading) -> topo
			DtDcm bodyToTopoDcm;
			DtDcmDcmMul(localToTopoDcm, newLocalDcm, bodyToTopoDcm);//localToTopoDcm * newLocalDcm //body -> local -> topo

			
			//body→topo(ref)
			DtTaitBryan newLocalTb;
			DtBodyToRef_to_Euler(bodyToTopoDcm, &newLocalTb);
			
			
			//Radian
			double tdmRollRad  = newLocalTb.phi();	//X
			double tdmPitchRad = newLocalTb.theta();//Y
			//double tdmYawRad = newLocalTb.psi();//Z


			//Degree
			double tdmRollDeg  = (tdmRollRad * 180.0)  / M_PI;
			double tdmPitchDeg = (tdmPitchRad * 180.0) / M_PI;
			//double tdmYawDeg = (treadmillYaw * 180.0) / M_PI;


			treadmillRollDeg  = tdmRollDeg;
			treadmillPitchDeg = tdmPitchDeg;

			//treadmillRollDeg  = Clamp<float>(tdmRollDeg,  -TDM_ROLL_MAX_DEG,  TDM_ROLL_MAX_DEG);
			//treadmillPitchDeg = Clamp<float>(tdmPitchDeg, -TDM_PITCH_MAX_DEG, TDM_PITCH_MAX_DEG);


			//std::cout << "[TerrainSlopePlugin][Debug] treadmillRollDeg: " << treadmillRollDeg << ", treadmillPitchDeg: " << treadmillPitchDeg << std::endl;

		}


		bool TerrainSlopePlugin::IsPlayerControlled(const std::string& role)
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
		


		//참고용 코드
		DtQuaternion TerrainSlopePlugin::getIntersectionQuat(const DtVector& intersection, const DtVector& normal, Coordinate_System* coordinateSystem, DtLocalObject* myEntity)
		{
			using namespace std;

			//[1] playerheading
			DtDcm newLocalOrientation;
			DtDcm topoToLocalDcm;
			coordinateSystem->topoToLocal(intersection, topoToLocalDcm);

			
			DtCalculateOrientation(normal, myEntity->nextFrameHeading(), topoToLocalDcm, newLocalOrientation);


			//localOri -> eulerOri
			DtTaitBryan localOrientationTb;
			DtBodyToRef_to_Euler(newLocalOrientation, &localOrientationTb);

			//Radian
			double yaw		= localOrientationTb.psi();		//myPsi		=> about reference z
			double pitch	= localOrientationTb.theta();	//myTheta	=> about intermediate y
			double roll		= localOrientationTb.phi();		//myphi		=> about body x

			//Degree
			double dYaw		= (yaw * 180.0)   / M_PI;
			double dPitch	= (pitch * 180.0) / M_PI;
			double dRoll	= (roll * 180.0)  / M_PI;


			cout << "[TDM_Topo_Plugin] PlayerHeading(Euler) \n";
			cout << "[PlayerHeading]RADIAN => "		<< "Roll: " << roll			<< ", Pitch: " << pitch			<< ", Yaw: " << yaw			<< endl;
			cout << "[PlayerHeading]DEGREE => "		<< "Roll: " << dRoll		<< ", Pitch: " << dPitch		<< ", Yaw: " << dYaw		<< endl;
			//cout << "[PlayerHeading]DEGREE+180 => " << "Roll: " << dRoll + 180	<< ", Pitch: " << dPitch + 180	<< ", Yaw: " << dYaw + 180	<< endl;

			std::cout << "\n";


			//[2] Treadmill
			DtDcm treadmill_newLocalOrientation;
			DtDcm treadmill_topoToLocalDcm;//topoToLocal
			coordinateSystem->topoToLocal(intersection, treadmill_topoToLocalDcm);
			

			//Treadmill- player 캐릭터 회전에 상관없이 기준 방향은 정북 : myEntity->nextFrameHeading() => 0로 변경
			DtCalculateOrientation(normal, 0, treadmill_topoToLocalDcm, treadmill_newLocalOrientation);//localOri



			//DtDcm treadmill_newLocalOrientation;
			////local -> dis
			//coordinateSystem->local2dis(treadmill_newLocalOrientation2, treadmill_newLocalOrientation);
			//virtual void local2dis(const DtDcm & in, DtDcm & out) const;



			DtTaitBryan treadmill_localOrientationTb;
			DtBodyToRef_to_Euler(treadmill_newLocalOrientation, &treadmill_localOrientationTb);


			

			//Radian
			double treadmill_yaw	= treadmill_localOrientationTb.psi();	//Z
			double treadmill_pitch	= treadmill_localOrientationTb.theta();	//Y
			double treadmill_roll	= treadmill_localOrientationTb.phi();	//X

			//Degree
			double treadmill_dYaw	= (treadmill_yaw * 180.0) / M_PI;
			double treadmill_dPitch = (treadmill_pitch * 180.0) / M_PI;
			double treadmill_dRoll	= (treadmill_roll * 180.0) / M_PI;

			cout << "[TdmTopoPlugin] Treadmill(Euler) \n";
			cout << "[tdmHeading]RADIAN => " << "Roll: " << treadmill_roll << ", Pitch: " << treadmill_pitch << ", Yaw: " << treadmill_yaw << endl;
			cout << "tdmHeading]DEGREE => " << "Roll: " << treadmill_dRoll << ", Pitch: " << treadmill_dPitch << ", Yaw: " << treadmill_dYaw << endl;
			std::cout << "\n";

			// [Ver 2]
			// Euler -> Quaternion
			DtQuaternion intersectionQuaternion;
			double qx, qy, qz, qw;

			//DtEulerToQuaternionDeg(yaw, pitch, roll, qx, qy, qz, qw);
			DtEulerToQuaternionDeg(treadmill_dYaw, treadmill_dPitch, treadmill_dRoll, qx, qy, qz, qw);


			intersectionQuaternion.setX(qx);
			intersectionQuaternion.setY(qy);
			intersectionQuaternion.setZ(qz);
			intersectionQuaternion.setW(qw);



			return intersectionQuaternion;
		}



		void TerrainSlopePlugin::SendDataToTreadmill(const double& xValue, const double& yValue)
		{
			if (tdmUdpUcastClient == nullptr) return;


			//Set Data
			terrainSlopePkt.xRollDegree  = xValue;
			terrainSlopePkt.yPitchDegree = yValue;


			//Struct -> byte*
			const size_t size = sizeof(terrainSlopePkt);
			unsigned char byteArray[size];
			std::memcpy(byteArray, &terrainSlopePkt, size);

			
			bool result = tdmUdpUcastClient->SendData(byteArray, size);
			if (result)
			{
				//std::cout << "[TerrainSlopePlugin][Debug] Send data to TDM : ";
				//std::cout << std::hex << std::uppercase;
				//for (int i = 0; i < size; ++i)
				//	std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(byteArray[i]) << " ";
				//std::cout << std::dec << std::endl;
			}
		}



}
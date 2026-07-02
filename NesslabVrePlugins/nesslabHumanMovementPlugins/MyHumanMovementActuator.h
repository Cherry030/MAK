#pragma once


#include "vrfExtensions/vreVrfmodel/vreHumanMovementActuator.h"
#include "vrfobjcore/analogIOPort.h"
#include "Nesslab/nesslabCommon.h"
#include <vreMessageManager/vreMessageManager.h>
#include <vreMessageManager/vreMessage.h>


/*
						캐릭터 이동 플러그인(BackEnd)
		
		특징
		- 캐릭터 input값만큼 이동 처리
		(FrontEnd에서 joystick 방식으로 하다가 정확하게 속도 처리가안되서 BackEnd에서 개발)
		
		- ComponentType 경로
		C:\MAK\vrengage2.1.1b\data\simulationModelSets\VR-Engage\vrfSim\systems\movement
		(human-movement
		(component - descriptor - type "vre-human-movement-actuator-descriptor")
		(component - type "vre-human-movement-actuator") => 이 부분


		TODO
		- 캐릭터 Heading 처리
		- 지형 기울기에 따른 이동속도 감소 제거

*/


struct TDMDataPacket;
struct StopAllPluginsRequestPacket;

class MyHumanMovementActuator : makVre::DtVreHumanMovementActuator
{

public:
	MyHumanMovementActuator(const DtString& name,
		DtLocalObject* owner,
		DtSimulationServices* simManager,
		DtComponentDescriptor* desc = 0,
		DtReaderWriterRegistry* parentRegistry = 0);

	virtual ~MyHumanMovementActuator();


	virtual bool init();
	virtual const char* type() const;
	virtual void tick() override;

	static DtSimComponent* creator(const DtString& name,
		DtLocalObject* owner,
		DtSimulationServices* simManager,
		DtComponentDescriptor* desc = 0,
		DtReaderWriterRegistry* parentRegistry = 0);


protected:

	
	//테스트 진행 중
	//void modulateDesiredSpeed(double& newDesiredSpeed, double& newMaxSpeed, double desiredSpeed) override;
	//double calculateMaxSpeed() override;
	//double calculateTerrainPitch(const DtVector& localPosition, double heading, const DtAttachedTerrain& terrain) override;

	//double maxSpeedSoilFactor() const override;
	//DtLifeformStaticPosture restrictPostureForTerrain(DtLifeformStaticPosture desiredPosture) override;
	//double maxSpeedFromFacingError(const double desiredHeading, const double currentHeading) const override;	
	


private:
	void InitPlugin();
	void OnDataReceived(unsigned char* data, int len);
	void OnStopDataReceived(const std::string& client, const uint8_t* data, int len);
	void OnTdmDataPkt(const TDMDataPacket& packet);
	void OnStopAllPluginsPkt(const StopAllPluginsRequestPacket& packet);
	void SetCharacterMovement(float headingDeg, const float& worldSpeedXKmh, const float& worldSpeedYKmh);
	void SendDataToDsms();
	float ConvertManipAngleToMak(float oldAngle);
	void normalizeDirection0To2Pi(double& directionRad);


	double myDesiredSpeed				= 0.0;// 속도 m/s
	double myDesiredMovementDirection	= 0.0;// 방향 rad, 0 ~ 2*pi
	//double myHeading					= 0.0;// rad, 0 ~ 2*pi


	std::thread netThread;
	std::vector<BYTE>   tdmUdpRecvBuffer{};
	std::vector<BYTE>   tdmTcpRecvBuffer{};
	nesslab_common::PacketHandlers packetHandlers{};

	//※테스트 진행 중 데이터들어오면 tick에서 이동처리(PC사용하는 훈련자들은 기존 방식대로 처리) 
	// => isTreadmillActive = true면 키보드 입력x 
	bool isTreadmillActive = false;
	bool stopRequested = false;

	void SendVreMessage();
	makVre::DtVreMessageResult HandleStopPluginMessage(makVre::DtVreMessage* msg);

};
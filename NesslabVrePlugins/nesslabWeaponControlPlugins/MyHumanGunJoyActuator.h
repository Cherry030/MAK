#pragma once

#include "vrfobjcore/vrfProcessStateRepository.h"
#include "vrfExtensions/vreVrfmodel/humanGunJoyActuator.h"
#include "Nesslab/nesslabCommon.h"

using namespace nesslab_common;
struct WeaponReloadPacket;


/*

		총기 재장전 및 탄 개수 조절하는 플러그인

특징
- 현재 탄창에 남은 탄, 전체 탄창 조절가능(+UI업데이트)
- 소총, 권총 각 각 생성자 생성


TODO: 추후에 가능하면 격발기능 추가

 
 
				MyHumanGunJoyActuator 생성 과정

1. 캐릭터.entity파일에서 무기 .sysdef (총기류)
경로 = C:\MAK\vrengage2.1.1b\data\simulationModelSets\VR-Engage\vrfSim
ex)      <componentSystem systemName="weapon" platform="@(system-dir)\weapons\handheld-K2.sysdef">//handheld-K2.sysdef



2. 무기.sysdef 파일에서 system-definition
경로 = C:\MAK\vrengage2.1.1b\data\simulationModelSets\VR-Engage\vrfSim\systems\weapons
		 (system-definition
			(filename "$(system-dir)\weapons\templates\engage-handheld-gun.template_sysdef")//engage-handheld-gun.template_sysdef


3. engage-handheld-gun.template_sysdef 파일에서 main-gun 확인
경로 = C:\MAK\vrengage2.1.1b\data\simulationModelSets\VR-Engage\vrfSim\systems\weapons\templates
	  (main-gun
		 (component-descriptor-type "vre-human-gun-actuator-descriptor")
		 (component-type "human-gun-Joystick-actuator") ------> component-type이름이 plugin.cxx에서 생성하는 Type이름이랑 일치하게 변경(현재는 부모 DtHumanGunJoyActuator Type사용중)
		 (min-tick-period -1.000000)
		 (min-tick-period-variance -1.000000)
		 (periodic-tick-while-paused False)
		 (process-state-repository-name $process-state-name)
		 (process-state-repository-type "MyVreVrfTurretedBalGunPSR")


*/


//const char HUMAN_GUN_JOY_ACTUATOR_PLUGIN_TYPE[] = "MyHumanGunJoyActuator";

class MyHumanGunJoyActuator :public makVre::DtHumanGunJoyActuator
{
public:
	MyHumanGunJoyActuator(const DtString& name,
		DtLocalObject* owner,
		DtSimulationServices* simManager,
		DtComponentDescriptor* desc = 0,
		DtReaderWriterRegistry* parentRegistry = 0);

	virtual ~MyHumanGunJoyActuator();


	virtual bool init();

	//! \return DtVreVrfTurretedBalGunPSRType
	virtual const char* type() const;


	virtual void tick() override;

	//! Returns a newly created instance of this class.
	//! Create an instance of a DtVreWeaponPSR
	static DtSimComponent* creator(const DtString& name,
		DtLocalObject* owner,
		DtSimulationServices* simManager,
		DtComponentDescriptor* desc = 0,
		DtReaderWriterRegistry* parentRegistry = 0);


	

protected:
	virtual void depleteResource(int amount = 1) override;

private:
	void InitPlugin();

	void OnWeaponReload(const WeaponReloadPacket& packet);
	void OnDataReceived(const std::string& clientAddr, const uint8_t* data, int len);

	void SetRoundsInCurrentMagazine(int roundsToSet);
	void SetTotalRoundsRemaining(int amount);


	std::thread netThread;
	nesslab_common::PacketHandlers packetHandlers{};
	std::vector<BYTE> reloadRecvBuffer{};


};



//void SetWeaponState(DtWeaponState state);
//makVre::DtVreMessageResult HandleForceFireCmd(makVre::DtVreMessage* msg);
//	void OnFire();
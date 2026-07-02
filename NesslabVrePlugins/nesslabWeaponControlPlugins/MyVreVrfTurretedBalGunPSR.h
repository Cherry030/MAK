#pragma once

#include "vrfExtensions/vreVrfobjcore/vreVrfTurretedBalGunPSR.h"
#include "AutoReloadBlocker.h"

#include "vrfobjcore/vrfProcessStateRepository.h"

/*
				MyVreVrfTurretedBalGunPSR 생성 과정

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
		 (component-type "MyHumanGunJoyActuator") 
		 (min-tick-period -1.000000)
		 (min-tick-period-variance -1.000000)
		 (periodic-tick-while-paused False)
		 (process-state-repository-name $process-state-name)
		 (process-state-repository-type "")   ------> process-state-repository-type이름이 plugin.cxx에서 생성하는 Type이름이랑 일치하게 변경
													, 지금은 DtVreVrfTurretedBalGunPSR type사용 중(기존 부모 타입)


						총기 PSR 플러그인

	메모
	- AutoReloadBlocker 클래스(자동재장전 방지 기능) 생성 및 총기에 적용
	- 소총, 권총 각 각 생성자 생성

*/

class MyVreVrfTurretedBalGunPSR:public makVre::DtVreVrfTurretedBalGunPSR
{
public:
	MyVreVrfTurretedBalGunPSR(DtLocalObject* vrfObject,
		const DtString& rwName = DtString::nullString(),
		DtReaderWriterRegistry* parentRegistry = 0);


	MyVreVrfTurretedBalGunPSR(const DtVreVrfTurretedBalGunPSR& orig,
		const DtString& rwName = DtString::nullString(),
		DtReaderWriterRegistry* parentRegistry = 0);


	//MyVreVrfTurretedBalGunPSR& operator=(const MyVreVrfTurretedBalGunPSR& orig);

	virtual ~MyVreVrfTurretedBalGunPSR();


	//! \return DtVreVrfTurretedBalGunPSRType
	virtual DtString type() const;

	
	//! Returns a newly created instance of this class.
	//! Create an instance of a DtVreWeaponPSR
	static DtVrfProcessStateRepository* creator(const DtString& name,
		DtLocalObject* vrfObject, 
		DtReaderWriterRegistry* parentRegistry);

	
	const DtVrfMunitionLoader& munitionLoader() const override;
	DtVrfMunitionLoader& munitionLoader() override;

	//const AutoReloadBlocker& munitionLoader() const;
	//AutoReloadBlocker& munitionLoader();

private:
	AutoReloadBlocker myMunitionLoader;

};


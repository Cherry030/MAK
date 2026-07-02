#pragma once

#include <vreVrfmodel/vreSimComponent.h>
#include <vrfobjcore/actuatorComponent.h>
#include <vrfobjcore/vrfMovingObjectStateRepository.h>
#include "vreMessageManager/vreMessageManager.h"
#include "vreMessageManager/vreMessage.h"

#include "SkBoundingVolume.h"
#include "array"

/*
					스켈레톤 플러그인(BE)

		메모
		- 캐릭터의 스켈레톤을 제어하기 위한 플러그인
		- 연세대 or Motive -> VRIS
		- ComponentType 경로
		C:\MAK\vrengage2.1.1b\data\simulationModelSets\VR-Engage\vrfSim\platforms => ExampleHumanHandWeaponControl.ope
		(스켈레톤 캐릭터에 따라 적용 .ope파일이 다름 => 스켈레톤 캐릭터 .entity파일에서 확인가능(platform= 부분)
		 ex)
		 <simObject objectType="1:3:1:120:11:35:42:5" matchType="1:3:1:120:11:35:42:5" platform="@(platforms-dir)/Skeleton_with_Handpose.ope">
		)

		(human-art-parts-actuator 컴포넌트 밑에 아래 내용 추가

	  (nesslab_skeleton
		 (component-descriptor-type "actuator-component-descriptor")
		 (component-type "nesslab_skeleton_type") => 이부분이 plugin.cxx생성 타입과 일치하게
		 (min-tick-period -1.000000)
		 (min-tick-period-variance -1.000000)
		 (tick-period-uses-real-time False)
		 (process-state-repository-name "")
		 (process-state-repository-type "")
		 (is-enabled True)
		 (debug-detail False)
		 (create-component True)
		 (create-on-remote-object False)
	  )


*/


namespace makVre
{

	//Type 경로 => C:\MAK\vrengage2.1.1b\data\simulationModelSets\VR-Engage\vrfSim\platforms\ExampleHumanHandWeaponControl.ope
	const char SKELETON_PLUGIN_TYPE[] = "nesslab_skeleton_type";

	class SkeletonPlugin : public DtVreSimComponent<DtActuatorComponent>
	{

	public:

		//! \copydoc DtSimComponent(const DtString&,DtLocalObject*,DtSimulationServices*,
		//! DtComponentDescriptor*,DtReaderWriterRegistry* parentRegistry)
		SkeletonPlugin(const DtString& name,
			DtLocalObject* owner,
			DtSimulationServices* simManager,
			DtComponentDescriptor* desc = 0,
			DtReaderWriterRegistry* parentRegistry = 0);

		//! default constructor; not implemented
		SkeletonPlugin() = delete;

		//! copy constructor; not implemented
		SkeletonPlugin(const SkeletonPlugin& orig) = delete;

		//! assignment operator; not implemented
		const SkeletonPlugin& operator=(const SkeletonPlugin& orig) = delete;

		//! destructor
		virtual ~SkeletonPlugin();

	public:
		//! Cache the existing hand item being held by the entity.  In the case of 
		//! this example, it is an "M4A1Carbine".  Will hang on to this in order to restore
		//! this hand item if/when the player releases control.
		bool init() override;

		//! \copydoc DtSimComponent::type()
		//! \return DtExampleHumanHandWeaponControlActuatorType
		const char* type() const override;

		//! Checks the following player state conditions:
		//! 
		//! If engaging entity.  If true, then switches the existing
		//! hand item ("M4A1Carbine") to "M4A1Carbine_Left_Hand_Example".  This hand item
		//! is configured to attach to the left hand joint of the skeleton.
		//! 
		//! If existing player control of the entity, restores the original hand item,
		//! in this case "M4A1Carbine".
		//! 
		//! If controlling the player, wave the left wrist back and forth.  You should
		//! see the weapon in the player's left hand in 3rd person view.
		void tick() override;

		//! \copydoc DtSimComponent(const DtString&,DtLocalObject*,DtSimulationServices*,
		//! DtComponentDescriptor*,DtReaderWriterRegistry* parentRegistry)
		//! \return New instance of this class.
		static DtSimComponent* creator(const DtString& name,
			DtLocalObject* owner,
			DtSimulationServices* simManager,
			DtComponentDescriptor* desc = 0,
			DtReaderWriterRegistry* parentRegistry = 0);

	protected:

		//! Cache player controlled state so we can check for transitions in/out
		//! of player controlled state.
		bool myPlayerControlled = false;


		//! Member to allow for access to platform specific data
		DtPlatformLocalObjectFacade myPlatformLocalObjectFacade;


		//-----------------DiGuyDisArtParts(15개)-----------------
		//중앙(3개)
		makVrf::DtArticulatedPartStateRepository* myBase		= nullptr;//골반 중앙, 중심
		makVrf::DtArticulatedPartStateRepository* myBack		= nullptr;//명치 약간 아래
		makVrf::DtArticulatedPartStateRepository* myCervical	= nullptr;//머리와 목 사이


		//왼쪽(6개)
		makVrf::DtArticulatedPartStateRepository* myLeftShoulder = nullptr;//왼쪽 어깨
		makVrf::DtArticulatedPartStateRepository* myLeftElbow	 = nullptr;//왼쪽 팔꿈치
		makVrf::DtArticulatedPartStateRepository* myLeftWrist	 = nullptr;//왼쪽 손목
		makVrf::DtArticulatedPartStateRepository* myLeftHip		 = nullptr;//왼쪽 엉덩이
		makVrf::DtArticulatedPartStateRepository* myLeftKnee	 = nullptr;//왼쪽 무릎
		makVrf::DtArticulatedPartStateRepository* myLeftAnkle	 = nullptr;//왼쪽 발목


		//오른쪽(6개)
		makVrf::DtArticulatedPartStateRepository* myRightShoulder	= nullptr;//오른쪽 어깨
		makVrf::DtArticulatedPartStateRepository* myRightElbow		= nullptr;//오른쪽 팔꿈치
		makVrf::DtArticulatedPartStateRepository* myRightWrist		= nullptr;//오른쪽 손목
		makVrf::DtArticulatedPartStateRepository* myRightHip		= nullptr;//오른쪽 엉덩이
		makVrf::DtArticulatedPartStateRepository* myRightKnee		= nullptr;//오른쪽 무릎
		makVrf::DtArticulatedPartStateRepository* myRightAnkle		= nullptr;//오른쪽 발목
		//--------------------------------------------------------------------



		// Bookkeeping for animating the motion of the joint over a given 
		// time interval.  Joint will periodically expand/contract.
		double myAnimationInterval;
		double myTimeInInterval;


	private:
		void InitPlugin();
		void SetSkeleton();
		void OnDataReceived(unsigned char* data, int len);

		std::array<makVrf::DtArticulatedPartStateRepository*, 15> mySkeletons{};
		SkBoundingVolume* skBoundingVolume = nullptr;
		std::thread netThread;


		//! Handles incoming CustomMessage from the simulation backend.
		makVre::DtVreMessageResult HandleStopPluginMessage(makVre::DtVreMessage* msg);
		bool stopRequested = false;

	};
}

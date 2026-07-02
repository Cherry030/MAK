#pragma once

#include "vrfobjcore/actuatorComponent.h"
#include <vrfobjcore/localObject.h>
#include <geometry/surface.h>
#include <vrfobjcore/vrfObjectStateRepository.h>
#include "vreMessageManager/vreMessageManager.h"
#include "vreMessageManager/vreMessage.h"



/*
					지형 타입 플러그인(BE)

		메모
		- 기동보조 장치에서 지형 값을 알기 위한 플러그인
		- ComponentType 경로
		C:\MAK\vrengage2.1.1b\data\simulationModelSets\VR-Engage\vrfSim\systems\movement  => human.sysdef

		(update-repository 컴포넌트 밑에 아래 내용 추가

	  (nesslab_terrain_soil
		 (component-descriptor-type "actuator-component-descriptor")
		 (component-type "nesslab_terrain_soil")
		 (min-tick-period -1.000000)
		 (min-tick-period-variance -1.000000)
		 (periodic-tick-while-paused False)
		 (process-state-repository-name "")
		 (process-state-repository-type "")
		 (is-enabled True)
		 (create-component True)
		 (create-on-remote-object False)
		 (default-art-part-list )
		 (ladder-climbing-speed 0.400000)
	  )


*/

namespace nesslab_backend_plugins {

		const char TERRAIN_SOIL_PLUGIN_TYPE[] = "nesslab_terrain_soil";


		class TerrainSoilPlugin : public DtActuatorComponent
		{
		public:

			//! \copydoc DtSimComponent(const DtString&,DtLocalObject*,DtSimulationServices*,
			//! DtComponentDescriptor*,DtReaderWriterRegistry* parentRegistry)
			TerrainSoilPlugin(const DtString& name,
				DtLocalObject* owner,
				DtSimulationServices* simManager,
				DtComponentDescriptor* desc = 0,
				DtReaderWriterRegistry* parentRegistry = 0);

			//! default constructor; not implemented
			TerrainSoilPlugin() = delete;

			//! copy constructor; not implemented
			TerrainSoilPlugin(const TerrainSoilPlugin& orig) = delete;

			//! assignment operator; not implemented
			const TerrainSoilPlugin& operator=(const TerrainSoilPlugin& orig) = delete;

			//! destructor
			virtual ~TerrainSoilPlugin();


			virtual bool init() override;

			//! \copydoc DtSimComponent::type()
			//! \return DtHumanGunActuatorType
			virtual const char* type() const override;

			virtual void tick() override;

			//! \copydoc DtSimComponent(const DtString&,DtLocalObject*,DtSimulationServices*,
			//! DtComponentDescriptor*,DtReaderWriterRegistry* parentRegistry)
			//! \return New instance of this class.
			static DtSimComponent* creator(const DtString& name,
				DtLocalObject* owner,
				DtSimulationServices* simManager,
				DtComponentDescriptor* desc = 0,
				DtReaderWriterRegistry* parentRegistry = 0);


		private:

			void InitPlugin();

			//Callback Function
			//void OnMadhDisconnected();
			//void HandleMadhDisconnectThr();

			void SendMoveResistanceToMadh();
			uint8_t FormatSoilTypeMobilityResistance();
			std::string GetSoilTypeString(const DtSurface& surface);


			//! Check for specific role, or any role by using default parameter.
			bool IsPlayerControlled(const std::string& role = "");


			uint8_t previousSoilType = 255;
			DtSoilType currentSoilType = DtSoilType::UndefinedSoilType;

			uint8_t previousBodyOfWater = 255;
			DtBodyOfWaterType currentBodyOfWater = DtBodyOfWaterType::UnspecifiedBodyOfWater;
			std::thread netThread;


			void SendSoilTypeToDsms();
			void OnDsmsDisconnected();
			void HandleDsmsDisconnectedThr();

			//! Handles incoming CustomMessage from the simulation backend.
			makVre::DtVreMessageResult HandleStopPluginMessage(makVre::DtVreMessage* msg);
			bool stopRequested = false;

		};
}
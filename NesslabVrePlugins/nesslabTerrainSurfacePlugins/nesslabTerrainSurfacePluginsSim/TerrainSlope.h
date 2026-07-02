#pragma once

#include "vrfobjcore/actuatorComponent.h"
#include <vrfobjcore/localObject.h>
#include <geometry/surface.h>
#include <vrfobjcore/vrfObjectStateRepository.h>


/*
						지형 기울기 플러그인(BE)

		메모
		- 트레드밀 기울기 값을 처리하기 위한 플러그인
		- ComponentType 경로
		C:\MAK\vrengage2.1.1b\data\simulationModelSets\VR-Engage\vrfSim\systems\movement  => human.sysdef

		(update-repository 컴포넌트 밑에 아래 내용 추가

        (nesslab_terrain_slope
           (component-descriptor-type "actuator-component-descriptor")
           (component-type "nesslab_terrain_slope")
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

		const char TERRAIN_SLOPE_PLUGIN_TYPE[] = "nesslab_terrain_slope";

		class TerrainSlopePlugin : public DtActuatorComponent
		{
		public:

			//! \copydoc DtSimComponent(const DtString&,DtLocalObject*,DtSimulationServices*,
			//! DtComponentDescriptor*,DtReaderWriterRegistry* parentRegistry)
			TerrainSlopePlugin(const DtString& name,
				DtLocalObject* owner,
				DtSimulationServices* simManager,
				DtComponentDescriptor* desc = 0,
				DtReaderWriterRegistry* parentRegistry = 0);

			//! default constructor; not implemented
			TerrainSlopePlugin() = delete;

			//! copy constructor; not implemented
			TerrainSlopePlugin(const TerrainSlopePlugin& orig) = delete;

			//! assignment operator; not implemented
			const TerrainSlopePlugin& operator=(const TerrainSlopePlugin& orig) = delete;

			//! destructor
			virtual ~TerrainSlopePlugin();


		public:
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

			//void playerControlledRequestCallback();
			DtQuaternion getIntersectionQuat(const DtVector& intersection, const DtVector& normal, Coordinate_System* coordinateSystem, DtLocalObject* myEntity);
			//DtQuaternion TerrainSlopePlugin::getIntersectionQuat(const DtVector& intersection, const DtVector& normal, Coordinate_System* coordinateSystem, DtLocalObject* myEntity, double& currentRoll, double& currentPitch);


		
		private:

			void InitPlugin();

			//VRIS -> TDM
			void SendDataToTreadmill(const double& xValue, const double& yValue);
			void GetTerrainOrientation(const DtVector& intersection, const DtVector& normal);

			//! Check for specific role, or any role by using default parameter.
			bool IsPlayerControlled(const std::string& role = "");

			float treadmillRollDeg = 0.0f;
			float treadmillPitchDeg = 0.0f;
			std::thread netThread;



			void GetTerrainOrientation(const DtVector& intersection, const DtVector& normal, Coordinate_System* coordinateSystem, DtLocalObject* myEntity);



		};

}
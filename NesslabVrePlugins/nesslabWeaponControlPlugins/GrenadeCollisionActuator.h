#pragma once

#include "vrfmodel/frictionlessGravityCollisionActuator.h"


/*

	수류탄 충돌 에러(충돌 처리 안되는 현상)를 해결하기 위한 플러그인

	메모 
	- vrengage2.2 버전에서는 해결된걸로 확인됨

*/

namespace nesslab_backend_plugins
{

	class GrenadeCollisionActuator : public DtFrictionlessGravityCollisionsActuator
	{
	public:

		GrenadeCollisionActuator(const DtString& name, DtLocalObject* object,
			DtSimulationServices* simServices, DtComponentDescriptor* compDescriptor,
			DtReaderWriterRegistry* parentRegistry = 0);

		virtual ~GrenadeCollisionActuator();


		//Returns a string from compTypes.h, identifying the type of component
		virtual const char* type() const;
		
	protected:
		//! Overriden to return true in all cases.
		//! This allows for correct falling even when the entity is under a roof or other part of the terrain.
		virtual bool isFallingApplicable() override;
	public:
		//Creator function to be registered with the DtSimComponentFactory --
		//see main.cxx
		static DtSimComponent* creator(const DtString& name, DtLocalObject* object,
			DtSimulationServices* simServices, DtComponentDescriptor* compDescriptor,
			DtReaderWriterRegistry* parentRegistry);

	};
}